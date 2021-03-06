/*
  The Titania programming language
  Copyright 2021 Eric J. Deiman

  This file is part of the Titania programming language.
  The Titania programming language is free software: you can redistribute it
  and/ormodify it under the terms of the GNU General Public License as published by the
  Free Software Foundation, either version 3 of the License, or (at your option) any
  later version.
  
  The Titania programming language is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with the
  Titania programming language. If not, see <https://www.gnu.org/licenses/>
*/

grammar titania;

file : 'module' name+=ID ( '.' name+=ID )* ( '(' params+=ID ( ',' params+=ID )* ')' )? 
            fromSection?
            typeSection?
            constSection?
            functionSection?
            varSection?
            moduleBodySection?
       'end' ;

fromSection : 'from' fromElem ( ';' fromElem )* ( ';' )? 
            ;
fromElem : ID '=' ID '(' ID ( ',' ID )* ')' #Instantiate // instantiate a module with types
         | name=ID 'use' uses+=ID ( ',' uses+=ID )*  #Import // import types, constants, functions from a module
         ;

typeSection : 'type' typeElem ( ';' typeElem )* ( ';' )? 
            ;
typeElem : typeName=ID '=' 'record' idDecl ( ';' idDecl )* 'end'   #TypeRecord // aggregate type
         | typeName=ID '=' base=ID   #TypeAlias // type alias
         | typeName=ID '=' '^' base=ID  #TypePointer // pointer type
           // pointer to function
         | typeName=ID '=' '^' '(' ( params+=ID ( ',' params+=ID )* )? ')' ':' base=ID #TypeFunctionPointer  
         | typeName=ID '=' base=ID length=NUMBER 'array' #TypeArray // array
         ;
idDecl : name=ID ':' type=ID ;
    
constSection : 'const' constElem ( ';' constElem )* ( ';' )? 
             ;
constElem : idDecl '=' expression
          ;

functionSection : 'function' functionElement ( ';' functionElement )* ( ';' )? 
                ;
                  // forward declaration of fn with only types of params
functionElement : fnName=ID '(' ( params+=ID ( ',' params+=ID )* )? ')' ':' fnType=ID   #ForwardFunction
                | fnName=ID '(' ( idDecl ( ',' idDecl )* )? ')' ':' fnType=ID '='
                    fromSection?
                    typeSection?
                    constSection?
                    varSection?
                    body  #FunctionDefinition
                ;

body : 'begin' statementList 'end' 
     ;

varSection : 'var' varElem ( ';' varElem )* ( ';' )? 
           ;
varElem : idDecl ( '<-' expression )?
        ;

moduleBodySection : 'begin' statementList 
                  ;

statementList : statement ( statement )*  
              ;

statement : lval=expression '<-' rval=expression ';'  #Assignment
          | 'while' test=expression 'do' whileBody=statementList 'end'  #WhileDo
          | 'if' test=expression 'then' thenBody=statementList ( 'else' elseBody=statementList )? 'end' #IfThen
          | expression ';' #ExprStmnt
          ;

expression : arithExpression  #ArithExp
           | logicExpression  #LogicExp
           | base=expression '.' field=ID  # FieldAccess
           | base=expression '[' index=expression ']'  #ArrayAccess
           | test=expression '?' consq=expression ':' altrn=expression  #ArithmaticIf
           | STRING  #StringLit
           | '^' expression  #DereferencePointer 
           | '@' expression  #TakeReference 
           ;

arithExpression : '-' expression  #PrefixNegative
                | '(' expression ')'  #Grouping
                | name=ID '(' ( args+=expression ( ',' args+=expression )* )? ')'  #FunctionCall
                | left=arithExpression op=( '*' | '/' | '%' ) right=arithExpression  #MultOp
                | left=arithExpression op=( '+' | '-' ) right=arithExpression  #AddOp
                | NUMBER  #NumberLit
                | ID  #Identifier
                ;

logicExpression : arithExpression  #Recusive
                | 'not' logicExpression  #NotOp
                | left=logicExpression 'and' right=logicExpression  #AndOp
                | left=logicExpression 'or' right=logicExpression  #OrOp
                | left=logicExpression 
                      op=('<' | '<=' | '?=' | '!=' | '>=' | '>') 
                      right=logicExpression  #CompOp
                | boolLiteral  #BoolLit
                ;

boolLiteral : 'true'
            | 'false'
            ;



STRING : '"' Schar* '"'
              ;

fragment
Schar : ~["\\\r\n]  // "]
      | EscapeSequence
      ;

fragment
EscapeSequence : '\\\''
               | '\\"'  
               | '\\?'
               | '\\\\'
               | '\\a'
               | '\\b'
               | '\\f'
               | '\\n'
               | '\\r'
               | '\\t'
               | '\\v'
               ;

COMMENT : '(*' .*? '*)' -> skip ;

NUMBER : '-'? DIGIT(DIGIT|'_')* ;

DIGIT : [0-9] ;

ID : [a-zA-Z_][a-zA-Z0-9_]* ;
WS : [ \t\r\n]+ -> skip ;