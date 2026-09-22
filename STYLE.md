# C Style

## Naming

- Every module must pick one short `PascalCase` prefix, roughly one per `.c`/`.h` pair.
- Every name a module exports must carry that prefix: `Elf_`, `Link_`, `Str_`, `Ast_`.
- Every public function must be named `Prefix_VerbNoun`: `Elf_BufReserve`, `Link_PlaceSections`.
- Every type must be named `Prefix_Noun`: `Link_Options`, `Ast_Node`.
- Every struct tag must match its typedef name.
- Every struct and union field must carry a short lowercase tag derived from the type name.
- Every field tag must be followed by an underscore, then the field name.
- Every `Link_Options` field must start `lo_`, and every `Ast_Node` field must start `an_`.
- Every field must stay unambiguous out of context: in a debugger, a grep, an error message.
- Every macro and enum constant must be `SCREAMING_SNAKE_CASE`, prefixed with its module or type.
- Every macro and enum constant must read like `ELF_LINK_MAX_PLACE` or `AST_NODE_KIND_ADD`.
- Every number that means something must get a name, even when the meaning looks obvious.
- Every `8` must be `EMU_X86_64_BITS_PER_BYTE` or `EMU_X86_64_STACK_SLOT`, whichever 8 it is.
- Every local variable and parameter must be short, lowercase and unprefixed.
- Do not name a function `Prefix_NounVerb`.
- Do not name 0 or 1 used as a plain count or flag.
- Do not name an index stepping through a loop.

## Layout

- Every controlled block must be braced, even a single statement.
- Every `switch` case must be a braced block written `case X: { ... } break;`.
- Every empty case body must still get its `{ }` with `// empty` inside.
- Every struct and enum member must align its name and value in a column.
- Every local declaration must have a single space after its type.
- Every line must stay within roughly 100 columns.
- Every function declaration, definition and call must stay on one line, however long it gets.
- Indent with 4 spaces.
- Write the opening `{` of a function definition on its own line.
- Write the opening `{` of an `if`, `for`, `while` or `switch` on the same line as the keyword.
- Write the `break` of a `switch` case after the closing brace.
- Write labels that share a body stacked above it, with the brace opening on the last one.
- Write negation as `! x`, never `!x`.
- Write a cast as `(type) expr`, never `(type)expr`.
- Write `sizeof` with parentheses: `sizeof(int)`, `sizeof(*item)`, `sizeof(buf)`.
- Order local declarations shortest first, where the code allows it.
  - Measure length as the type plus the name.
  - Order `int x;`, then `char *hello;`, then `unsigned long long big;`.
  - Place a local that depends on an earlier one after it, ahead of the length order.
- Do not use tabs, except where the tool requires them, as in a Makefile recipe line.
- Do not write a one-line `switch` case.
- Do not align local variables in columns.
- Do not write `sizeof buf`.
- Do not wrap a declaration, definition or call across two lines.

## Structure

- Every function must be declared in the module's header.
- Every header must read as a complete overview of what its `.c` file is and does.
- Every file-scope variable must be `static` and kept out of the header.
- Every define, enum and struct must live in the header wherever it can.
- Every `.c` file must define things in the same order its header declares them.
- Every `.c` file must follow one layout order.
  - Put includes, defines, enums and structs first.
  - Put global variables, then static global variables, then function definitions.
- Do not make a function `static`.
- Do not export a file-scope variable unless another module needs it, as with `Ast_Program`.
- Do not make a reader open a `.c` file to find out what is in it.

## Comments

What gets a comment:

- Every comment must tell the reader something the code cannot.
  - Carry intent, a non-obvious invariant, or a reason a workaround exists.
  - Carry what a name abbreviates.
- Every function must have exactly one comment directly above it, saying what it does.
- Every function comment must say why it exists or what it assumes, where the signature does not.
- Every function comment must fit one line, and wrap to a second only when genuinely necessary.
- Every function comment must be imperative.
  - Write `// Emit a REX prefix`, never `// Emits a REX prefix`.
- Every verb in a comment must be imperative, not only the first.
- Every verb after an `and` or an `or` must follow: `// Show usage information and exit`.
- Every verb with its own subject must keep that form, as in `// The section it patches`.
- Every comment that is a noun phrase must stay as it is: `// True if name was declared`.
- Every macro, typedef, struct and enum must have one comment above it.
- Every non-obvious struct field and enum constant must get a short trailing `// comment`.
- Use a section divider to group related declarations in a header.
- Use a section divider to group related phases inside a long function, as `// Phase: ...`.
- Use a section divider only where the grouping itself is information.
- Use a short inline comment at a line where the code does something surprising.
- Use an inline comment for a non-obvious flag, a deliberate deviation or an external workaround.
- Use a comment in build configuration, glue code or boilerplate only where a choice is non-obvious.

What does not get a comment:

- Do not restate the following line of code in English.
- Do not describe *what* a line does when the code already says so in plain identifiers.
- Do not comment a thing you can name well instead.
- Do not narrate normal control flow with an inline comment.
- Do not leave commented-out code.
- Do not leave TODOs-as-narration.
- Do not leave changelog-style notes such as "added X for the Y fix".
- Do not put in a source file what belongs in a commit message.
- Do not comment a block just because it looks like it needs one.
- Do not comment build configuration, glue code or boilerplate by default.
- Do not keep a comment you cannot point to a purpose for beyond the code itself.

## Documentation

- Every guide must live in `docs/`, one per stage or half-stage, named `Guide<N>[.<M>]_<Topic>.md`.
- Every guide must be written for a competent C programmer implementing the same piece themselves.
- Every guide must open with a title, one paragraph, then one short example the paragraph leans on.
- Every guide must give one `##` section per module the stage changed, in build order:
  - Log, File, Str.
  - Lexer, Parser, Ast, Sem, Elf.
  - Asm, Gen, Rel, Enc, Txt, Emu.
  - Cc, As, Ld, Emu.
  - Tests.
- Every `##` section must open with one paragraph, then give one `###` per piece of the work.
- **Every `###` must be a heading, then one paragraph, then one code block, in that order.**
- Every `###` must end at its code block.
- Every Tests section must give one `###` per test, showing that test's code.
- Every code block must be ordinary code, as it stands once written.
- Write the guide after implementing the stage, never before.
- Write what to build and why it is built that way.
- Write the traps, the orderings that matter and the mistakes that stay silent.
- Write plain, active, present tense, addressed to the reader.
  - "Add `Sem_CheckByValue()` to reject the struct values the ABI cannot move yet."
- Write one idea per sentence.
- Write a note in the prose where a piece replaces something a compiler already has.
- Use a colon before a list or an explanation.
- Correct a stale guide deliberately, and all at once.
- Do not give a `##` section to a module the stage did not touch.
- Do not document renames, module splits, moved files or named constants.
- Do not document anything but the language the compiler accepts.
- Do not write a sentence that needs this compiler's source to make sense.
- Do not write prose after a code block.
- Do not write two paragraphs or two code blocks under one `###`.
- Do not write a diff.
- Do not write a test's filename in place of its code.
- Do not use the editorial we.
- Do not join two statements into one.
- Do not use em dashes.
- Do not edit a guide because a later refactor made it stale.
