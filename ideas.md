# code-name-titania
- procedural language inspired by 
-- Pascal
-- Modula-2
-- Oberon

## example

    module <id> <(tid <, tid>*)>?
        
        from 
            <id> = <module> (<type list>); (* instantiate a generic module *)
            <module> use <id list>;
            (* ... *)

        type
            <id> = record (* compound type *)
                <id> : <type>;
                (* ... *)
            end;
            <id> = type; (* type alias *)
                (* 
                    built in types:
                    integer
                    string
                    boolean
                    float
                    dictionary ?
                    unit/void/() ?
                *)
            <id> = ^<type>; (* pointer *)
            <id> = ^(<type> <, <type>>* ) : <type>; (* pointer to function *)
            <id> = <type> array <N>; (* arrays *)
            (* ... *)
    
        const
            <id> : <type> = <value>;
            (* ... *)
    
        function
            <id> ( <id> : <type>, ...) : <type> =
                from
                    ...
                type
                    ...
                const
                    ...
                function
                    ...
                var
                    ...
            begin
                <statement list>
                (* return from funtion written as <function id> <- <value> *)
            end;
            (* ... *)
    
        var (* only used when module is top level? *)
            <id> : <type> = <value>;
            (* ... *)
    
    begin  (* only used when module is top level, as the main entry point? *)
        <statement list>
    end <id>;

    statement list ::= statement <; statement>*

    statement ::= assignment (* <id> <- <value> *)
              ::= while expression do statement list end
              ::= case ... of | ... : ... else ... end ?
              ::= expression statement

    expression ::= if expression then expression else expression end
               ::= <id> ( expression list ) (* function call *)
               ::= arithmetic & comparison
               ::= selector (* . [] *) 
               ::= range ".." ?

# error handling
- uh, what error handling?

# modularity
- module ids can contain dots '.' to create namespaces
-- how will this interact with using dots to access fields of a (nested) record?
-- modules as a special kind of record?

# polymorphism or generics or functors
- i don't know yet.  i'd like something to cut down on source code duplication, but not
another turing complete language.
-- i think a variation on functors is called for

# function pointers?
- idk yet but probably

# build instructions
- run the config script to set up the shell
-- . tools/ConfigShell.sh
- the makefile is in the src directory

# notes
To build an ANTLR grammar to C++ code, run:
   antlr4 -Dlanguage=Cpp tatania.g4

To build an executable program, run:
    g++ -std=c++17 -I ../include/antlr4-runtime/ \*.cpp \*.cc ../lib/libantlr4-runtime.a -o main

# todo
    [x] the type checker doesn't check the body of while/do
    [x] the type checker dosen't check the then body of if/then/else
    [x] the type checker doesn't check the else body (if there is one) of if/then/else
    [] the ir generator doesn't look at #ExprStmnt (does it need to? the default is to visit the children)
    [x] the ir generator doesn't look at #FieldAccess
    [x] the ir generator doesn't look at #ArrayAccess as RHS
    [x] the ir generator doesn't look at #ArrayAccess as LHS
    [] the ir generator doesn't look at #TakeReference
    [x] the ir generator doesn't look at #ArithmaticIf
    [x] array decls should have a size component
    [x] get the absolute address of variables and constants rather than offset from ARP
    [] look up variables and constants in outer scopes
    [] what to do with non-interger-sized results
    [x] change function precall/prolouge/epilouge/postreturn--no need to reserve space
        for return value; callee pops args and pushes results
    [x] implement assign to fn name as return
    [] ir code to do record to record assignment
    [] then there's the whole returning a record from a function thing to work out
    [] abstract away from text representation of IR
    [] abstract away from text representation of registers (keep interesting details about
        them: source program value or compiler generated temporary; known or unknown value;
        constant or not)
    [x] break the IR into basic blocks
    [] manipulate basic blocks
        ## Local (in one basic block)
        [x] local value numbering
        [x] local constant folding
        [x] local expression identities (e.g., 0 + a = a, a - 0 = a, 1 x a = a)
        [] tree-height balancing
        ## Regional (between basic blocks)
        [] Suprelocal value numbering
        [] loop unrolling
        ## Global (in an entire function)
        ## Interprocedural (in the entire program)
    [x] abstract functions into their own container (i modified code buffer)
    [x] make code templates for IR generation (that didn't help as much as I thought it would)
    [] make code templates for x86-64 generation?
    [] lower IR to x86-64, then...
    [] register allocation
    [x] make an iloc interpreter?
    [x] figure out what to do with comparisons
    [] implement (literal) strings
    [] add some way of measuring the benefit of optimizations