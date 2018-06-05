## Introduction and purpose   

This repository represents a semestral work for a college course. Since during
its development I found that LLVM's C API documentation is not very
straightforward (basically you find function declarations - but not really how
they relate to one another), I decided to make this repository public. In that
sense, the most interesting part of the repository is the code generation
interface to LLVM.

The code is developed against LLVM 4.0.1, but it should be pretty similar to
other versions. At least, it should give you a general idea of the usage of the
API.   

There is not much reason for doing this in C, unless you have a very specific
use case. I thought it'd be fun.    

## Limitations   

This code parses a small subset of Pascal. Namely, it supports nested blocks,
functions, procedures, recursion, indirect recursion, if, while, for structures,
and integer data types. Arrays are on the parser but they are not recognized on
the code generation part yet.   

Since we only use integers (aside from literals), we don't really do much type
checking. If you wanted to do it, you may need to add some structures that serve
that purpose. 

## Organization of the code  

The formal grammar is described in `grammar.md`. In `libs/` you will find the
important part of the code, and if you are interested in the API, just skip to
`libs/codegen.c`.

## Useful links  

[Current LLVM-C documentation](http://llvm.org/doxygen/group__LLVMC.html) at
llvm.org.   

