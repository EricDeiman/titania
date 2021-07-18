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
   g++ -std=c++17 -I ../include/antlr4-runtime/ *.cpp *.cc ../lib/libantlr4-runtime.a -o main

# todo
    [] the type checker doesn't look at #ExprStmnt
    [x] the type checker doesn't check the body of while/do
    [x] the type checker dosen't check the then body of if/then/else
    [x] the type checker doesn't check the else body (if there is one) of if/then/else
    [] the ir generator doesn't look at #ExprStmnt
    [] the ir generator doesn't look at #FieldAccess
    [x] the ir generator doesn't look at #ArrayAccess as RHS
    [x] the ir generator doesn't look at #ArrayAccess as LHS
    [] the ir generator doesn't look at #TakeReference
    [] the ir generator doesn't look at #ArithmaticIf
    [] array decls should have a size component