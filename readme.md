# Why

I've got some projects that explore parsing, generating byte codes, and running them 
with a virtual machine.  Now, I want to learn more about optimizations compilers often 
perform.  I'm primarily going to follow _Engineering a Compiler, 2/e_ by Cooper and 
Torrczon.  

Since I learn best by building, I'm going to implement an optimizing compiler.  For that, I designed a simple procedural language to make it easier to follow writings on the 
subject.  One of the languages I started out programming in was Pascal, which is 
relatively straight forward.  I'm familar with other Wirth languages, like Modula-2 and 
Oberon, so I decided to follow along the path set out in those languages.  When I cook a 
recipie, I only follow it exactly the first time--after that, I always make changes.  So, 
for this language, I haven't followed any of the Wirth languages exactly--I've made some 
changes.

Now, without further ado, introducing...

# Titania
Since I consider this language a follow-up to Oberon, and in folklore he's married to Titania,
that's the name I chose.  One of the things I like about Pascal is having sections for 
various constructs. One of the things I don't like about Pascal is the distinction between
procedures and functions.  So, Titania has sections and not procedures.

Here's an example of what Titania looks like:

```pascal

      module example  (* this is a comment *)
          from   (* Import constructs from other modules *)
              std use showi 

          type
              newType = record
                      foo : integer;  (* semicolons are separators, not terminators *)
                      bar : string
                  end;

          const
              pi : integer = 3 

          function
              fact( x : integer ) : integer =  
                  begin
                      if 2 > x then fact <- 1 else fact <- x * fact( x - 1 ) end
                  end;

          var
              tau : integer <- pi * 2

      begin

          showi( fact( 5 ) )

      end

```

A few other things the novelty budget has been spent on:
- the assignment operator is spelled `<-`
- the equality operator is spelled `?=`

The file `ideas.md` has some other notes on the language along with the todo list.

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
optimize the code, along with a number of test files.  This is were most of the changes 
and additions are going to happen.

The `tools` directory holds scripts that are useful for building everything.
