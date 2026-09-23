# Syntax

Checklist toward ISO C99.

## Lexical

- [x] `//` line comments
- [x] `/* */` block comments
- [x] identifiers
- [x] decimal integer literals
- [x] hex integer literals (`0x...`)
- [ ] octal integer literals (`0...`)
- [ ] integer suffixes (`u U l L ll LL` and combinations)
- [ ] floating-point literals (`1.0`, `1e10`, `0x1p0`)
- [x] character literals
- [ ] full escape set (`\a \b \f \v \xHH`, octal `\nnn`, `\uXXXX`/`\UXXXXXXXX`)
- [x] string literals
- [ ] adjacent string literal concatenation (`"a" "b"`)
- [ ] wide literals (`L"..."`, `L'x'`)
- [ ] multi-character constants (`'ab'`)

## Types

- [x] `int`, `char`, `void`
- [ ] `short`
- [ ] `long`, `long long`
- [ ] `float`, `double`
- [ ] `long double` (80-bit x87, or aliased to `double` — both conform)
- [ ] `_Bool`
- [ ] `signed` / `unsigned`
- [x] `const` qualifier
- [ ] `volatile` qualifier
- [ ] `restrict` qualifier
- [x] pointer declarators (`int *p`, any number of stars)
- [x] array declarators, incl. multi-dimensional (`[N][M]`)
- [ ] variable-length arrays (`int a[n]`, `sizeof` of one evaluated at run time)
- [ ] function-pointer declarators
- [x] `struct`
- [x] `union`
- [x] `enum`
- [x] `typedef`
- [ ] bitfields
- [x] flexible array members (`struct s { int n; char d[]; }`)
- [x] compound literals (`(T){ ... }`)

## Declarations

- [x] single declarator with optional initializer
- [x] multiple declarators per statement (`int a, b, c;`)
- [x] array initializers (`{1, 2, 3}`)
- [x] designated initializers (`[i] = v` and `.field = v`)
- [x] designated initializers, nested (`[1].f[2] = v`, and brace elision around them)
- [x] storage classes: `static`, `extern`, `register`, `auto`
- [x] `inline`
- [x] top-level (global) variable declarations
- [x] function prototypes (parsed; currently a no-op)

## Expressions and operators

- [x] arithmetic: `+ - * / %`
- [x] relational: `< > <= >=`
- [x] equality: `== !=`
- [x] logical: `&& ||`
- [x] unary `-` and `!`
- [x] unary `+`
- [x] bitwise: `& | ^ ~ << >>`
- [x] compound assignment: `+= -= *= /= %= &= |= ^= <<= >>=`
- [x] increment / decrement: `++ --` (prefix and postfix)
- [x] ternary `?:`
- [x] comma operator
- [x] assignment `=` (to a variable, `*p`, or `a[i]`)
- [x] address-of `&` (unary)
- [x] dereference `*` (unary)
- [x] array subscript `a[i]`
- [x] struct/union member access `.` and `->`
- [ ] call through a function pointer
- [x] `sizeof` (type and expression forms)
- [x] cast expressions (`(T) expr`)
- [x] function calls, fixed arity
- [x] compound literals in expression position
- [x] parenthesized expressions

## Statements

- [x] expression statement
- [x] empty statement (`;`)
- [x] compound statement / block
- [x] `if` / `else`
- [x] `while`
- [x] `for` (expression-only init; no `for (int i = 0; ...)`)
- [x] declaration as a `for`-loop initializer
- [x] `do`-`while`
- [x] `switch` / `case` / `default`
- [x] `break`
- [x] `continue`
- [x] `goto` and labels

## Functions

- [x] definitions with a fixed parameter list
- [x] variadic marker `...` (arguments reachable via `__builtin_va_arg`, no `va_list`)
- [x] prototypes (parsed, no-op)
- [ ] function-pointer parameters/variables actually callable
- [ ] qualifiers and `static` in array parameters (`int a[static 4]`, `int a[const 4]`)
- [ ] old-style (K&R) parameter lists (`int f(a, b) int a; char b; { ... }`)
- [ ] unprototyped declarations (`int f();`) and default argument promotions

## Preprocessor

- [ ] `#include`
- [ ] `#define` (object-like)
- [ ] `#define` (function-like, incl. variadic macros)
- [ ] `#undef`
- [ ] `#if` / `#ifdef` / `#ifndef` / `#elif` / `#else` / `#endif`, `defined()`
- [ ] `#pragma`
- [ ] `#error` / `#warning`
- [ ] token pasting (`##`) and stringizing (`#`)
- [ ] predefined macros (`__FILE__`, `__LINE__`, `__func__`, ...)
