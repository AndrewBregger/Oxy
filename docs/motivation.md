# Motivation
After learing about the Pratt operator parser, I felt the desire to build a language using it. The idea (kind of) is to combine the tokenizer and parser into a single operation. Such that while the compiler is parsing the source text instead of a set of tokens. 


# Implementation Details
This is implemented in C++ in a C style. The only reason it is not in C is for the use
of generics and the already implemented containers. 
