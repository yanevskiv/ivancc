// C header file for ELF objects and executables.

#ifndef ELF_H
#define ELF_H

#include <stddef.h>
#include <stdint.h>
#include "util/file.h"

// Format
// Object file types (e_type).
#define ELF_ET_REL  1
#define ELF_ET_EXEC 2

// Target machine (e_machine).
#define ELF_EM_X86_64 62

// Section header types (sec_type).
#define ELF_SHT_PROGBITS 1
#define ELF_SHT_SYMTAB   2
#define ELF_SHT_STRTAB   3
#define ELF_SHT_RELA     4
#define ELF_SHT_NOBITS   8

// Section header flags (sec_flags).
#define ELF_SHF_WRITE     0x1
#define ELF_SHF_ALLOC     0x2
#define ELF_SHF_EXECINSTR 0x4

// Symbol binding (sym_bind).
#define ELF_BIND_LOCAL  0
#define ELF_BIND_GLOBAL 1
#define ELF_BIND_WEAK   2

// Symbol type (sym_type).
#define ELF_TYPE_NOTYPE 0
#define ELF_TYPE_OBJECT 1
#define ELF_TYPE_FUNC   2

// ELF identification bytes: 64-bit, little-endian, version 1.
#define ELF_CLASS64  2
#define ELF_DATA2LSB 1
#define ELF_VERSION  1

// Virtual address an executable is loaded at, and the page size segments align.
#define ELF_BASE 0x400000
#define ELF_PAGE 0x1000

// Loadable program-header segment, readable and executable.
#define ELF_PT_LOAD 1
#define ELF_PF_R 4
#define ELF_PF_W 2
#define ELF_PF_X 1

// The undefined section index used by external references.
#define ELF_SHN_UNDEF 0

// Pack and unpack the symbol binding/type nibbles of st_info.
#define ELF_ST_INFO(bind, type) (((bind) << 4) | ((type) & 0xF))
#define ELF_ST_BIND(info) ((info) >> 4)
#define ELF_ST_TYPE(info) ((info) & 0xF)

// Pack and unpack the symbol index and type of r_info.
#define ELF_R_INFO(sym, type) (((uint64_t) (sym) << 32) | (uint32_t) (type))
#define ELF_R_SYM(info)  ((uint32_t) ((info) >> 32))
#define ELF_R_TYPE(info) ((uint32_t) ((info) & 0xFFFFFFFF))

// Forward declaration: a symbol names its defining section.
typedef struct Elf_Sec Elf_Sec;

