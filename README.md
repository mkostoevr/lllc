# Low Level pseudoLisp Compiler

Yet another attempt to write a compiler.

## The goal

Make the simplest compiler possible. BUT: No globals.

## The language
### Module system

No module system for now, this is just a single-source language.

### Root level declarations
#### Function declaration

Just declares that there's some function existing somewhere.

```C
(function functionName (*arguments: declaration list*) ReturnType)
```

Example:

```C
(function main ((argc Int) (argv *Char)) Int)
```

#### Function definition

Defines a function.

```C
(function functionName (*arguments: declaration list*) ReturnType
 (*function body: list of function calls*))
```

Example:

```C
(function main ((argc Int) (argv *Char)) Int
 ((printf "Hello, world!")
  (return 0)))
```

#### Import

Makes some declaration external and specifies its source. Example:

```C
(import messageBoxA "MessageBoxA" "USER32.DLL")
```

### Service structures
#### Declaration list

A list of 0 or more declarations of things that have name and type. Example:

```C
((windowHandle HWND) (text *char) (caption *char) (flags int))
```

#### Declaration

A declaration of thing that has name and type. Example:

```C
(message *char)
```

#### List of function calls

Just a sequence of function calls. Arithmetic operators are also functios.

```C
(*function call* *function call* ...)
```

Example:

```C
((local a)
 (local b)
 (= a 2)
 (= b 2)
 (return (+ a b)))
```

### Function call
#### In general

Everything in parentheses in a function call, but some calls just do magic instead of calculating anything, like `return`, `if` or `local` function.

#### Basic math functions

Mathematical operations (`=`, `+`, `-`, `*`, `/`, `%`) are actually functions that perform mathematical operation and return the result of the operation. Example that returns `a + b * c`:

```C
(= a_plus_b_mul_c_result (+ (a (* b c))))
```

#### Local function

Creates a local variable in current function call list. Initial value is optional. If value was not supplied than it's uninitialized and can contain garbage.

```C
(local *declaration* *initial value: function call*)
```

Example:

```C
(local sum int 0)
```

#### Return function

Returns some value from the function. Example:

```C
(return 42)
```

#### If function

Executes the first branch if first argument evaluated to `true`, else executes the second branch if it supplied. The second branch is optional.

```C
(if
 (*condition: function call*)
 (*first branch: list of function calls*)
 (*second branch: list of function calls*))
```

#### For function

Executes body from 0 to infinite ammount of times while condition results in `true`. Takes list of declarations of local variables for the body as first argument. After each execution of the body also executes postloop.

```C
(for
 (*locals: declaration list*)
 (*condition: function call*)
 (*postloop: list of function calls*)
 (*body: list of function calls*))
```

#### While function

Executes body from 0 to infinite ammount of times while condition results in `true`.

```C
(while
 (*condition: function call*)
 (*body: list of function calls*))
```
