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
- Every macro constant and enum constant must be `SCREAMING_SNAKE_CASE`.
- Every macro constant and enum constant must be prefixed with its module or type.
- Every macro constant and enum constant must read like `ELF_LINK_MAX_PLACE`.
- Every function-like macro must be named as a function, as `Log_ShowErrorAt` is.
- Every name fixed by an external spec must keep that spec's spelling, as `R_X86_64_PC32` does.
- Every magic number must be named by a `#define` or an enum constant.
- Every 0, 1 and NULL that stands for a quantity must be named too, as `AST_TYPE_SIZE_CHAR` is.
- Every name must identify which quantity it is, so two different `8`s never share one name.
- Every local variable and parameter must be short, lowercase and unprefixed.
- Do not name a function `Prefix_NounVerb`.
- Do not leave a number bare unless it means only itself, as a loop index does.

## Layout

- Every controlled block must be braced, even a single statement.
- Every `switch` case must be a braced block written `case X: { ... } break;`.
- Every empty case body must still get its `{ }` with `// empty` inside.
- Every struct and enum member must align its name and value in a column.
- Every local declaration must have a single space after its type.
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
- Every header must be guarded by `#ifndef <FILE>_H` and `#define <FILE>_H`.
- Every enum and struct must be typedef'd on the line above its definition.
- Every define, enum and struct must live in the header wherever it can.
- Every file-scope variable must be `static` and kept out of the header.
- Every `.c` file must define things in the same order its header declares them.
- Every `.c` file must include system headers first, then a blank line, then project headers.
- Every project include must carry its module path: `#include "ast/ast.h"`.
- Every `.c` file must follow one layout order.
  - Put includes, defines, enums and structs first.
  - Put global variables, then static global variables, then function definitions.
- Do not make a function `static`.
- Do not export a file-scope variable unless another module needs it, as with `Ast_Program`.

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

- Do not describe *what* a line does when the code already says so in plain identifiers.
- Do not comment a thing you can name well instead.
- Do not narrate normal control flow with an inline comment.
- Do not leave commented-out code.
- Do not leave TODOs-as-narration.
- Do not leave changelog-style notes such as "added X for the Y fix".
- Do not comment a block just because it looks like it needs one.
- Do not comment build configuration, glue code or boilerplate by default.
- Do not keep a comment you cannot point to a purpose for beyond the code itself.

## Documentation

- Files:
  - Every stage must have a guide in `docs/`, named `Guide<N>_<Topic>.md`.
  - Every stage guide must open with a title and the paragraphs saying what the stage implements.
  - Every stage guide must name its parts in a paragraph of links, then end with the target program.
  - Every module a stage changed must have a part file, named `Guide<N>.<M>_<Module>.md`.
  - Every part must be numbered in the order the toolchain is built:
    - Log, File, Str.
    - Lexer, Parser, Ast, Sem, Elf.
    - Asm, Gen, Rel, Enc, Txt, Emu.
    - Cc, As, Ld, Emu.
    - Tests.
  - Extend the existing part files in a later phase of a stage, rather than adding new ones.
  - Do not give a part file to a module the stage did not touch.
- Structure:
  - Every part file must open with `## <Module>` followed by one paragraph.
  - Every piece of the work must be a `###` heading naming its subject as a noun phrase.
  - Every `###` heading must be two to four words long.
  - **Every `###` must run: introduction, concept paragraphs, mechanical paragraphs, code block.**
  - Every `###` must end at its code block.
  - Every Tests part must give one `###` per test, showing that test's code.
  - Do not write prose after a code block.
- Paragraphs:
  - Every subsection must open with a paragraph introducing the concepts the rest of it uses.
  - Every paragraph after that introduction must defend or explain the code block.
  - Every concept and every solution must get a paragraph of its own.
  - Every rule, function, field or case the code block names must get a paragraph of its own.
  - Every loop, branch or step inside a shown function must get a paragraph of its own.
  - Every paragraph must hold two or three sentences.
  - Every paragraph must run to between 20 and 55 words.
  - Every paragraph must occupy one line, however long that line gets.
  - Every paragraph must do one job, and the jobs are these:
    - Introduce a concept.
    - Recognise a problem.
    - Propose a solution.
    - Show an example.
    - Explain a concept or a solution.
  - Do not wrap a paragraph across two lines.
  - Do not write a paragraph of one sentence.
  - Do not refer back, since a paragraph refers forward to the code block below it.
- Sentences:
  - Every sentence must do one of the jobs a paragraph does.
  - Use a connective to name the relation to the sentence before it.
  - Write "For example" before an illustration and "However" before a qualification.
  - Delete a sentence that neither motivates the code block nor explains part of it.
  - Do not join two jobs into one sentence with ", and".
- Voice:
  - Write plain, active, present tense prose.
  - Write the compiler part as the subject of the sentence.
  - Write "we" only where a choice is being made rather than a fact stated.
  - Use an Oxford comma in a list of three or more.
  - Use a colon before a list or an explanation.
  - Do not write an imperative sentence.
  - Do not use em dashes.
- Code blocks:
  - Every code block must be tagged `c`.
  - Every code block must show ordinary code as it stands once written.
  - Use a comment on its own line to label a group of lines.
  - Use a trailing comment, aligned in a column, to annotate one line.
  - Use a commented-out line to show the surrounding code a block does not change.
  - Use `/* ... */` on its own line to elide code that stays as it was.
  - Do not put in a code comment what belongs in the prose.
  - Do not put a file path in a code block.
- Scope:
  - Write the guide after implementing the stage, never before.
  - Write for a competent C programmer implementing the same piece themselves.
  - Write what to build and why it is built that way.
  - Write the traps, the orderings that matter and the mistakes that stay silent.
  - Correct a stale guide deliberately, and all at once.
  - Do not document renames, module splits, moved files or named constants.
  - Do not write a sentence that needs this compiler's source to make sense.
  - Do not edit a guide because a later refactor made it stale.
