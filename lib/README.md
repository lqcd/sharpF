# Compatibility Libraries
                         
This directory contains #F libraries for some subsets of Scheme. Each library is implemented as a single .sf file and can be compiled and linked in the regular manner, e.g.

```
$ sfc libs.sf myprog.sf     # sfc produces 2 C files
$ gcc libs.c myprog.c       # gcc produces a.out
```

All libraries adhere to #F's minimalistic approach to error checking: run-time errors are checked with C asserts, so programs compiled in debug mode report them and exit; there is no error checking in release mode. If you need to debug your code that uses one of these libraries, you may use the corresponding interpreter with traditional error checking (interpreters are located in the `/int` subdirectory).

Please note that to dress an exisiting Scheme source file as a #F program that
uses a library like LibS, one has to add `(load "libs.sf")` line to the beginning of the
file and `(define (main argv) #f)` to the end.


## LibXXS (Extra Extra Small) Library

LibXXS (see [libxxs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libxxs.sf)) provides bare-bones Scheme-like functionality. It has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  library function `string->symbol` is also case-sensitive
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dymamic macroexpansion/compilation
  *  no support for `read`
  *  no `dynamic-wind`, so `call/cc` is faster
  *  fixnums are limited to 24 bits, generic math is fixnum-only
  *  no support for flonums/bignums/rational/complex numbers
  *  no support for `sqrt` and trigonometry functions 
  *  no support for dotted/variable-length argument lists
  *  all variable-argument and high-order primitives like `+` and `map` are macros
  *  `apply` is a macro limited to macros `+`, `*`, `list`, `append`, `vector`, `string`, `string-append` 
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL

LibXXS supports the following additional functions:

  *  many fixnum (`fx`) - specific operations
  *  `letrec*`, `let-values`, `let*-values`, `rec`, `when`, `unless` forms
  *  `fold-left`, `fold-right` forms implemented as macros
  *  `error` macro (not based on exceptions)
  *  `exit`, `abort`, and `reset`


## LibXS (Extra Small) Library

LibXS (see [libxs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libxs.sf)) is an extended version of LibXXS. It has the following additional functionality:

  *  support for flonums, generic math is mixed fixnum/flonum
  *  many flonum (`fl`) - specific operations
  *  `sqrt` and trigonometry functions are available in (`fl`) form (e.g. `flsqrt`)
  *  `andmap`, `ormap` forms implemented as macros


## LibS (Small) Library

LibS (see [libs.sf](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libs.sf)) generally targets
R^5RS feature set; most of the forms and procedures behave as expected. Compared to a regular R^5RS system, it has the following known limitations:

  *  SFC reader used to read #F source code is case-sensitive
  *  `read` and `string->symbol` are also case-sensitive
  *  there is no support for `eval` and environment functions
  *  no dynamic `load` or dymamic macroexpansion/compilation
  *  fixnums are limited to 24 bits, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  `max` and `min` do not preserve inexactness
  *  `dynamic-wind` has pre-R^6RS semantics
  *  `set!` to built-in bindings is not allowed
  *  there is no REPL and no transcript functions

In addition to R^5RS-level functionality, LibS supports some popular extensions
defined in pre-R^5RS Scheme standards, SRFIs, and R^6RS/R^7RS libraries:

  *  many fixnum (`fx`) and flonum (`fl`) - specific operations
  *  `letrec*`, `rec`, `receive`, `let-values`, `let*-values`, `when`, `unless`, `case-lambda` forms
  *  `reverse!`, `for-all`, `exists`, `fold-left`, `fold-right`
  *  `filter`, `partition`, `remq`, `remv`, `remove`, `remp`, `memp`, `assp`
  *  `list-sort!`, `list-sort`, `list-merge`, `vector-sort!`
  *  operations on boxes: `box?`, `box`, `unbox`, `set-box!`
  *  `error`, `assertion-violation` (not based on exceptions)
  *  `file-exists?`, `delete-file`, `rename-file`, `open-input-string`
  *  `exit`, `abort`, `reset`, `command-line`
  *  `get-environment-variable`, `system`, `current-jiffy`, `jiffies-per-second` 