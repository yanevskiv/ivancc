# Roadmap

Plan for closing out [SYNTAX.md](SYNTAX.md) — every unchecked box, in dependency
order, on top of a machine of our own
([`ivanemu`](#stage-3--ivanemu-the-uart-and-hello-world)).

**North star:**

```sh
make tests
```

Every file in [`tests/syntax/`](#tests) green — compiled by our compiler,
assembled by our assembler, linked by our linker, and agreed on by our emulator,
with no kernel and no libc anywhere underneath it. The corpus is the one thing
the whole toolchain is measured against, and closing SYNTAX.md means closing the
corpus. Stage 3 made that measurable by giving us a machine of our own to run it
on; every stage since has been counted in tests.

ISO C99 is the destination, and it is reached at the end of stage 14: the
complete C99 *language*, every box in SYNTAX.md closed, 80 tests green. Every
stage up to there closes SYNTAX.md boxes and teaches whichever tools the new
syntax reaches — the encoder and the interpreter together when it needs
instructions, the linker when it needs sections or relocations.

Stage 15 is not the end of this roadmap. It is the first stage of the next one:
libc, which is a different kind of work and deserves a plan of its own.

Work is test-driven. Every stage below names the tests in
[`tests/syntax/`](#tests) that must go green before it is done; nothing is
finished on prose alone.

## Where we are

The toolchain half is finished. `cc -S`, `cc -c`, `as` and `ld` all work, the
objects are honest ET_REL files, and the GNU cross-checks hold in both
directions: the system `as` assembles our `-S` output, the system `ld` links our
`.o`, and our `ld` links a GNU-produced `.o`.

The back end can already do Hello World. This assembles and links with nothing
but our own tools, and prints:

```
  .section .rodata
.Lmsg:
  .string "Hello World\n"
  .text
  .globl _start
_start:
  mov $1, %rax
  mov $1, %rdi
  lea .Lmsg(%rip), %rsi
  mov $12, %rdx
  syscall
  ...
```

So `.rodata`, string literals, PC-relative addressing, relocations, section
placement and PT_LOAD synthesis are all proven — by the GNU tools' agreement,
which was the only witness we had until stage 3.

Stages 1 to 10 have since landed. `Ast_Type` carries the LP64 sizes and
alignments, the `Sem_*` pass annotates every expression bottom-up, and pointers,
arrays, `sizeof` and casts all work: `int *p = &x;` and `char c = s[i];`
compile, and `char c = 300` narrows to 44 the way C says it must. A variadic
function reaches its anonymous arguments through `__builtin_va_arg`, which is
what `printf` will be built on.

Stage 9 closed the integer type zoo. `short`, `long`, `long long`, `_Bool` and
`signed`/`unsigned` are types of their own, octal literals and the integer
suffixes give a constant its type, and `Sem_*` applies the integer promotions
and the usual arithmetic conversions before any operator sees its operands.
Unsigned code generation follows: `div` rather than `idiv`, `shr` rather than
`sar`, `setb`/`setbe` rather than `setl`/`setle`, and a zero-extending load
where the type has no sign bit. `volatile` and `restrict` parse everywhere a
qualifier may stand and are recorded on the type.

Stage 10 closed the lexical gaps. The full escape set decodes in one place, so
`\a`, `\v`, `\xHH`, octal `\nnn` and the universal character names all reach
`.rodata` as the bytes gcc writes, and an escape the standard does not name is
now a diagnostic rather than a silent guess. Adjacent string literals splice
before anything reads a type, and an `L` prefix makes a literal's elements
`wchar_t`, which is `int` here. A multi-character constant packs into an int the
way the implementation is free to choose.

Stage 11 moved every diagnostic into `util/console/err.h`. Each one has a code
and an entry in one table, and prints in gcc's form with its code in brackets:
`main.c:3: error: break outside a loop [ERR_GEN_BREAK_OUTSIDE_LOOP]`. A test can
now expect a compile to fail with `// (Test) Compiler error:`.

And the claims above stopped being comparisons. `ivanemu` decodes our bytes and
says what they mean: its disassembly agrees with `objdump` over 23,480
instructions, and every test in the corpus exits with the same status under the
interpreter as on the host. A freestanding program storing to a memory-mapped
UART prints through it with nothing linked at all, which is what proves the
device rather than the instruction set.

The language is unfinished, and those gaps are ordinary:

```c
double x = 1.5;         // parse error: no floating-point types
#include <stdio.h>      // lexer error: no preprocessor
int a[] = {1, 2, 3};    // error: too many initializers for an array of 0
char s[] = "abc";       // error: an array needs a braced initializer
```

## The critical path to the north star

The language needs nothing it does not already have. This compiles today,
unchanged:

```c
int main()
{
    char *uart;
    char *msg;

    uart = (char *) 0x10000000;
    msg = "Hello world!\n";
    while (*msg) {
        *uart = *msg;
        msg = msg + 1;
    }
    return 0;
}
```

A cast of a constant to `char *`, a byte store through it, and a `char *` walk
over a literal — stage 1 paid for all three. What is missing is the machine at
the other end of that store:

| Need | Why | Stage |
|---|---|---|
| a loader | turn our `ET_EXEC` into memory | 3 |
| an interpreter | the twenty-two opcodes we emit today | 3 |
| a UART data register | `*uart = c` reaches a terminal | 3 |
| a halt register | a program can stop and report a status | 3 |
| a freestanding `crt0` | a `_start` that halts instead of calling `exit` | 3 |
| `write` and `exit` | so a `-mtarget=linux` binary runs here too, unchanged | 3 |

All six landed together, which is what made the emulator reachable this early
rather than a milestone three stages out. `printf` keeps a dependency table of
its own, in stage 15.

## Tests

Work is test-driven: write the test, watch it fail, make it pass, move on.
`make tests` runs the corpus through [`tests/run_test`](tests/run_test). Each
test is its own phony target, so it always runs, `make -j` parallelises it, and
`make test05_logical` runs a single one.

```
tests/
  run_test                       the harness
  syntax/
    testNN_<description>.c       one feature each, numbered in dependency order
```

`tests/syntax/` holds only tests that pass. A stage's tests land with that
stage, so `make tests` is green at every commit and a red suite always means a
regression rather than a wishlist. The numbering below reserves the names in
advance; the files appear as the features do.

A `testNN_` file pins down exactly one feature, so it fails for exactly one
reason, and it may use no syntax a higher-numbered test introduces. That rule is
what makes the corpus a dependency order rather than a pile: read it top to
bottom and it is the language being built up one construct at a time.

The directory is named for what it holds rather than for the suite, because a
second suite is coming. Emulator tests want a tree of their own, and a libc
suite belongs beside `tests/syntax/` rather than inside it.

### The corpus

**Already passing**, and written. Stage 1 rewrites every local's offset and every
load width, and these are the net underneath that.

- `test01_return`
- `test02_arith`
- `test03_unary`
- `test04_compare`
- `test05_logical`
- `test06_if_else`
- `test07_while`
- `test08_for`
- `test09_block`
- `test10_locals`
- `test11_call`
- `test12_call_stack_args`
- `test13_recursion`

`test12_call_stack_args` passes more than six arguments, covering the ABI's stack
path — easy to break and silent when broken.

**Stage 1 — types and pointers.**

- `test14_char_type`
- `test15_deref`
- `test16_address_of`
- `test17_pointer_arith`
- `test18_pointer_levels`
- `test19_array`
- `test20_array_2d`
- `test21_sizeof`
- `test22_cast`
- `test23_string_walk`

`test23_string_walk` walks a literal through a `char *` and returns its length.
It is `printf`'s inner loop, isolated.

**Stage 2 — varargs.**

- `test24_varargs`
- `test25_varargs_stack`, which reaches the overflow area past six arguments.

**Stage 3 — the emulator.** Landed with no numbered tests and no change to the
ones above:
the corpus is built `-mtarget=linux` and run on the host, as it is today and as
it will be until C99 is closed out. The emulator is exercised by hand — any of
those binaries under `ivanemu`, and a freestanding UART program for the device —
until there are enough emulator tests to be worth a harness.

**Stage 4 — operators.**

- `test26_bitwise`
- `test27_shift`
- `test28_compound_assign`
- `test29_incr_decr`
- `test30_ternary`
- `test31_comma`

**Stage 5 — statements.**

- `test32_do_while`
- `test33_switch`
- `test34_break_continue`
- `test35_goto`
- `test36_block_scope`

**Stage 6 — globals and storage.**

- `test37_globals`
- `test38_storage_class`
- `test39_multi_declarator`
- `test40_extern`
- `test41_array_init`
- `test42_designated_init`

**Stage 7 — aggregates.**

- `test43_struct`
- `test44_struct_ptr`
- `test45_union`
- `test46_enum`
- `test47_typedef`
- `test48_bitfields`
- `test49_struct_byval`
- `test50_compound_literal`
- `test51_stdarg`
- `test52_flex_array`
- `test53_designated_nested`

**Stage 8 — function pointers.**

- `test54_func_ptr`
- `test55_array_param_quals`
- `test56_knr_params`

**Stage 9 — the integer type zoo.**

- `test57_int_widths`
- `test58_unsigned`
- `test59_int_literals`
- `test60_qualifiers`

**Stage 10 — lexical completeness.**

- `test61_escapes`
- `test62_string_concat`
- `test63_wide_literals`
- `test64_multichar`

**Stage 11 — errors.** No numbered tests. Every diagnostic moves into one table,
and the corpus must stay green through the move. `run_test` gains the
compile-failure path here, ready for stage 12's `test71_pragma_error`.

**Stage 12 — the preprocessor.**

- `test65_include`
- `test66_define`
- `test67_macro_func`
- `test68_conditionals`
- `test69_include_guard`
- `test70_predefined`
- `test71_pragma_error`
- `test72_trigraphs`
- `test73_digraphs`
- `test74_depend`

**Stage 13 — floating point.**

- `test75_float_basic`
- `test76_float_literals`
- `test77_float_abi`

**Stage 14 — the conformance tail.**

- `test78_vla`
- `test79_vla_sizeof`
- `test80_vla_param`

80 tests, closing every box in SYNTAX.md. That is the end of the C99 language,
and the end of this roadmap's obligations.

**Stage 15 — libc**, which opens the next roadmap rather than closing this one.

- `test81_putchar`
- `test82_printf_int`
- `test83_printf_str`

### What the harness needs

A test declares what success means on its own first line, above the description,
so the expectation cannot drift away from the code it describes:

```c
// (Test) Return: 42
// The smallest program that can succeed at all.
```

`run_test` reads that, compiles, runs under `timeout`, and compares. The
`(Test)` prefix keeps directives apart from ordinary prose, and a file declaring
none fails outright — otherwise a key typed `// (Test) Retrun:` would assert
nothing and pass.

Four directives exist: `Return`, `Output`, `Compiler status` and `Compiler
error`. `Output` is the primary oracle from stage 3 on, because an exit status is
eight bits and unsigned — enough for `return 42` and nothing a UART or a `printf`
does. Every test is built `-mtarget=linux` and run on the host; nothing in the
corpus names a platform, because so far there is only one worth naming.
It holds a single line, in which `\n` stands for a newline, and a test may
assert on a return, on output, or on both, but never on nothing.

gcc is the oracle for the expectations themselves, run by hand rather than
through the harness: catching a wrong expectation matters more than catching a
wrong compiler, because the corpus is what everything else is measured against.
The 23 tests through stage 1 all agree with gcc; stage 2's two reach their
arguments through a builtin of our own, so their expectations were checked
against an equivalent `<stdarg.h>` program instead. For stages 4, 9 and 13 —
`-7 % 3`, mixed-signedness comparison, float rounding — gcc answers "what does C
actually do here" better than a hand-written expectation does.

Still to come, roughly in the order the stages need them:

- **Emulator tests.** A way to say that a test belongs to `-mtarget=ivanemu`, or
  a tree of its own that the host never runs — and, separately, a second run of
  the ordinary corpus under `ivanemu` on the binaries it already builds. Both
  wait until hand-running the emulator stops being enough, rather than arriving
  with it.
- **Object inspection.** `test37_globals` asserts on `readelf` output (an RW
  `PT_LOAD`, a `.bss` with `p_memsz > p_filesz`), not on what the program prints.
- **Multi-file compiles.** `test40_extern` and `test69_include_guard` need more
  than one input, and they are the only thing that exercises `ivanld` from the
  suite.
- **Expected failures.** Diagnostics are behavior too: an undeclared identifier,
  a non-assignable lvalue, an undefined symbol at link time. These assert on the
  error text and a nonzero exit.
- **The GNU cross-checks**, which stay the cheapest correctness oracle we have
  until `objdump` exists: our `-S` through the system `as`, our `.o` through the
  system `ld`, a system `.o` through our `ld`. All three pass today.

### Whole programs

The corpus is one feature per file on purpose, and that leaves a gap: nothing in
it proves the pieces compose into something a person would actually write. Whole
programs are how that gets covered, and they belong to the libc roadmap rather
than to this one, because every program worth writing wants `printf` first.

Hello World through our own `printf` is stage 15's milestone. `fizz_buzz` follows
it almost for free. Small unix tools — `echo`, `cat`, `wc` — are the obvious
direction after that, but they are a horizon item, not a plan: each one needs
runtime we have deliberately not scoped, such as `argv` handed over by `crt0`,
file descriptors, `string.h` and an allocator.

A freestanding program storing bytes to the emulator's UART is a different thing
again, and it is the emulator's test rather than the language's. It wants the
emulator tree named above, not a numbered slot in the corpus.

---

## Stage 1 — `Ast_Type`, the `Sem_*` pass, and pointers

*Landed.*

Give the compiler a type system and stop discarding declarations. This is the
largest single stage in the file and most of the rest depends on it.

Front end:

- `Ast_Type` (`at_` fields) in `ast.h`, since `Ast_Var` and `Ast_Node` both point
  at one: `at_kind` (VOID/CHAR/INT/PTR/ARRAY), `at_size`, `at_align`, `at_base`
  for the pointee or element type, `at_len` for arrays. Grows `at_members` /
  `at_ret` / `at_params` in stages 7 and 8. If the constructors and queries
  outgrow `ast.c` — struct layout in stage 7 and the conversion rules in stage 9
  are the likely tipping points — they split into `ast/type.c` under the same
  `Ast_Type*` name.
- The sizes and alignments those fields carry are the target ABI's, not the
  language's: `int` is 4 and a pointer is 8 because we are LP64. Keep the table
  in one place so a second target would only have to replace it. Everything else
  in `Sem_*` — decay, conversions, lvalue rules — is genuinely arch-independent,
  which is why the pass belongs in `ast/` and not under `arch/`.
- `Ast_Var` gains `av_type`; `Ast_Node` gains `an_type`.
- `type_name` stops being a no-op: `base` + `stars` build an `Ast_Type`, and the
  `%union` gains an `Ast_Type *`.
- Declarator suffixes for arrays (`int a[10]`, `int m[2][3]`), which is the first
  point where the declarator grammar has to become recursive rather than a flat
  `quals base stars`.
- New node kinds `AST_NODE_KIND_DEREF` and `AST_NODE_KIND_ADDR`.

New `Sem_*` pass, run from `cc.c` between `yyparse()` and code generation, in
`include/lang/sem.h` + `src/lang/sem.c`:

- annotate every expression node with its `an_type`, bottom-up;
- array-to-pointer decay;
- scale pointer arithmetic (`p + i` → `p + i * sizeof *p`, `p - q` → byte
  difference over element size);
- absorb the checks `c.y` does inline today (undeclared identifier,
  non-assignable lvalue) and add call arity checks.

Every node already carries `an_line` and every variable `av_line`, so `Sem_*`
diagnostics say `Log_ShowErrorAt(node->an_line, ...)` and name a line from the
first one written. Locations are bison `@n` values — `%define api.location.type
{int}`, stamped per token by the lexer's `YY_USER_ACTION` — so a rule reports
the line its own operator or keyword sits on, not wherever the parser's
lookahead has reached.

Back end:

- `Gen_x86_64_AssignLvarOffsets` uses real sizes and alignment instead of
  `WORD_SIZE` for every slot.
- Loads and stores pick a width from the type: `movsbq`/`movzbq`/`movslq`/`movq`
  to load, `mov %al`/`%eax`/`%rax` to store.
- `Gen_x86_64_EmitAddr` learns `DEREF` (the address of `*p` is the value of `p`).
- Encoder gains `0F BE` (movsx r64,r/m8), `0F BF` (movsx r64,r/m16), `63`
  (movsxd r64,r/m32), `88` (mov r/m8,r8), and the `66` prefix for 16-bit stores.
- `Txt_x86_64_Reg*Name` gains 16- and 32-bit name tables.

Ticks: dereference `*`, address-of `&`, array subscript `a[i]`, array declarators
incl. multi-dimensional, `sizeof` (type and expression forms), cast expressions.

**Tests:** `test14_char_type` through `test23_string_walk`, with `test01`–`test13`
staying green the whole way.

## Stage 2 — Varargs

*Landed.*

Make the `...` that the grammar already accepts actually reachable.

- In a variadic function's prologue, spill all six integer argument registers
  into a 48-byte register save area in the frame. Named parameters keep spilling
  to their own slots as they do now.
- Add one builtin, `__builtin_va_arg(n)`, returning the n-th anonymous argument
  as a 64-bit value: for `index = nparams + n`, read slot `index` of the save
  area when `index < 6`, otherwise `16 + (index - 6) * 8` off `%rbp`.
- `Gen_x86_64_EmitExpr` already emits `mov $0, %al` before every call, which is
  exactly the SysV "zero vector registers used" convention for a variadic
  callee. It stays correct until floats land in stage 13.

This is deliberately *not* C's `va_list` — that needs a struct, so the real
`va_start`/`va_arg`/`va_end` over the ABI's four-field `va_list` arrives in stage
7 and `__builtin_va_arg` becomes its implementation detail.

**Tests:** `test24_varargs`, `test25_varargs_stack`.

## Stage 3 — `ivanemu`, the UART, and Hello World

*Landed.*

A machine of our own: load the executable our linker writes, decode the bytes our
encoder emits, run them, and let the program say something. It lands before
stage 4 doubles the size of the instruction set, and it is where the north star
is reached.

```
src/emu.c                              driver: arguments, -march selection
include/arch/x86_64/emu.h
src/arch/x86_64/emu.c                  fetch, decode, execute
include/arch/x86_64/load.h
src/arch/x86_64/load.c                 ELF executable -> flat memory
libc/src/x86_64/target/ivanemu/crt0.s  _start that halts through the device
```

```sh
ivancc -mtarget=ivanemu main.c -o main && ivanemu -march=x86_64 ./main
```

### `Load_x86_64_*` in `src/arch/x86_64/load.c`

`Elf_Read_Mem` is the wrong tool here and reusing it would be a mistake. It builds
the *section* model — `Elf_Sec`, symbols, relocations — which is what a linker
wants. A loader wants the segment view, and for an executable the program header
table is the authoritative one; it is also the view that survives the existing
debt where the reader discards non-PROGBITS sections. So the loader reads program
headers straight out of the file bytes. `Elf64_Ehdr` and `Elf64_Phdr` are already
declared in `util/object/elf.h`, so no new structures are needed — only a walk
`Elf_Read_Mem` never does. `Elf_Read_Ehdr` checks the size and the magic and nothing
else, so class, type and machine are the loader's to reject.

```c
// A loaded program: one flat buffer holding every PT_LOAD and a stack.
typedef struct Load_x86_64_Image Load_x86_64_Image;
struct Load_x86_64_Image {
    uint8_t  *li_mem;      // li_size bytes, zeroed and then filled
    uint64_t  li_base;     // virtual address li_mem[0] stands for
    uint64_t  li_size;     // bytes li_mem holds
    uint64_t  li_entry;    // e_entry
    uint64_t  li_stack;    // initial %rsp, 16-byte aligned
    uint16_t  li_machine;  // e_machine, for the caller to accept or reject
};

bool  Load_x86_64_ReadExec(const char *path, Load_x86_64_Image *img);
void *Load_x86_64_At(const Load_x86_64_Image *img, uint64_t vaddr, uint64_t size);
```

1. Read the file, `Elf_Read_Ehdr` to validate it, and require `e_type ==
   ELF_ET_EXEC`.
2. Walk `e_phnum` headers from `e_phoff`, striding by `e_phentsize` rather than
   `sizeof(Elf64_Phdr)` — they agree in our own output, and the check costs
   nothing.
3. First pass over the `PT_LOAD`s for the extent: `li_base` is the lowest
   `p_vaddr` rounded down to a page, the top is the highest
   `p_vaddr + p_memsz` rounded up.
4. Reserve a stack — a megabyte above the top is plenty — and fold it into the
   same allocation so there is one range to check rather than two. `li_stack` is
   its top, 16-byte aligned.
5. Allocate `li_size` **zeroed**, then second pass:
   `memcpy(li_mem + p_vaddr - li_base, file + p_offset, p_filesz)`. Everything
   between `p_filesz` and `p_memsz` is already zero, which is exactly `.bss` —
   so the loader handles stage 6's `p_memsz > p_filesz` before stage 6 writes it.

`Load_x86_64_At` is the part that matters. Every fetch and every data access in the
interpreter goes through it, bounds-checked against `[li_base, li_base + li_size)`,
and a failure reports the faulting virtual address and the `%rip` that asked for
it. That is the difference between *the emulator segfaulted* and *the program read
8 bytes at 0x0 from %rip = 0x401037*, and it is most of why an emulator is worth
having at all.

Two things it deliberately does not do. It does not `mmap` or set page
protections: a flat `malloc` is enough for an interpreter, and once stage 6 makes
`p_flags` honest, keeping each segment's flags alongside the range is a cheaper way
to catch a write into `.text` than real protections would be. And it puts nothing
on the stack: SysV hands `argc`, `argv`, `envp` and auxv to `_start`, but our
`crt0.s` ignores all of it, so a zeroed frame suffices until the libc work wants
`argv` — at which point this is where it gets built.

What the loader sees today is worth knowing: `Elf_Write_Exec` emits one `PT_LOAD`
per ALLOC section that has bytes, every one flagged `R|X`, with
`p_memsz == p_filesz` and `p_align = ELF_PAGE`, at file offsets made page-congruent
by `Elf_Write_PlaceOffset`. Several tiny R+X segments, in other words — correct to load,
and two of those properties are things stage 6 changes.

### The device

Three byte-wide registers at a fixed address, far above anything `ld` places:

| Address | Register | Store | Load |
|---|---|---|---|
| `0x10000000` | UART data | write the byte to standard output | — |
| `0x10000004` | UART status | — | always ready |
| `0x10000008` | halt | stop, and exit with the byte as the status | — |

`Load_x86_64_At` bounds-checks every access against the image, so the device is the one
range it is allowed to miss: an access there is intercepted before the range
check rather than reported by it.

The status register earns its place by keeping the C honest — a driver that polls
before it writes is what a real 16550 needs — but nothing has to read it, and the
emulator never makes it say anything but ready.

The halt register is what lets a freestanding program stop at all. With no
kernel to ask, `exit` has nowhere else to go, and every test asserts on an exit
status, so an interpreter that could print but not stop would be no use beyond
the milestone.

The device is how a `-mtarget=ivanemu` program talks, and it is the whole of
what such a program may assume: `libc/src/x86_64/target/ivanemu/crt0.s` ends
with a store to the halt register rather than `mov $60, %rax; syscall`, and
nothing it links knows what a kernel is.

The interpreter answers `write` and `exit` as well, which is the other half of
the stage. A `-mtarget=linux` binary — everything we build today — then runs
here unchanged, so `./main` and `ivanemu ./main` are the same bytes executed two
ways, and a disagreement between them is a bug in one of us rather than a
difference between two builds. That is worth two syscalls we could have refused:
it makes every test we already have runnable here without being rebuilt, by hand
now and by the harness whenever that earns its keep. qemu keeps these two jobs
in separate programs, `qemu-x86_64` and `qemu-system-x86_64`; at our size they
are a few dozen lines each and one binary can be both.

Twenty-two opcodes cover everything the compiler emits today — `MOV LEA PUSH POP
ADD SUB IMUL IDIV CQO NEG CMP SETE SETNE SETL SETLE MOVZB JMP JE JNE CALL RET
SYSCALL` — and of the syscalls behind that last one, only `write` and `exit`
need answering. The fiddly part is
not the loop, it is EFLAGS: `cmp` followed by `setl`/`setle` needs SF, OF, ZF
and CF right, and signed comparison is exactly where that goes wrong.

`ivanemu` exits with the guest's status, so `// (Test) Return:` means the same
thing on both sides. It needs one status of its own for the cases where the
guest never got to decide — an opcode it cannot decode, an access outside the
image — and 125 is free next to `timeout`'s 124. That leaves the same hole those
two already have, where a guest legitimately returning 124 or 125 cannot be told
apart from the machinery failing, and it is tolerable for the same reason.

What it buys:

- **A second run of the whole corpus.** `run_test --emulator` runs every test
  again under the interpreter rather than on the host: deterministic, independent
  of the machine the suite runs on, and the only way a cross-compiled target gets
  tested at all.
- **A check on the encoder.** Nothing validates the bytes `Enc_x86_64_Instr`
  produces except that they ran on real silicon, which tells us *that* something
  broke and never *what*. The decoder is an independent implementation going the
  other way — bytes back to an opcode and operands — and the two share only the
  `Asm_x86_64_Op` enum, which is a set of names rather than any encoding data. So
  a disagreement at the byte level means one of them is wrong, and REX, ModRM, SIB
  and displacement handling all get exercised from both directions.
- **Traces.** Stage 1's load and store widths are the fiddliest code generation in
  this file, and a wrong exit status is a poor signal for them.
- **A reason for `-march` to exist at all.** Cross-compiling is only worth doing
  if the output can be run, so a second target becomes testable the day it emits
  its first instruction rather than whenever hardware turns up.
- **`objdump`.** The horizon list already wants one, and a decoder is most of it.

Two costs to watch. The interpreter has to keep pace with the compiler: every
stage from here on adds instructions it must decode, stage 4 roughly doubles the
list, and stage 13 means implementing floating-point rounding a second time. And
the syscall side creeps — `write` and `exit` now, `brk` or `mmap` once there is
an allocator, `open`/`read`/`close` for file I/O. The freestanding side does not
creep, because everything it wants has to arrive as a device instead, which is a
cost of its own paid in a different currency.

**Tests:** none automated, and the corpus is untouched. A freestanding program
prints through the device with no runtime linked at all, and the existing tests
are run under `ivanemu` by hand, which is what the two syscalls are for.
Automating either is a harness question, listed with the others and deliberately
not part of this stage.

---

Everything below is the rest of SYNTAX.md, in the order that keeps each stage
useful on its own.

## Stage 4 — The rest of the operators

- Unary `+`; bitwise `& | ^ ~ << >>`; compound assignment (`+= -= *= /= %= &= |=
  ^= <<= >>=`); `++`/`--` prefix and postfix; ternary `?:`; comma operator.
- Compound assignment and `++`/`--` must evaluate the lvalue address exactly
  once — the push/pop pattern `AST_NODE_KIND_ASSIGN` already uses.
- Encoder gains `21`/`09`/`31` (and/or/xor), `F7 /2` (not), `C1 /4 /5 /7` for
  shifts by an immediate and `D3` for shifts by `%cl`.
- Lower compound assignment and `++`/`--` in `Sem_*` where it is cheap to do so,
  so the code generator stays small.

**Tests:** `test26_bitwise` through `test31_comma`, each checked against gcc on
the same input.

## Stage 5 — The rest of the statements

- Block scoping. `Ast_BeginScope` currently resets one flat per-function list;
  it becomes a proper push/pop scope stack, which `for (int i = 0; ...)` needs.
- `do`-`while`; declaration as a `for`-initializer.
- `break` / `continue`: a stack of enclosing loop/switch targets in `Gen_*`.
- `switch` / `case` / `default`: lower to a compare-and-branch chain first; a
  jump table for dense cases is an optimization, not a correctness requirement.
- `goto` and labels: collect each function's labels in `Sem_*`, emit them as
  `.L.user.<func>.<name>`, and diagnose a jump to an undefined label.

**Tests:** `test32_do_while` through `test36_block_scope`.

## Stage 6 — Globals, storage classes, initializers

The first stage that needs ELF work rather than front-end work.

- `external_decl` accepts top-level variable declarations, not just functions.
- `.data` for initialized globals, `.bss` for zero-initialized ones.
- `Elf_Write_Exec` currently hardcodes `ELF_PF_R | ELF_PF_X` on every PT_LOAD.
  Segment flags must follow the section: `.text` R+X, `.rodata` R, `.data`/`.bss`
  R+W.
- `.bss` needs `p_memsz > p_filesz`, which the writer does not do today.
- `Elf_Read_Mem` skips every non-PROGBITS section, so a `.bss` in an input object
  silently vanishes and its symbols come back undefined. It has to keep NOBITS
  sections (size only, no bytes).
- Storage classes: `static` at file scope means `ELF_BIND_LOCAL`; `static` in a
  function means a `.data`/`.bss` slot with a mangled name; `extern` is a
  declaration without a definition; `register` and `auto` are accepted and
  ignored, as is `inline`.
- Multiple declarators per statement, scalar initializers, array initializers
  (`{1, 2, 3}`) and designated initializers (`.field = v`, `[i] = v`).

**Tests:** `test37_globals` through `test42_designated_init`. `test37` asserts on
`readelf` output as well as on what the program prints.

## Stage 7 — Aggregates: struct, union, enum, typedef

- `struct` and `union` declarations, member layout with correct offsets,
  alignment and tail padding; `.` and `->`.
- `enum`, whose constants fold to `int` in `Sem_*`.
- `typedef`. This brings the classic lexer-feedback problem: the lexer must
  return a distinct token for a name currently in scope as a typedef, which
  means the parser has to publish its scope to the lexer.
- Bitfields; compound literals, in declarations and in expression position.
- Flexible array members. The member contributes nothing to `sizeof` and may
  only sit last in a struct that has at least one other member, so this is a
  layout rule rather than a new kind of type.
- Designated initializers in their full nested form: `[1].f[2] = v`, a
  designator list walking into subobjects, and the brace elision C allows
  around it. Stage 6 built the flat `[i] = v` case; this generalises the
  walk to a current-object cursor that `.` and `[` both step.
- Passing and returning structs by value, which needs the SysV argument
  classification algorithm (INTEGER/SSE/MEMORY, and the hidden pointer for large
  returns). This is the hardest ABI work in the file and can ship after
  by-reference struct use works.
- Real `va_list` / `va_start` / `va_arg` / `va_end` over the ABI's four-field
  struct, replacing stage 2's `__builtin_va_arg`.

**Tests:** `test43_struct` through `test53_designated_nested`, with `sizeof` of
every padded struct matching gcc.

## Stage 8 — Function pointers

- Recursive declarator parsing for `int (*f)(int)` and function-pointer
  parameters. Stage 1 starts this rework; this stage finishes it.
- Indirect calls: `call *%rax` (`FF /2`), which the encoder does not emit today.
- `Ast_Type` gains `at_ret` and `at_params` so calls through a pointer can be
  type-checked.
- Qualifiers and `static` inside an array parameter's brackets: `int a[static
  4]`, `int a[const 4]`. Both are declarator syntax that only parses in a
  parameter list, and both decay to a qualified pointer, so this is parsing and
  a qualifier on the decayed type rather than code generation. `static` is a
  promise to the optimizer we do not have yet, and is accepted and ignored.
- Old-style (K&R) definitions: an identifier list in the declarator, then a
  declaration list between the `)` and the `{`. Obsolescent in C99 (6.11.7) but
  still conforming, and cheap once the declarator grammar is recursive — the
  parameters arrive as bare names and the declaration list fills in their types.
  C90's rule that an omitted name defaults to `int` does not survive: C99 6.9.1p6
  requires every identifier in the list to be declared, so an omission is a
  diagnostic rather than a default, exactly as implicit declaration is.
- Unprototyped declarations. `int f();` declares a function taking an
  unspecified number of arguments, which is a different type from `int f(void)`
  and the reason the latter exists. `Ast_Type` records the distinction, and
  `Sem_*` skips arity and argument checking for the unprototyped case rather
  than reporting a mismatch it cannot know about.
- Default argument promotions, which is what makes the above safe to call:
  arguments to an unprototyped or old-style function are promoted before the
  call — `char` and `short` to `int`, and from stage 13, `float` to `double`.
  The same rule already governs a variadic call's anonymous arguments, so the
  two share one path. Stage 9's integer promotions land first and this reuses
  them; until then only `int` and `char` exist and the rule is nearly trivial.

The one piece C99 genuinely removed stays out: implicit declaration, where
calling an undeclared `f()` conjured `int f()`. C99 deleted it, we diagnose it
today, and K&R support does not bring it back.

**Tests:** `test54_func_ptr`, `test55_array_param_quals`, `test56_knr_params`.

## Stage 9 — The integer type zoo

- `short`, `long`, `long long`, `_Bool`, `signed` / `unsigned`.
- Integer suffixes (`u U l L ll LL`) and octal literals in the lexer.
- Usual arithmetic conversions, integer promotion and truncation in `Sem_*`.
- Unsigned code generation: `div` rather than `idiv`, and `setb`/`setbe` rather
  than `setl`/`setle`.
- `volatile` and `restrict` parse and are recorded on the type. We do no
  optimization yet, so neither changes code generation; `volatile` matters the
  day it would.

**Tests:** `test57_int_widths` through `test60_qualifiers`, checked against gcc.

A qualifier written after a pointer star, as in `int * const p`, parses and is
dropped rather than recorded. Nothing reads a qualifier yet, so this costs
nothing today; recording it needs `stars` to carry a qualifier set per star
instead of a count.

## Stage 10 — Lexical completeness

- Full escape set: `\a \b \f \v \xHH`, octal `\nnn`, `\uXXXX` / `\UXXXXXXXX`.
- Adjacent string literal concatenation.
- Wide literals `L"..."` and `L'x'`, with `wchar_t` as `int` and a wide string
  section.
- Multi-character constants (`'ab'`), implementation-defined and worth one line.

**Tests:** `test61_escapes` through `test64_multichar`, compared byte for byte
against gcc's `.rodata`.

## Stage 11 — Errors

Every diagnostic in one place. Today 104 calls to `Log_ShowError` and
`Log_ShowErrorAt` carry their message text inline, spread over thirteen files,
and most sit inside an `if` of their own. This stage moves them into
`util/console/err.h` and `util/console/err.c`. Each diagnostic gets a code, one
table holds every message, and an assertion folds the `if` into the call.

It lands before the preprocessor, so that stage is written against it from the
start.

`Err` is for diagnostics a user sees: a fixed set of messages, each with a code,
predictable enough to test. `Log` prints any message in the form
`ivancc: error: <anything>`, and `Err` prints each of its codes through it.
Developer output can say anything, so it calls `Log` directly.

```
include/util/console/log.h  severities, the locator type, printing
src/util/console/log.c      the program name, the locator, the severity words
include/util/console/err.h  codes, the entry type, raising
src/util/console/err.c      the table, the status
```

`log.c` writes with `fprintf(stderr, ...)`, so it depends on nothing else in the
tree. It offers four functions:

- `Log_ShowError`, which exits, and `Log_ShowWarning`.
- `Log_ShowInfo`, which is on in every build. It is what `-ftime-report` and
  `-fdump-tree-*` print through when they arrive.
- `Log_ShowDebug`, which prints under `ivancc: debug:`. It is always on for now,
  because nothing needs a switch between debug and release builds yet.

`Log_Show` and `Log_ShowVa` take a severity and a line. `Err` prints through
them. `Log_ShowErrorAt` goes, and `Err_AssertAt` replaces it.

```c
// Program name a message carries before Log_SetProgramName.
#define LOG_PROGRAM_DEFAULT "ivancc"

// Line of a message that names no line.
#define LOG_LINE_NONE 0

// Map a line of the compiled text to its file and source line.
typedef const char *(*Log_LineLocator)(uint32_t line, uint32_t *source);

// Setup
void Log_SetProgramName(const char *name);
void Log_SetLineLocator(Log_LineLocator locator);

// Messages
void Log_ShowError(const char *fmt, ...);
void Log_ShowWarning(const char *fmt, ...);
void Log_ShowInfo(const char *fmt, ...);
void Log_ShowDebug(const char *fmt, ...);
void Log_Show(Log_Severity severity, uint32_t line, const char *fmt, ...);
void Log_ShowVa(Log_Severity severity, uint32_t line, const char *fmt, va_list ap);
```

### Codes and the table

```c
// Diagnostic codes, one per distinct message.
typedef enum Err_Code Err_Code;
enum Err_Code {
    ERR_SUCCESS,
    ERR_PAR_BITFIELD_NEGATIVE,
    ERR_PAR_BITFIELD_TOO_WIDE,
    ERR_SEM_ARGS_TOO_FEW,          // callee, count given, count wanted
    ERR_SEM_UNDECLARED,            // identifier
    ERR_CODE_COUNT
};

// A diagnostic's name and message format.
typedef struct Err_Entry Err_Entry;
struct Err_Entry {
    const char *ee_name;
    const char *ee_format;
};
```

```c
static const Err_Entry Err_Table[ERR_CODE_COUNT] = {
    [ERR_PAR_BITFIELD_NEGATIVE] = { "ERR_PAR_BITFIELD_NEGATIVE", "a bit-field width cannot be negative" },
    [ERR_SEM_ARGS_TOO_FEW]      = { "ERR_SEM_ARGS_TOO_FEW",      "too few arguments to %s: got %zu, expected at least %zu" },
};
```

- A code names what went wrong, under the prefix of the module that raises it:
  `ERR_PAR_BITFIELD_NEGATIVE`, not `ERR_PAR_ADD_BITFIELD_3`. A code named after
  its function breaks when the code moves.
- A code stands for one message, not one call site. `par.c` raises "two or more
  data types in one declaration" from two places, and both use one code.
- The comment on a code lists the arguments its format expects. It is the only
  record of them, because nothing checks a call's arguments against the table.
- An entry holds the code's name as well as its format. A test asserts on the
  name, so rewording a message breaks no test.
- The table is sized `ERR_CODE_COUNT` and filled with designated initializers.
  A code without an entry is refused when raised, rather than printing `NULL`.
- The table is `static` in `err.c`. `Err_Message` is the only way in.
- Compiler bugs, such as `codegen: unexpected node kind`, are codes too. Every
  error goes through the table.

### Raising

```c
// Raise an error unless a condition holds.
#define Err_Assert(cond, ...)                     \
    do {                                          \
        if (! (cond)) {                           \
            Err_Raise(__VA_ARGS__);               \
        }                                         \
    } while (0)

// Raise an error at a line unless a condition holds.
#define Err_AssertAt(line, cond, ...)             \
    do {                                          \
        if (! (cond)) {                           \
            Err_RaiseAt((line), __VA_ARGS__);     \
        }                                         \
    } while (0)

// Raising
void Err_Raise(Err_Code code, ...);
void Err_RaiseAt(uint32_t line, Err_Code code, ...);
void Err_WarnAt(uint32_t line, Err_Code code, ...);
void Err_ShowVa(Log_Severity severity, uint32_t line, Err_Code code, va_list ap);

// Inspection
Err_Code    Err_Status(void);
const char *Err_Message(Err_Code code);
```

A call site before and after:

```c
    if (bits < 0) {
        Log_ShowErrorAt(line, "a bit-field width cannot be negative");
    }

    Err_AssertAt(line, bits >= 0, ERR_PAR_BITFIELD_NEGATIVE);
    Err_AssertAt(node->an_line, given >= want, ERR_SEM_ARGS_TOO_FEW, what, given, want);
```

- The code is the first variadic argument, not a named parameter. A call to a
  variadic macro must pass more arguments than the macro names (6.10.3p4), so a
  named code would make `Err_AssertAt(line, cond, CODE)` break that rule.
- The condition is evaluated exactly once, so `Sem_Fold(width, &bits)` inside it
  is safe. The message arguments are evaluated only when the check fails, so a
  `Sem_TypeName` among them allocates nothing on the passing path.
- The condition is what must hold, as with `assert`. The check fires when it is
  false.
- `Err_Raise` and `Err_Assert` carry no line. They are for `as`, `ld` and `emu`,
  whose errors name a file or a symbol rather than a source line.
- `Err_RaiseAt` records the code, prints the diagnostic and calls `exit(1)`.
- `Err_WarnAt` prints and returns. Stage 12's `#warning` is its first user.
- All three print through `Err_ShowVa`. It formats the code's message, then
  hands `Log_Show` the message and the code's name. A line of `LOG_LINE_NONE`
  prints the program name instead of a location.
- A file that cannot be opened raises `ERR_FILE_ACCESS` with `strerror(errno)`,
  and reads `ivanas: error: x.s: No such file or directory`.
- gcc cannot see that `Err_Raise` never returns, since C99 has no way to say
  so. A local that only a failed check leaves unset must be initialized.
- Every tool's `main` calls `Log_SetProgramName(argv[0])`. `log.h` hard-coded
  `cc:`, so `ivanld` and `ivanas` reported their errors as `cc`.
- A located diagnostic takes gcc's form, which editors and build logs know how
  to jump to. It ends with the code's name in brackets, as gcc ends a warning
  with its flag:

  ```
  main.c:3: error: too few arguments to f: got 1, expected at least 2 [ERR_SEM_ARGS_TOO_FEW]
  ```

- The locator maps a line to the file and line that `Log` prints as `main.c:3`.
  Until stage 12, `cc.c` sets one that returns the input path and the line
  unchanged. Stage 12 swaps in `Pp_Locate`. With no locator set, the line
  prints as `ivancc: line 3`.
- `util/` stays free of front-end knowledge. `elf.c` depends on libc alone.

### Shutting down

`Err_RaiseAt` exits. `cc.c` registers an `atexit` handler that deletes the output
file when `Err_Status()` is not `ERR_SUCCESS`. `gen.c` can fail after
`Cc_OpenOutput` has created the file, and that file is the only cleanup worth
doing at exit.

`Err_Try`, a catch point built on `setjmp` and `longjmp`, is deferred. It would
return control to the caller after an error, and nothing needs that yet: `ivancc`
compiles one file per run and stops at the first error. Its costs are real:

- `Err_Try` must be a macro, because a `longjmp` into a function that has
  returned is undefined.
- C99 allows `setjmp` only as a whole controlling expression, or compared with a
  constant, or negated, or as a statement (7.13.1.1p4). So
  `(status = Err_Try()) == ERR_SUCCESS` is not allowed.
- A local changed between the `setjmp` and the `longjmp` is indeterminate
  afterwards unless it is `volatile`.
- A jump out of `yyparse` leaves flex and bison in a broken state.

It arrives when a caller must carry on after an error: `ivancc a.c b.c`, error
recovery that reports more than one error, or a harness inside the process.
Every error already goes through `Err_RaiseAt` by then, so the change touches
`err.c` and `cc.c` alone.

Three alternatives were weighed and dropped:

- **A raising function per error**, such as `Err_RaiseSemArgsTooFew`. It
  type-checks its arguments, but it only makes sense for errors that take
  arguments. That leaves two ways to raise an error.
- **A table of function pointers** tying codes to those functions. Their
  signatures differ, so no single pointer type fits them all.
- **Message macros that build a heap string.** They allocate on every passing
  check and hand ownership across the call.

### Order

1. Add `err.h` and `err.c` beside `log.h`.
2. Convert one module at a time: `par.c`, `sem.c`, `ast.c`, `c.flex` and `c.y`
   including `yyerror`, `gen.c`, then `as`, `ld`, `emu`, `elf.c`, `txt.c`,
   `rel.c`, `str.c` and `file.c`. The corpus stays green after each.
3. Call `Log_SetProgramName` in every `main`. Set the locator and the `atexit`
   handler in `cc.c`. Add `Log_ShowInfo` and `Log_ShowDebug` to `log.h`, and
   remove `Log_ShowErrorAt`, which has no callers left.
4. Give `run_test` its compile-failure path. `// (Test) Compiler error:` names a
   code, and the test passes when the compile fails with that name in brackets.
   Today `setup` fails on any nonzero compile, and it writes the key into
   `exp.cret.txt`, over `Compiler return`.

**Tests:** no numbered tests, like stage 3. The corpus stays green through the
move, and stage 12's `test71_pragma_error` is the first test to expect a failure.

## Stage 12 — Preprocessor

It needs stage 11's `Err` module and nothing else. It can be built at any point
after that, and the earlier it lands, the more real C we can compile.

The preprocessor is a pass of its own. It turns a source file into the text a
`.i` file would hold, kept in memory. The existing lexer and parser then read
that text unchanged. Nothing is bolted onto `c.flex`, and no temporary file is
written.

```
include/lang/pp.h
src/lang/pp.c             directives, macros, #if, includes, the line map
src/lang/syntax/pp.flex   pp-tokens, scanned under the prefix pp
include/util/str.h        Str_Buf, a growable string
src/util/str.c
libc/include/             the system include directory, new here
```

### The pipeline

1. `Fs_FileGetContents` reads the source file. It is the only reader `Pp` uses.
   `Elf_Read_Bytes` stays private to the ELF module, which depends on libc
   alone. `As_ReadSource` belongs to `ivanas` and has no other callers.
2. A pre-pass replaces trigraphs, then deletes each backslash-newline. The
   newlines it deletes come back at the next real newline, so every later line
   keeps its number. Trigraphs go first because `??/` before a newline is itself
   a splice.
3. `pp.flex` tokenizes the whole file into an array of `Pp_Token`s. Each file is
   tokenized in full before any of it is processed, so recursing into an include
   never re-enters the scanner.
4. `Pp_Run` walks the tokens. A directive runs its handler. An `#include` goes
   back to step 1 for the header. A macro name expands. Every other token is
   printed into a `Str_Buf`.
5. `Par_ParseText` hands that buffer to `c.flex` through `yy_scan_bytes`, runs
   `yyparse` and deletes the flex buffer.
6. `Sem_Analyze`, code generation and linking run as today. None of them learns
   that a preprocessor ran.

```c
    Str_Buf *text = Str_BufNew();
    Pp_Run(input, text);
    Par_ParseText(Str_BufData(text), Str_BufLen(text));
    Str_BufFree(text);
    Sem_Analyze(Ast_Program);
```

`Par_ParseText` lives in the user-code section of `c.flex`, so flex's types never
reach `cc.c`. It replaces `extern FILE *yyin` and the `fopen`/`fclose` pair. It
refuses a text longer than `INT_MAX`, because `yy_scan_bytes` takes an `int`.
`yy_scan_bytes` copies the text, and `c.flex` clones every identifier and decodes
every literal, so nothing in the AST points into the buffer. The buffer is freed
as soon as the parse ends.

`fmemopen` or `open_memstream` would have kept today's `yyin` code. They stay
rejected, because `yy_scan_bytes` reads the buffer directly and needs no stream.

`c.flex` still asks `Ast_FindTypedef` about every identifier it lexes. That keeps
working, because the lexer still runs lazily, one token per request from the
parser.

With `-E`, steps 1 to 4 run and the compile stops. The buffer goes to stdout, or
to `-o`, with line markers rebuilt from the [line map](#the-line-map).

### `Str_Buf` in `util/str`

The output's size is unknown until the pass ends. Includes splice in whole files,
macros grow or shrink the text, and skipped groups vanish. So the text is appended
to a buffer that grows. The pre-pass and the `-E` writer use the same buffer.

```c
// A growable string, kept NUL-terminated.
typedef struct Str_Buf Str_Buf;

Str_Buf    *Str_BufNew(void);
void        Str_BufFree(Str_Buf *buf);
const char *Str_BufData(const Str_Buf *buf);
size_t      Str_BufLen(const Str_Buf *buf);
void        Str_BufReserve(Str_Buf *buf, size_t n);
void        Str_BufPutByte(Str_Buf *buf, char byte);
void        Str_BufPutBytes(Str_Buf *buf, const char *data, size_t len);
void        Str_BufPutText(Str_Buf *buf, const char *text);
void        Str_BufPrint(Str_Buf *buf, const char *fmt, ...);
char       *Str_BufTake(Str_Buf *buf);
```

- The struct is opaque. Its fields live in `str.c`, and callers go through
  `Str_BufData` and `Str_BufLen`.
- `Str_BufPrint` calls `vsnprintf` twice, once to measure and once to write.
- `Str_BufTake` frees the buffer and hands its data to the caller.
- `Elf_Buffer` stays in `elf.c`. The ELF code is kept independent of the rest of
  the tree, and `Elf_Buffer` is a binary writer, not a string.

### Tokens

`pp.flex` recognises the pp-tokens of 6.4: header names, identifiers, pp-numbers,
character constants, string literals and punctuators, plus any other single
character.

- `%option prefix="pp"` renames its symbols to `pplex`, `pptext` and so on, so
  they do not clash with `c.flex`'s. The Makefile gains a second flex rule.
- A pp-number is `\.?[0-9]([0-9A-Za-z_.]|[eEpP][+-])*`. It already covers stage
  13's floating literals.
- A header name is only a token after `#include`. A start condition entered on
  the directive handles it.
- A comment becomes one space.
- An unterminated quote lexes as a lone character, not as an error. Skipped
  groups hold text like `don't`, and gcc accepts it there too.
- Digraphs are punctuators. `%:` acts as `#` and `%:%:` as `##`, both at the
  start of a directive and as operators. They keep their spelling in the output,
  as `gcc -E` does, so `c.flex` gains four rules for `<: :> <% %>`.

```c
// One preprocessing token.
typedef struct Pp_Token Pp_Token;
struct Pp_Token {
    Pp_TokenKind pt_kind;
    const char  *pt_text;
    size_t       pt_len;
    Pp_Flags     pt_flags; // PP_FLAG_BOL, PP_FLAG_SPACE
    uint32_t     pt_file;  // index into the opened-file list
    Ast_Line     pt_line;
};
```

`pt_hide` and `pt_next` joined the struct in step 2, with the first expansion.
A file's own tokens stay in its array. Only expansion tokens are heap copies,
linked through `pt_next`.

`PP_FLAG_BOL` marks a token that starts a line, which is what makes a `#` a
directive. `PP_FLAG_SPACE` marks a token with whitespace before it, which `#`
and the printer both need.

A hand-written scanner in C was the alternative. It would handle splices inline
and need no second generated file. The pp-token grammar is small and regular,
though, which is what flex is for, and the splice pass is about twenty lines.

### Macros

- Expansion uses hide sets (Prosser's algorithm). Each token carries the set of
  macros it came from, and a macro never expands inside its own hide set.
- Tokens are linked lists, so an expansion is pushed back onto the input
  cheaply. A `Pp_Reader` reads a range of a file's tokens, with pending
  expansion tokens in front. `Pp_RunFile` and `Pp_ExpandRange` both read
  through one.
- Only a token read from the file itself can open a directive. An expansion
  that starts with `#` never does (6.10.3.4p3).
- The first token of an expansion takes the macro name's flags, file and line.
  A macro that expands to nothing carries its flags to the next token, so the
  printer still breaks the line where the name started one.
- A `Pp_Macro` holds a name, its parameters, a variadic flag, its body and a
  kind: object-like, function-like or builtin. Its body points into the tokens
  of the file that defined it, so nothing is copied.
- Macros live in a table of `PP_MACRO_BUCKETS` chains, hashed with FNV-1a.
- `#undef` unlinks a macro without freeing it, because hide sets hold pointers
  to it.
- A redefinition with a different body is a warning, as in gcc. An identical
  one is silent (6.10.3p2).
- `#` stringizes an argument. It escapes `"` and `\` inside string and character
  literals, and it keeps a digraph's spelling.
- `##` pastes two spellings and lexes the result again. A paste that does not
  form one token is an error.
- An empty argument is a placemarker (6.10.3.3), so `##` next to it pastes
  nothing.
- `__VA_ARGS__` names the variadic arguments.

A disable flag per macro, set while its body is rescanned, was the simpler
alternative. It gets the nested examples of 6.10.3.5 wrong, because an argument
list can end outside the body that produced its first token.

The printer puts a space before a token when it had one in the source. It also
puts one where `Pp_NeedsSpace` says the two neighbours would lex as a single
token. With `#define f(x) -x`, the text `-f(1)` must print as `- -1`, not `--1`.
The compile reads this text, so the check is a matter of correctness, not
looks.

### Conditionals

- `#if`, `#ifdef`, `#ifndef`, `#elif`, `#else` and `#endif`, nested.
- `defined X` and `defined(X)` resolve before expansion. Every identifier left
  after expansion becomes `0`.
- The evaluator is hand-written precedence climbing. It computes in `intmax_t`
  and `uintmax_t`, as 6.10.1p4 requires, and it tracks which one each value is.
- Character constants go through `Par_CharLiteral`.
- The unevaluated operand of `&&`, `||` and `?:` is parsed but not computed, so
  `1 || 1 / 0` is not an error.
- A skipped group only tracks nesting. Its other directives are not run.

Reusing `c.y` was the alternative. Its arithmetic is `int`'s, not `intmax_t`'s,
and it would need a second start symbol.

### Includes

- A quoted include searches the including file's directory, then each `-I`
  directory, then the system directory.
- An angle include searches each `-I` directory, then the system directory.
- The system headers live in `libc/include`. The Makefile copies them to
  `build/include`, and `cc.c` searches `<exe dir>/../include`, the way
  `Cc_GetRuntimeDir` finds the runtime. `libc/include` holds no headers yet.
- `Pp_FindInclude` raises `ERR_PP_INCLUDE_NOT_FOUND` itself, so it never
  returns without a path.
- `#include_next` resumes the search after the directory the current file was
  found in. Each `Pp_File` records that directory's index.
- An operand that is neither `"..."` nor `<...>` is macro-expanded first
  (6.10.2p4). This lands in step 2, since macros do not exist before it. Until
  then such an operand raises `ERR_PP_INCLUDE_MALFORMED`. A `<...>` built this way is rebuilt from the token spellings, with
  a space where a token had one.
- The include depth is limited to 200, as in gcc.
- A `Pp_File` holds a path, its directory index, a system flag and its tokens. It
  is cached by path, so a header without a guard is not read twice.
- The preprocessor keeps the list of files it opened, in order. That list is what
  [dependency generation](#dependency-generation--m--mmd--mp) prints.
- `--include=<file>` acts as `#include "file"` placed before the first line of
  the source. Its search starts in the current directory. It is gcc's
  `-include`, spelled as a long option, because `getopt` reads a single-dash
  `-include` as the letters `-i -n -c ...`.

Re-inclusion is correct by construction. Two fast paths are optional. A header
whose whole body sits in `#ifndef X` / `#endif` is skipped while `X` is defined.
Its guard need not be defined inside. A skipped group runs nothing, so such a
header gives nothing while `X` is defined. `#pragma once` is keyed by the path
string as resolved, because `realpath` is not C99.

### The line map

`Ast_Line` keeps its type, but it now means a line of the `.i` text. The
preprocessor records one entry wherever output lines and source lines stop
advancing together: entering or leaving an include, a skipped group, a macro call
spanning lines, and `#line`.

```c
#include "inc/one.h"
#define TWO 2
int main(void) { return one() + TWO; }
```

That source, with `inc/one.h` holding `int one(void);`, becomes this text:

```c
int one(void);

int main(void) { return one() + 2; }
```

| Output line | File | Source line |
|---|---|---|
| 1 | `inc/one.h` | 1 |
| 2 | `main.c` | 2 |

- A directive leaves an empty line behind, as gcc does. So the map only needs
  an entry where continuity breaks.
- The printer pads with up to `PP_PAD_MAX` (8) blank lines to bring the output
  level with its source, as gcc does. A longer gap starts a new entry instead.
- `Pp_Locate` turns an `Ast_Line` into a file and a source line. Line 3 above
  falls under the second entry, one line further on, so it is `main.c:3`.
- `-E` rebuilds gcc's markers from the map: `# 1 "inc/one.h" 1` on entering a
  file and `# 2 "main.c" 2` on returning. `-P` omits them.

### Diagnostics

- Once `Pp_Run` has built the map, `cc.c` replaces stage 11's input-path locator
  with `Log_SetLineLocator(Pp_Locate)`. Every located error then names the
  header it came from.
- Errors raised after the parse need the map too, so it lives until the process
  exits.
- `Pp_Run` raises its own errors before the map exists. It sets
  `Pp_LocateSource` for that, which names the file being read and its line, as
  `#line` presents them.
- The preprocessor's own errors are `ERR_PP_*` codes. `#error` raises
  `ERR_PP_ERROR_DIRECTIVE`, with the directive's text as its argument.
  `#warning` goes through `Err_WarnAt` in the same way.

### Directives and predefined macros

- `#pragma once` is handled. Other pragmas are dropped. The compile reads the
  same text `-E` prints, so `-E` drops them too, where gcc would keep them.
- `_Pragma("...")` removes the quotes and escapes, then runs the result as a
  `#pragma` (6.10.9).
- `#error` prints its message and fails the compile. `#warning` prints its
  message and carries on.
- `#line` sets the next line's number, and optionally its file name (6.10.4).
  Its operands are macro-expanded.
- A lone `#` is a null directive. Any other unknown directive is an error.
- `__FILE__`, `__LINE__` and `__COUNTER__` are builtin macros.
- `__DATE__`, `__TIME__`, `__STDC__` (`1`), `__STDC_VERSION__` (`199901L`),
  `__STDC_HOSTED__` (`0`, until libc makes us hosted), `__x86_64__` and
  `__LP64__` are defined before the source is read. They are `#define` lines in
  a `<built-in>` buffer, run before the `<command line>` one.
- `__func__` is not a macro (6.4.2.2). The parser declares it the first time a
  function body names it, as a `static const char` array. Its initializer is a
  braced list of characters, because a string literal cannot initialize an
  array until [the fix after this
  stage](#between-stages-12-and-13--arrays-sized-by-their-initializer).
- `-D name`, `-D name=value` and `-U name` become `#define` and `#undef` lines in
  a `<command line>` buffer, processed in order before the source.

### Dependency generation: `-M`, `-MMD`, `-MP`

The Makefile passes `-MMD -MP` on every compile, so that editing a header
rebuilds the objects that include it. These flags are on the short list of
things `ivancc` must accept before it can build this project.

- **What the family does.** `-M` writes a make rule naming every header the
  translation unit opened; `-MM` drops the system ones. `-MD` and `-MMD` write
  that rule to a `.d` beside the object instead of to stdout, and compile
  normally at the same time, which is the form a build actually wants. `-MP`
  adds an empty phony target for each header, so deleting or renaming one does
  not leave make refusing to build with "No rule to make target".
- **Why it is not optional.** Without it, `make` compares a `.o` against its
  `.c` alone, leaves the object alone when only a header moved, and links
  objects built from disagreeing views of the same struct. Every symbol still
  resolves, so it surfaces as wrong behavior rather than as an error. That is
  the bug that made `sizeof(union Word)` answer 8 during stage 8, and the
  `Elf/` collapse and the `ast/` rename are exactly the shape of change that
  triggers it.
- **What it costs.** The opened-file list is the data, and the system flag on
  each `Pp_File` is what `-MM` and `-MMD` filter on. The rest is a printer and
  driver flags:
  - `-MF` names the output file.
  - `-MT` sets the rule's target.
  - `-MQ` sets the target, escaped for make, with `$` as `$$` and a space as
    `\ `.
- **The trap.** `cc.c` silently ignores an option it does not recognise, so
  `ivancc -MMD` today writes no `.d` and says nothing. That quietly restores the
  stale-object bug. Until step 8 lands, `cc.c` refuses every `-M` flag with an
  error. `-I`, `-D` and `-U` are ignored the same way today, so step 0 refuses
  them too, until steps 1 and 2 give them meaning. `--include` is refused until
  step 7. Any option `cc.c` does not know now prints the usage and fails.
- **`-include` is two different things.** GNU make's `-include` directive pulls
  the generated `.d` files into the Makefile, and asks nothing of us. gcc's
  `-include <file>` flag is the preprocessor feature described under
  [includes](#includes), which `ivancc` spells `--include`.

### Build order

The steps follow the test numbering. Each step closes its test and leaves the
suite green.

0. **Skeleton.** Add `pp.h`, `pp.c`, `pp.flex` and the second Makefile flex rule.
   Add `Str_Buf`, the pre-pass with trigraphs, the line map, `Pp_NeedsSpace` and
   the digraph rules in `c.flex`. Switch `cc.c` to `Par_ParseText`. Add `-E` and
   `-P`. Refuse `-I`, `-D`, `-U` and the `-M` flags. The whole corpus must pass
   through the new path unchanged. **Done.** Every `-E` output in the corpus
   matches `gcc -std=c99 -E -P` modulo whitespace.
1. **`#include`** (`test65_include`). Add both search orders, `#include_next`,
   the depth limit, the file cache and `libc/include`. Give `Pp_File` its
   directory index and system flag, and `Pp_Run` a `Pp_Options` carrying the
   `-I` list. Add the `1` and `2` flags to the `-E` markers. Stop refusing `-I`.
   **Done.** A header with no tokens before a nested include loses its own
   `1` marker to the nested one, which only `-E` output shows.
2. **Object-like macros** (`test66_define`). Add `#define` and `#undef`. Stop
   refusing `-D` and `-U`, and turn them into the `<command line>` buffer. Add
   computed includes, which `test66_define` covers. **Done.** Hide sets landed
   here too, since an object-like macro that names itself needs them. A
   function-like `#define` raises `ERR_PP_MACRO_FUNCTION_UNSUPPORTED` until
   step 3.
3. **Function-like macros** (`test67_macro_func`). Add parameters, variadic
   macros, placemarkers, `#` and `##`. Extend the hide sets with Prosser's rule
   for a call: the expansion's set is the intersection of the name's and the
   closing parenthesis's, plus the macro. Delete
   `ERR_PP_MACRO_FUNCTION_UNSUPPORTED`. **Done.** `##` re-lexes the pasted
   spelling through `Pp_LexOne` in `pp.flex`. An unterminated comment now lexes
   as `PP_TOKEN_OPEN_COMMENT`, so a paste that forms `/*` is a bad paste, not a
   bad comment. The printer also syncs on a token from a later source line, so
   the text after a call that spans lines keeps its own line. A variadic macro
   accepts a call that leaves out `...` entirely, as gcc does.
4. **Conditionals** (`test68_conditionals`). Add the directives, `defined` and
   the evaluator. **Done.** Each `Pp_RunFile` keeps its own stack of open
   conditionals, so a conditional cannot span files. A false group is skipped
   by walking directive lines to the matching `#elif`, `#else` or `#endif`. An
   `#elif` after a kept group is never evaluated. `defined` is answered while
   the line expands, so one a macro produces also works, as in gcc. The comma
   operator is accepted. Extra tokens after `#ifdef`, `#else` or `#endif` are a
   warning. A bad escape in a character constant reports no line, as it does in
   `c.flex`.
5. **Re-inclusion** (`test69_include_guard`). Add the guard fast path.
   **Done.** `Pp_FindGuard` looks for the guard once, when the file is read, and
   `Pp_RunInclude` skips the file while its guard is defined. A skipped include
   leaves no line marker. Entering or leaving a file now always starts a new
   map entry. Before, a header included twice in a row printed its second copy
   on the first copy's line.
6. **Predefined macros** (`test70_predefined`). Add the builtins, the predefined
   set, `#line` and `__func__`. **Done.** `__FILE__`, `__LINE__` and
   `__COUNTER__` are `PP_MACRO_BUILTIN` macros. `#line` shifts the line numbers
   and renames the file for the rest of the file it is in. Returning from an
   include restores the includer's. The map, the `-E` markers and the
   diagnostics all use those names and lines. A line number of zero or above
   2147483647 is an error, where gcc accepts it unless `-pedantic` is given. A
   header whose first line is `#line` shows its entry marker under the new name.
   The printer now starts a new map entry after `#line`.
7. **Pragmas and errors** (`test71_pragma_error`). Add `#pragma once`,
   `_Pragma`, `#error`, `#warning` and `--include`. Stop refusing `--include`.
8. **Dependency generation** (`test74_depend`). Add the `-M` family, and stop
   refusing it.

`test72_trigraphs` and `test73_digraphs` exercise code from step 0. They are
numbered after `test71` because they use directives, and a test may not use
syntax a higher-numbered one introduces.

### Tests

- Helper headers live in `tests/syntax/inc/`, named after their test:
  `test65_include.c` includes `"inc/test65_include.h"`. The test glob matches
  only `tests/syntax/test*.c`, and a quoted include finds them without `-I`.
- No syntax test includes or calls the standard library. So `<...>` and the
  system directory go untested here, and a later suite outside `syntax` covers
  them.
- `test71_pragma_error` expects its compile to fail with
  `ERR_PP_ERROR_DIRECTIVE`, through stage 11's compile-failure path. The pragmas
  and the `#warning` before the `#error` must get that far.
- `test74_depend` runs `-M` and checks the printed rule. It never runs a binary.
- `run_test` needs three more changes before step 8:
  - A `// (Test) Flags:` key, passed to `ivancc`.
  - A check of `// (Test) Compiler output:`, which is read into `exp.cout.txt`
    today but never compared.
  - No run of `a.out` when a test declares only compiler expectations.
- `-E` is checked against `.i` files generated once with
  `gcc -std=c99 -E -P` and checked in. The comparison collapses whitespace,
  because gcc's spacing differs from ours. gcc ignores trigraphs by default, which
  is why `-std=c99` matters.

**Tests:** `test65_include` through `test74_depend`, plus `-E` output matching
`gcc -std=c99 -E -P` modulo whitespace.

## Between stages 12 and 13 — Arrays sized by their initializer

A critical bug, found while writing `test67_macro_func`. It comes straight after
the preprocessor, unless it blocks the preprocessor first.

`[]` builds a complete array of length 0. The grammar sets `pd_empty` and leaves
`pd_len` at 0, and `Par_ApplyDerivs` calls `Ast_NewArray(inner, 0)`. That array
has size 0 and takes `at_complete` from its element type. `pd_empty` is read
only for flexible array members and for `static` without a length. So nothing
treats `T x[]` as incomplete, and nothing completes it from an initializer.

| Code | Today | C99 |
|---|---|---|
| `int r[] = {1, 2, 3};` | too many initializers for an array of 0 | length 3 (6.7.8p22) |
| `int r[][2] = {1, 2, 3};` | the same error | `int[2][2]` |
| `int r[];` in a block | accepted, 0 bytes | error: no size |
| `extern int e[]; sizeof(e)` | 0 | error: `sizeof` of an incomplete type |
| `int t[];` at file scope | a 0-byte object | tentative, `int t[1]` at the end of the unit (6.9.2p5) |

A second gap sits next to it. A string literal cannot initialize an array.
`char s[4] = "abc"` fails with `ERR_PAR_INIT_ARRAY_UNBRACED`, and
`char s[4] = { "abc" }` reaches gen and fails with `ERR_GEN_INIT_ADDRESS_WIDTH`.

The fix:

1. `[]` builds an incomplete array. `Par_CheckComplete`, `sizeof`, dereference
   and member access already read `at_complete`, so they start rejecting the
   bad cases.
2. A declaration with an initializer counts its top-level elements before it
   flattens. The count is the highest index reached plus one, so it follows
   `Par_Designate`'s cursor, not the number of items. The declaration's type is
   rebuilt with that length.
3. A string literal initializes a `char` or `wchar_t` array, braced or not. An
   unsized array takes the literal's length plus one. A sized one drops the NUL
   when the characters fill it exactly (6.7.8p14). `__func__` then takes a
   string literal instead of its list of characters.
4. A file-scope `T x[];` with no later definition becomes `T x[1]`, with a
   warning, as gcc does.

It is parser work, plus writing string data in gen. No existing test covers
it, because every test so far gives its arrays a length. Its tests take the next
free numbers after `test74_depend`, and stage 13's tests move up to make room.

## Stage 13 — Floating point

Last because it touches every layer and nothing else depends on it.

- `float` and `double`; FP literals including hex (`0x1p0`).
- A new register class. `Asm_x86_64_Reg` is a flat 16-entry enum indexing
  16-entry name tables, so xmm0–15 needs either a widened enum or a separate
  operand class.
- SSE encodings: `movsd`/`movss`, `addsd`/`subsd`/`mulsd`/`divsd`,
  `cvtsi2sd`/`cvttsd2si`, `ucomisd`, `pxor`.
- ABI: FP arguments in xmm0–7, and `%al` becomes the real count of vector
  registers used for a variadic call instead of the constant 0 we emit now.
- `printf("%f")`.
- **A decision, not a task: what `long double` is.** C99 requires only that it
  be at least as wide as `double` (5.2.4.2.2), so aliasing the two is
  conforming and costs nothing. The SysV ABI says otherwise — 80-bit x87,
  `sizeof` 16, aligned 16, passed on the stack and returned in `st(0)` — and
  matching it is the only way our objects and gcc's can pass one to each other.
  Alias now and the cost of changing later is `<float.h>`, the classification
  table and an x87 operand class; take the ABI now and stage 13 grows a second
  register file it otherwise would not need. Decide here, record it in
  SYNTAX.md, and keep stage 7's classification enum honest either way.

**Tests:** `test75_float_basic` through `test77_float_abi`.

## Stage 14 — The conformance tail: variable-length arrays

Last of the language stages, and alone, because a VLA is the one construct in
C99 that breaks an assumption every stage before it was built on: that a type's
size is known when the compiler runs. `at_size` is an `int` filled in at parse
time, and `Gen_x86_64_AssignLvarOffsets` hands every local a constant offset off
`%rbp`. Neither survives `int a[n]`.

- `Ast_Type` grows a size that may be an expression rather than a constant, and
  `Sem_*` learns which types are variably modified. The property is infectious:
  `int (*p)[n]` and a typedef of a VLA type are variably modified too.
- A dynamic frame. The object cannot live at a fixed offset, so the frame keeps
  a pointer slot, `%rsp` moves at run time to make room, and every access goes
  indirect through the slot.
- Unwinding that frame on every way out of the scope — falling off the end,
  `break`, `continue`, `goto`, `return`. This is the real cost of the stage, and
  it is the first time leaving a block has to emit anything at all.
- `sizeof` on a VLA evaluates its operand at run time, which makes `sizeof` emit
  code for the first time and makes it the one operator that is sometimes not a
  constant expression.
- VLA parameters (`void f(int n, int a[n])`), which decay to a pointer but whose
  size expression is still evaluated and discarded.
- Diagnosing a `goto` that jumps into a VLA's scope, which C forbids outright.

C11 later made VLAs optional behind `__STDC_NO_VLA__`, which is a fair measure
of how much they cost relative to what they buy. That is the argument for last,
not for never — C99 is the target and C99 requires them.

Closing this stage closes SYNTAX.md. Everything after it is library, not
language.

**Tests:** `test78_vla`, `test79_vla_sizeof`, `test80_vla_param`.

## Stage 15 — `stdio.c`, `printf()`, and libc

The first stage of the next roadmap rather than the last of this one. A C
library is the one thing that consumes every part of the language, so it can
only start once the language is finished, and by then everything it wants
already exists:

| Need | Why | Stage |
|---|---|---|
| `*p`, `&x`, pointer arithmetic, 1-byte loads | walk a format string | 1 |
| varargs access | reach the arguments after `fmt` | 2 |
| `struct` | a real `va_list` rather than `__builtin_va_arg` | 7 |
| `unsigned`, `long` | `%u`, `%x` and `%lu` mean what they say | 9 |
| `#include`, macros | `stdio.h` as a header rather than a splice | 11 |
| `double` | `%f` at all | 12 |

Which is the reason for putting it after the language rather than fourth.
Written early it
would have needed a file-splicing hack in the lexer, `__builtin_va_arg` standing
in for `va_list`, and a `printf` with holes where `%u` and `%f` belong. Written
last it needs none of that, and it doubles as the closing exam: a C library, in
our C, on our machine.

```
libc/src/x86_64/target/linux/crt0.s    _start only
libc/src/x86_64/target/linux/sys.s     write() and exit() syscall wrappers
libc/src/stdio.c                       putchar, puts, printf   (replaces libc.c)
libc/include/stdio.h                   the prototypes #include <stdio.h> pulls in
```

- `putchar` moves out of `crt0.s` and into C, which `test16_address_of` already
  made possible: `int putchar(int c) { char b = c; write(1, &b, 1); return c; }`.
- `putd` stops being public and becomes the `%d` helper.
- `write()` gets two implementations: the syscall wrapper for the host, and a
  store into the UART data register for `-mtarget=ivanemu`. Stage 3 left a
  hand-written `putchar` in each target's assembly; this is where that becomes
  one `putchar` in C over two `write`s, and where it stops being the only
  function the emulator target can offer.
- `printf` handles `%s`, `%d`, `%c` and `%%`; width, precision, flags, `%u`,
  `%x` and `%p` come later, and `%f` waits for stage 13.
- Makefile gains a `LIBC_OBJS` list per target, bundled into one
  `build/lib/<target>/libc.o` with our own `ivanld -r`. Stage 3 already does
  this for the emulator's runtime, so it keeps `Cc_RuntimeNames[]` a two-element
  array and needs no change in `cc.c`.
- `string.c` and `ctype.c` alongside it: `strlen`, `strcmp`, `strcpy`, `memcpy`,
  `memset`, `isdigit` and friends. `memcpy` and `memset` stop being optional at
  stage 7, where struct assignment starts generating calls to them.

**Tests:** `test81_putchar`, `test82_printf_int`, `test83_printf_str`, then a
Hello World program compiling as written and printing `Hello world!` through our
own `printf` — on the host and under `ivanemu`, from one binary. Those three are
the seed of a libc suite beside `tests/syntax/`, not the tail of it.

## Not in this roadmap

Stage 14 closes the C99 language. Two parts of C99 remain after it, and both
belong to the libc roadmap that stage 15 opens:

- **The hosted library.** C99 specifies 24 headers; stage 15 scopes `stdio`,
  `string` and `ctype`, and a `printf` without width, precision, `%u`, `%x` or
  `%p`. `<math.h>`, `<stdlib.h>` and an allocator, `<setjmp.h>`, `<signal.h>`,
  `<time.h>`, `<locale.h>`, `<stdint.h>`, `<inttypes.h>`, `<fenv.h>`,
  `<tgmath.h>`, `<wchar.h>` and `<wctype.h>` are all still owed. Plausibly more
  code than the compiler.
- **`_Complex` and `_Imaginary`**, with `<complex.h>`. A language feature, but
  one whose whole point is the library on top of it, so it travels with it.

So the claim at the end of stage 14 is a complete C99 *language*, and the claim
at the end of stage 15 is that plus a freestanding library — not a hosted
implementation, which is what the next roadmap is for.

---

## Infrastructure debts

Cross-cutting work that no single stage owns, but which several stages need.

- **Segment flags are hardcoded R+X.** See stage 6; also worth fixing on its own
  merits, since `.rodata` is currently mapped executable.
- **The reader drops non-PROGBITS sections.** See stage 6.
- **The instruction set is a closed enum in three places**, and `ivanemu` makes
  it four. Only one of them fails quietly, though: `Enc_x86_64_Instr` is a
  `switch` with no `default:`, so `-Wall` already reports an unhandled enumerator,
  and a decoder written the same way inherits that for free. The silent one is
  `Txt_x86_64_OpName[]`, where a missing designated initializer leaves a `NULL` —
  `Txt_x86_64_OpByName` then reports an unknown mnemonic and the text writer hands
  `NULL` to `%s`, neither of which points at the cause. Sizing that array
  `[ASM_X86_64_OP_COUNT]` and refusing a `NULL` on lookup is the whole fix.

  Deliberately no generated table. At twenty-two opcodes the duplication is one
  extra line per instruction, against X-macros putting `ASM_X86_64_OP_MOV`
  somewhere `grep` cannot find it. `-Werror=switch` is also declined: the Makefile
  passes `$(WARN)` to `$(CC)`, and `$(CC)` becomes `ivancc` the day we try to
  self-host, so every warning flag we add is one more thing our own compiler has
  to accept. Worth revisiting for an architecture with hundreds of opcodes, not
  for this one. `-MMD -MP` is the one flag added against that rule, because the
  alternative was a build that silently links stale objects; the debt it creates
  is written down under [stage 12](#dependency-generation--m--mmd--mp).
- **No `ar`.** libc is one object, so every program links all of it. `ivanld -r`
  bundling covers stage 15, but once libc is more than a few functions, archives
  with member selection are the real answer. The old roadmap already lists `ar`,
  `objcopy` and `objdump` on the horizon.
- **Two small leaks.** `Asm_x86_64_EmitSection` stores `ai_secname` without
  owning it while `Asm_x86_64_Reset` frees the other name fields, so the
  `strndup`'d name from `txt.c`'s `.section` handler is never freed; the static
  `Enc_x86_64_Labels` / `Globls` / `Fixes` arrays are never released. Both are
  once-per-process today and become real once the compiler handles more than one
  translation unit.
- **Host portability.** STYLE allows only standard C and POSIX, and three things
  fall outside it. `cc.c` uses `getopt_long`, a GNU extension, until the tools
  parse their own command lines. `Cc_GetExeDir` reads `/proc/self/exe`, which
  only Linux has. The Makefile builds with `-std=gnu99`. All three wait until the
  preprocessor is finished.
- **Command-line names follow gcc's where `getopt_long` can parse them.** A
  single letter with an argument stays a short option, as `-MMD` and `-fno-x`
  do. A multi-letter single-dash option of gcc's takes two dashes, as `--std`
  and `--include` do.

## Future roads

Not commitments. Directions that open up once SYNTAX.md is closed, written down so
the reasoning behind them does not have to be rediscovered.

### Self-hosting, against musl rather than a libc of our own

`ivancc` compiling its own 4,554 lines is a far harsher test than any number of
hand-written ones, and it is a single unambiguous criterion: the binary compiles
itself, and *that* binary compiles itself to an identical image.

The compiler side already works. `ivancc -c` output links against glibc through
the system driver, statically and dynamically, and runs. So self-hosting is a
property of the compiler, not of `ivanld` — bootstrap with the system linker and
do not block on the linker growing up first.

What the compiler consumes of a libc: `string.h` (`strcmp`, `strlen`, `strchr`,
`strcpy`, `strncmp`, `strrchr`, `strstr`, `strcspn`, `memcpy`, `memset`),
`stdlib.h` (`malloc`/`calloc`/`realloc`/`free`, 52 calls to `free` alone, plus
`strtol`, `strtoull`, `exit`), and a `FILE *` layer (`fopen`, `fread`, `fwrite`,
`fclose`, `fseek`, `ftell`, `vfprintf`, `snprintf`, `vsnprintf`). Linking a real
libc deletes the allocator and the `FILE *` layer from the critical path, which
are the two pieces that would otherwise dominate.

The `FILE *` layer is called directly from `<stdio.h>`. `util/file.h` wrapped
it until stage 11, and `util/fs.h` now holds only whole-file reads and writes.
`strdup` and `strndup` are gone, replaced by `Str_Clone` and `Str_Slice`.

One of the two non-C89 dependencies this section used to name is already gone.
`regex.h` had six call sites in `src/arch/x86_64/txt.c`, all short anchored
patterns, and they are now hand-written scanners over `<ctype.h>`. `getopt_long`
still has its one call site in `cc.c`. The rest are gathered in the next
section.

**musl, not glibc, and for the headers rather than the linking.** The hard part is
`#include <stdio.h>`, not the link step. glibc's headers lean on `__attribute__`,
`__extension__`, `__asm__` renaming, `__builtin_*` and statement expressions;
musl's are far closer to plain C. Either way this needs stages 7 through 12, which
are already planned, so it is the same work with a better payoff rather than extra
work.

**The real fork is flex and bison.** Either their generated C becomes something we
compile — plausible after stage 12, but a lot of surface — or the lexer and parser
get hand-written, which is what most self-hosting C compilers do. Worth deciding
deliberately rather than discovering.

Guard: stage 15's milestone is Hello World through *our* `printf` and *our*
linker. Keep it, and keep our libc a teaching artifact that stops growing once it
is reached. musl arriving before stage 15 would eat it.

### The host-free bootstrap: `ivancc` under `ivanemu`

The end state is `ivanemu ivancc -S main.c -o -`. Our compiler, compiled by
itself, running on our emulator. Then the emulator inside itself, so that the
only thing the toolchain asks of the host is a way to load the first image.
Self-hosting proves the compiler. This proves the whole toolchain.

Nothing in the emulator's design blocks it. What blocks it is the set of host
calls our tools make, on both sides of the boundary.

**The guest side.** `ivanemu` implements two syscalls, `write` and `exit`. A
guest `ivancc` has to read a source file and allocate, so it needs at least
`open`, `read`, `close` and `brk`. A guest `ivanemu` needs the same to load an
image. That is the larger half of the work and it is ordinary.

**The host side.** Four calls in our own source are not standard C. `chmod` in
`cc.c` and `ld.c` sets the executable bit on what they write. `readlink` on
`/proc/self/exe` in `cc.c` locates the runtime objects. `getopt_long` parses
`ivancc`'s options. `write` in `emu.c` gives a guest an unbuffered terminal,
which stdio buffering cannot. Each is one syscall stub or one hand-written loop.

**We are keeping all four.** They cost nothing while the host is Linux, and a
syscall stub beside the two we already emulate is cheaper than a standard-C
rewrite we would then have to emulate anyway. `chmod` is the clear case: ISO C
has no file-permission API at all, so the standard-only alternatives are to stop
setting the bit, which breaks `ivancc hello.c && ./a.out`, or to call
`system("chmod +x")`, which needs a shell we would also have to emulate.

They are written down because they are invisible until the day the bootstrap is
tried.

### An IR and register allocation

Code generation is a push/pop stack machine with no intermediate representation.
An IR plus real register allocation is the largest performance work available and
the most interesting, but it is invisible to correctness and nothing depends on
it. It also wants a benchmark, and self-hosting supplies the obvious one: how long
the compiler takes to compile itself. Second, therefore, not first.

### `ivanld` grows into a real linker

What linking a real static libc would actually demand, measured against glibc's
`libc.a`: 2,216 archive members, so `ar` reading plus iterative member selection;
`R_X86_64_GOTTPOFF` (1,735) and `TPOFF32` (36), so TLS — a `PT_TLS` segment,
TPOFF arithmetic and `%fs` established at startup; `GOTPCREL` (83) and
`REX_GOTPCRELX` (71), so a synthesized `.got`; 50 COMDAT `.group` sections to
deduplicate; 2,376 TLS or weak symbols, so weak semantics stop being optional;
and `.init_array` running before `main`. We handle `PC32`, `PLT32`, `32`, `32S`
and `64`. TLS cannot be dodged by linking a subset, since both glibc and musl put
`errno` there.

Worth doing on its own merits, and `ar` gets pulled in earlier anyway once libc is
more than a couple of objects. Not a prerequisite for anything above it.

### Multi-arch

A second target multiplies back-end work without deepening it, and it is much
cheaper once an IR exists than before. `ivanemu` is what makes it testable at
all, which is why the emulator comes first.

## SYNTAX.md coverage

Every box in SYNTAX.md, and the stage that closes it.

| Section | Item | Stage |
|---|---|---|
| Lexical | octal literals, integer suffixes | 9 |
| Lexical | floating-point literals | 13 |
| Lexical | full escape set, adjacent concatenation, wide literals, multi-char constants | 10 |
| Types | `short`, `long`, `long long`, `_Bool`, `signed`/`unsigned` | 9 |
| Types | `float`, `double`, `long double` | 13 |
| Types | `volatile`, `restrict` | 9 |
| Types | array declarators | 1 |
| Types | variable-length arrays | 14 |
| Types | function-pointer declarators | 8 |
| Types | `struct`, `union`, `enum`, `typedef`, bitfields, compound literals | 7 |
| Types | flexible array members | 7 |
| Declarations | multiple declarators, array and designated initializers | 6 |
| Declarations | designated initializers, nested | 7 |
| Declarations | array length from its initializer, string initializers | between 12 and 13 |
| Declarations | storage classes, `inline`, global variables | 6 |
| Expressions | unary `+`, bitwise, compound assignment, `++`/`--`, `?:`, comma | 4 |
| Expressions | address-of `&`, dereference `*`, subscript, `sizeof`, casts | 1 |
| Expressions | `.` and `->`, compound literals | 7 |
| Expressions | call through a function pointer | 8 |
| Statements | declaration as `for`-initializer, `do`-`while`, `break`, `continue` | 5 |
| Statements | `switch`/`case`/`default`, `goto` and labels | 5 |
| Functions | varargs access (`va_list`) | 2, 7 |
| Functions | function-pointer parameters and variables callable | 8 |
| Functions | qualifiers and `static` in array parameters | 8 |
| Functions | old-style (K&R) parameter lists, unprototyped declarations | 8 |
| Preprocessor | `#include` | 12 |
| Preprocessor | all other items | 12 |

## Notes

- Stages stay independently shippable, as the toolchain stages did. Once a test
  is green it stays green: `test01`–`test13` guard the language that already
  works, and no stage may regress a test an earlier stage landed.
- Stage 12 (preprocessor) depends only on stage 11 (errors). The two can be
  pulled forward together whenever real headers become more valuable than more
  syntax. Nothing before them splices a file, so pulling them forward costs
  nothing and skipping ahead to them breaks nothing.
- `Sem_*` is where semantics belong. `c.y` does two checks inline today
  (undeclared identifier, non-assignable lvalue); they move into `Sem_*` in stage
  1 and nothing new gets added to the grammar's actions.
- We keep emitting standard ELF on purpose. Cross-checking each tool against its
  GNU counterpart stays the cheapest correctness test we have.
- Stage 3 closes no SYNTAX.md box, and is a stage anyway: it is what the rest of
  the file is measured on. From it on, a stage is not done until the emulator
  decodes what the stage taught the compiler to emit.
- Syntax is the spine, but a stage owns whatever the new syntax reaches. New
  instructions mean `enc.c` and `emu.c` together, and the two are checked against
  each other and against `objdump`. New sections, symbols or relocations mean
  `elf.c` and `arch/x86_64/link.c`. `ivanas` follows whenever `-S` grows a form it cannot
  parse back. A feature that compiles but cannot be assembled, linked or run is
  not done.
