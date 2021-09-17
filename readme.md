# Titania
This is a basic procedural programming language for a testbed as I work through _Engineering a Compiler, 2/e_ by Cooper and Torrczon in 2021.

# Directories
The `src` directory holds all the source code for the language.  The `grammar` subdirectory hold the ANTLR 4 (version 4.7.1 ) grammar for the langauge along with a test file to exercise the parser.  The `include` subdirectory has the C++ header files generated for the language, along with ANTLR runtime header files.  The `lib` subdirectory hold a static library for the parser and runtime ANTLR libraries.  The `semantics` subdirectory holds code to typecheck, generate the ILOC IR, and optimize the code, along with a number of test files.


The `tools` directory holds scripts that are useful for building everything.