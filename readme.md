# Titania
This is a basic procedural programming language for a testbed as I work through _Engineering a Compiler, 2/e_ by Cooper and Torrczon in 2021.

# Building
- This is being developed on a Linux machine and assumes a Unix-like command line build 
environment
1. Run `. tools/ConfigShell.sh` file to set up the environment variables
1. Run the makefile in the `grammar` directory to build the parser and related files
1. Run the makefile in the `semantics` directory to build the program executables
   1. The command `tc` will compile a titania file into iloc
   1. The command `ti` will interpret an iloc file
   1. The command `topt` will perform optimizations on an iloc file
      1. Creates file `<source-name>.cfg.dot` to draw the control flow graph
      1. Creates file `<souce-name>.ebbs.dot` to draw the extended basic blocks

# Directories
The `src` directory holds all the source code for the language.  

The `src/grammar` subdirectory hold the ANTLR 4 (version 4.7.1 ) grammar for the langauge 
along with a test file to exercise the parser.  The grammar described here is not fully 
implemented yet.  Code that conforms to the grammar will type check, but the only built-in
data types are integer and boolean.  Importing names from other modules works, but not 
importing functionality.

The `src/include` subdirectory has the C++ header files generated for the language, along 
with ANTLR runtime header files.  

The `src/lib` subdirectory hold a static library for the parser and runtime ANTLR 
libraries.  

The `src/semantics` subdirectory holds source to typecheck, generate the ILOC IR, and 
optimize the code, along with a number of test files.  This is were most of the changes and
additions are going to happen.

The `tools` directory holds scripts that are useful for building everything.