// Types
// The fixed-size ELF file header, on disk.
typedef struct Elf64_Ehdr Elf64_Ehdr;
struct Elf64_Ehdr {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

// One program header, describing a segment to load.
typedef struct Elf64_Phdr Elf64_Phdr;
struct Elf64_Phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

// One section header in the section header table.
typedef struct Elf64_Shdr Elf64_Shdr;
struct Elf64_Shdr {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
};

// One entry in an on-disk .symtab.
typedef struct Elf64_Sym Elf64_Sym;
struct Elf64_Sym {
    uint32_t st_name;
    uint8_t  st_info;
    uint8_t  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
};

// One entry in an on-disk .rela.* section.
typedef struct Elf64_Rela Elf64_Rela;
struct Elf64_Rela {
    uint64_t r_offset;
    uint64_t r_info;
    int64_t  r_addend;
};

// A growable byte buffer, the only storage primitive here.
typedef struct Elf_Buffer Elf_Buffer;
struct Elf_Buffer {
    uint8_t *eb_data;
    size_t   eb_len;
    size_t   eb_cap;
};

// One symbol.
typedef struct Elf_Sym Elf_Sym;
struct Elf_Sym {
    const char *sym_name;    // owned by the Elf string pool
    Elf_Sec    *sym_sec;     // defining section, or NULL
    uint64_t    sym_value;   // offset within sym_sec
    uint64_t    sym_size;    // bytes the object occupies, 0 where unknown
    uint8_t     sym_bind;    // ELF_BIND_*
    uint8_t     sym_type;    // ELF_TYPE_*
    uint8_t     sym_other;   // visibility
};

// One relocation.
typedef struct Elf_Rela Elf_Rela;
struct Elf_Rela {
    uint64_t  rel_offset;    // within the patched section
    Elf_Sym  *rel_sym;       // referenced symbol
    uint32_t  rel_type;      // R_<machine>_* (opaque here)
    int64_t   rel_addend;    // constant added to the symbol's address
};

// One section.
struct Elf_Sec {
    const char *sec_name;      // owned by the string pool
    uint32_t    sec_type;      // ELF_SHT_*
    uint64_t    sec_flags;     // ELF_SHF_*
    uint64_t    sec_addr;      // load address, 0 = unplaced
    uint64_t    sec_addralign; // address multiple the section must sit on
    uint64_t    sec_entsize;   // bytes one entry takes in a table section
    Elf_Buffer  sec_data;      // raw contents (PROGBITS)
    Elf_Rela   *sec_relas;     // relocations patching THIS section
    size_t      sec_nrelas;    // relocations sec_relas holds
    size_t      sec_caprelas;  // relocations sec_relas has room for
};

// An ELF object: header fields, sections, symbols and a name string pool.
typedef struct Elf Elf;
struct Elf {
    uint16_t    elf_type;    // ELF_ET_*
    uint16_t    elf_machine; // ELF_EM_*
    uint64_t    elf_entry;   // entry point of an executable
    Elf_Sec   **elf_secs;    // sections, in the order they were added
    size_t      elf_nsecs;   // sections elf_secs holds
    size_t      elf_capsecs; // sections elf_secs has room for
    Elf_Sym   **elf_syms;    // symbols, in the order they were added
    size_t      elf_nsyms;   // symbols elf_syms holds
    size_t      elf_capsyms; // symbols elf_syms has room for
    char      **elf_pool;    // every name a section or symbol points into
    size_t      elf_npool;   // names elf_pool holds
    size_t      elf_cappool; // names elf_pool has room for
    const char *elf_err;     // why the last read failed, or NULL
};

// One -place request: load the named section at a fixed address.
typedef struct Elf_LinkPlace Elf_LinkPlace;
struct Elf_LinkPlace {
    const char *lp_name;
    uint64_t    lp_addr;
};

// Options controlling a link.
typedef struct Elf_LinkOptions Elf_LinkOptions;
struct Elf_LinkOptions {
    const char     *lo_entry;        // entry symbol (NULL selects _start)
    int             lo_relocatable;  // -r: merge into an ET_REL object, keep relocs
    Elf_LinkPlace  *lo_places;       // -place requests, in the order given
    size_t          lo_nplaces;      // requests lo_places holds
};

// A loaded program: one flat buffer holding every PT_LOAD and a stack.
typedef struct Elf_LoadImage Elf_LoadImage;
struct Elf_LoadImage {
    uint8_t  *li_mem;      // li_size bytes, zeroed and then filled
    uint64_t  li_base;     // virtual address li_mem[0] stands for
    uint64_t  li_size;     // bytes li_mem holds
    uint64_t  li_entry;    // e_entry
    uint64_t  li_stack;    // initial %rsp, 16-byte aligned
    uint16_t  li_machine;  // e_machine, for the caller to accept or reject
};

// Buffers
void   Elf_Buffer_Init(Elf_Buffer *buf);
void   Elf_Buffer_Free(Elf_Buffer *buf);
void   Elf_Buffer_Reserve(Elf_Buffer *buf, size_t n);
void  *Elf_Buffer_At(Elf_Buffer *buf, size_t off);
size_t Elf_Buffer_Byte(Elf_Buffer *buf, uint8_t value);
size_t Elf_Buffer_Data(Elf_Buffer *buf, const void *data, size_t n);
size_t Elf_Buffer_U16(Elf_Buffer *buf, uint16_t value);
size_t Elf_Buffer_U32(Elf_Buffer *buf, uint32_t value);
size_t Elf_Buffer_U64(Elf_Buffer *buf, uint64_t value);
size_t Elf_Buffer_Zero(Elf_Buffer *buf, size_t n);
size_t Elf_Buffer_Align(Elf_Buffer *buf, size_t align);

// Objects
const char *Elf_Intern(Elf *elf, const char *name);
Elf        *Elf_New(uint16_t type, uint16_t machine);
void        Elf_Free(Elf *elf);
void        Elf_SetEntry(Elf *elf, uint64_t vaddr);
void        Elf_SetType(Elf *elf, uint16_t type);
uint16_t    Elf_GetType(const Elf *elf);
const char *Elf_Error(const Elf *elf);

// Sections
Elf_Sec    *Elf_Section_Add(Elf *elf, const char *name, uint32_t type, uint64_t flags);
Elf_Sec    *Elf_Section_Find(Elf *elf, const char *name);
Elf_Sec    *Elf_Section_Get(Elf *elf, const char *name, uint32_t type, uint64_t flags);
size_t      Elf_Section_Count(const Elf *elf);
Elf_Sec    *Elf_Section_At(const Elf *elf, size_t i);
Elf_Buffer *Elf_Section_Data(Elf_Sec *sec);
void        Elf_Section_Addr(Elf_Sec *sec, uint64_t addr);

// Symbols
Elf_Sym *Elf_Symbol_Add(Elf *elf, const char *name, Elf_Sec *sec, uint64_t value, uint8_t bind, uint8_t type);
Elf_Sym *Elf_Symbol_Find(Elf *elf, const char *name);
size_t   Elf_Symbol_Count(const Elf *elf);
Elf_Sym *Elf_Symbol_At(const Elf *elf, size_t i);

// Relocations
Elf_Rela *Elf_Rela_Add(Elf_Sec *target, uint64_t offset, Elf_Sym *sym, uint32_t type, int64_t addend);
size_t    Elf_Rela_Count(const Elf_Sec *target);
Elf_Rela *Elf_Rela_At(const Elf_Sec *target, size_t i);

// Reading
const Elf64_Ehdr *Elf_Read_Ehdr(const uint8_t *data, size_t n);
Elf              *Elf_Read_Mem(const void *buf, size_t n);
Elf              *Elf_Read_Path(const char *path);

// Writing
uint32_t Elf_Write_Str(Elf_Buffer *strtab, const char *name);
uint32_t Elf_Write_SectionIndex(const Elf *elf, const Elf_Sec *sec, const uint32_t *secidx);
void     Elf_Write_Symtab(const Elf *elf, const uint32_t *secidx, Elf_Buffer *symtab, Elf_Buffer *strtab, uint32_t *slot, uint32_t *first_global);
void     Elf_Write_Relas(const Elf_Sec *sec, const uint32_t *slot, const Elf *elf, Elf_Buffer *out);
int      Elf_Write_Rel(const Elf *elf, File_Stream *out);
uint32_t Elf_Write_SegFlags(const Elf_Sec *sec);
uint64_t Elf_Write_PlaceOffset(uint64_t pos, uint64_t vaddr);
int      Elf_Write_Exec(const Elf *elf, File_Stream *out);
int      Elf_Write_File(const Elf *elf, File_Stream *out);
int      Elf_Write_Path(const Elf *elf, const char *path);

// Linking
long     Elf_Link_SectionIndex(const Elf *elf, const Elf_Sec *target);
long     Elf_Link_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Elf_Link_FindGlobal(Elf *elf, const char *name);
void     Elf_Link_Merge(Elf *out, Elf *in);
void     Elf_Link_MergeFiles(Elf *out, const char *const *paths, size_t npaths);
void     Elf_Link_AddPlace(Elf_LinkOptions *opts, const char *name, uint64_t addr);
uint64_t Elf_Link_PlacedAddr(const Elf_LinkOptions *opts, const char *name, int *placed);
void     Elf_Link_PlaceSections(Elf *elf, const Elf_LinkOptions *opts);
void     Elf_Link_CheckDefined(Elf *elf);
void     Elf_Link_Exec(Elf *elf, const Elf_LinkOptions *opts);
Elf     *Elf_Link_Run(const char *const *paths, size_t npaths, const Elf_LinkOptions *opts);

// Loading
uint64_t Elf_Load_AlignDown(uint64_t addr, uint64_t align);
uint64_t Elf_Load_AlignUp(uint64_t addr, uint64_t align);
int      Elf_Load_ReadExec(const char *path, Elf_LoadImage *img);
void    *Elf_Load_At(const Elf_LoadImage *img, uint64_t vaddr, uint64_t size);
void     Elf_Load_Free(Elf_LoadImage *img);

#endif // ELF_H
