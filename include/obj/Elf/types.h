#ifndef ELF_TYPES_H
#define ELF_TYPES_H

#include <stdint.h>
#include <stddef.h>

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

// A growable byte buffer -- the only storage primitive, with no ELF knowledge.
typedef struct Elf_Buffer Elf_Buffer;
struct Elf_Buffer {
    uint8_t *eb_data;
    size_t   eb_len;
    size_t   eb_cap;
};

// Forward declaration: a symbol's defining section is a pointer to one of these.
typedef struct Elf_Sec Elf_Sec;

// One symbol; sym_sec == NULL means undefined (an external reference).
typedef struct Elf_Sym Elf_Sym;
struct Elf_Sym {
    const char *sym_name;    // owned by the Elf string pool
    Elf_Sec    *sym_sec;     // defining section, or NULL
    uint64_t    sym_value;   // offset within sym_sec
    uint64_t    sym_size;
    uint8_t     sym_bind;    // ELF_BIND_*
    uint8_t     sym_type;    // ELF_TYPE_*
    uint8_t     sym_other;   // visibility
};

// One relocation; it lives in the section it patches and rel_type is opaque here.
typedef struct Elf_Rela Elf_Rela;
struct Elf_Rela {
    uint64_t  rel_offset;    // within the patched section
    Elf_Sym  *rel_sym;       // referenced symbol
    uint32_t  rel_type;      // R_<machine>_* (opaque here)
    int64_t   rel_addend;
};

// One section; PROGBITS carry bytes in sec_data and may own their relocations.
struct Elf_Sec {
    const char *sec_name;    // owned by the string pool
    uint32_t    sec_type;    // ELF_SHT_*
    uint64_t    sec_flags;   // ELF_SHF_*
    uint64_t    sec_addr;    // load address, 0 = unplaced
    uint64_t    sec_addralign;
    uint64_t    sec_entsize;
    Elf_Buffer  sec_data;    // raw contents (PROGBITS)
    Elf_Rela   *sec_relas;   // relocations patching THIS section
    size_t      sec_nrelas;
    size_t      sec_caprelas;
};

// An ELF object: header fields, sections, symbols and a name string pool.
typedef struct Elf Elf;
struct Elf {
    uint16_t    elf_type;
    uint16_t    elf_machine;
    uint64_t    elf_entry;
    Elf_Sec   **elf_secs;
    size_t      elf_nsecs;
    size_t      elf_capsecs;
    Elf_Sym   **elf_syms;
    size_t      elf_nsyms;
    size_t      elf_capsyms;
    char      **elf_pool;
    size_t      elf_npool;
    size_t      elf_cappool;
    const char *elf_err;
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
    int             lo_nplaces;
};

// A loaded program: one flat buffer holding every PT_LOAD plus a stack above
// them, and the two addresses execution starts from.
typedef struct Elf_LoadImage Elf_LoadImage;
struct Elf_LoadImage {
    uint8_t  *li_mem;      // li_size bytes, zeroed and then filled
    uint64_t  li_base;     // virtual address li_mem[0] stands for
    uint64_t  li_size;
    uint64_t  li_entry;    // e_entry
    uint64_t  li_stack;    // initial %rsp, 16-byte aligned
    uint16_t  li_machine;  // e_machine, for the caller to accept or reject
};

#endif // ELF_TYPES_H
