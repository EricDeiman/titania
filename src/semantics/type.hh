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

#ifndef TYPE_HH
#define TYPE_HH

#include <cstddef>
#include <iostream>
#include <ostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std::literals::string_literals;

#include "antlr4-runtime.h"
#include "ParserRuleContext.h"

#include "../include/titaniaLexer.h"
#include "../include/titaniaParser.h"
#include "../include/titaniaBaseVisitor.h"

using Any = antlrcpp::Any;

enum class symbolType {
    none,
    builtin,
    record,
    alias,
    datapointer,
    fnpointer,
    array,
    forwardFn,
    function,
    variable,
};

class Symbol {
public:
    std::string name;
    symbolType type;

    // Used by records and function parameters
    std::vector< Symbol > fields;

    // Used by alias, data pointer, array, constant, variables, functions
    std::string base;

    // Used by funtion pointers
    std::vector< std::string > fnPointerParameters;

    bool constFlag;

    // size of this type
    size_t sizeInBytes;

    // for arrays; how many elements
    size_t arrayLength;

    // for fields; the offset from the front of the record to this field.
    size_t fieldOffset;

    // for variables and constants; how far from the activation record pointer the data is
    size_t arpOffset;
    size_t lexicalNest;

    std::ostream &
    dump( std::ostream & );
};

using SymbolTable = std::unordered_map<std::string, Symbol>;
using argsTypes = std::vector< std::pair< std::string, std::string > >;

class typeVisitor : public titaniaBaseVisitor {
public:

    typeVisitor();

    Any
    visitFile( titaniaParser::FileContext* ) override;

    Any
    visitImport( titaniaParser::ImportContext* ) override;

    Any
    visitTypeRecord( titaniaParser::TypeRecordContext* ) override;

    Any
    visitTypeAlias(titaniaParser::TypeAliasContext* ) override;

    Any
    visitTypePointer( titaniaParser::TypePointerContext* ) override;

    Any
    visitTypeFunctionPointer( titaniaParser::TypeFunctionPointerContext* ) override;

    Any
    visitTypeArray( titaniaParser::TypeArrayContext* ) override;

    Any
    visitForwardFunction( titaniaParser::ForwardFunctionContext* ) override;

    Any
    visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ) override;

    Any
    visitConstElem( titaniaParser::ConstElemContext* ) override;

    Any
    visitVarElem( titaniaParser::VarElemContext* ) override;

    Any
    visitAssignment( titaniaParser::AssignmentContext* ) override; 

    Any
    visitWhileDo( titaniaParser::WhileDoContext* ) override;

    Any
    visitIfThen( titaniaParser::IfThenContext* ) override;

    Any
    visitArithExp( titaniaParser::ArithExpContext* ) override;

    Any
    visitLogicExp( titaniaParser::LogicExpContext* ) override;

    Any
    visitFieldAccess( titaniaParser::FieldAccessContext* ) override;

    Any
    visitArrayAccess( titaniaParser::ArrayAccessContext* ) override;

    Any
    visitArithmaticIf( titaniaParser::ArithmaticIfContext* ) override;

    Any
    visitStringLit( titaniaParser::StringLitContext* ) override;

    Any
    visitDereferencePointer( titaniaParser::DereferencePointerContext* ) override;

    Any
    visitTakeReference( titaniaParser::TakeReferenceContext* ) override;

    Any
    visitGrouping( titaniaParser::GroupingContext* ) override;

    Any
    visitPrefixNegative( titaniaParser::PrefixNegativeContext* ) override;

    Any
    visitFunctionCall( titaniaParser::FunctionCallContext* ) override;

    Any
    visitMultOp( titaniaParser::MultOpContext* ) override;

    Any
    visitAddOp( titaniaParser::AddOpContext* ) override;

    Any
    visitNumberLit( titaniaParser::NumberLitContext* ) override;

    Any
    visitIdentifier( titaniaParser::IdentifierContext* ) override;

    Any
    visitRecusive( titaniaParser::RecusiveContext* ) override;

    Any
    visitNotOp( titaniaParser::NotOpContext* ) override;


    Any
    visitAndOp( titaniaParser::AndOpContext* ) override;

    Any
    visitOrOp( titaniaParser::OrOpContext* ) override;

    Any
    visitCompOp( titaniaParser::CompOpContext* ) override;

    Any
    visitBoolLit( titaniaParser::BoolLitContext* ) override;

    std::ostream &
    dumpSymbols( std::ostream & );

    std::vector<SymbolTable> &&
    moveScopes();

    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > &&
    moveSymbolTables();

    int
    errorCount() {
        return errors;
    }

private:

    Symbol
    lookUpField( Symbol, std::string, bool = false );

    size_t
    sourceLine( antlr4::ParserRuleContext * );

    bool
    validId( std::string );

    bool
    validateId( std::string, std::string, size_t );

    bool
    sameType( Symbol, Symbol );

    bool
    compareArgParamTypes( std::vector< Symbol >, argsTypes );

    bool
    compareArgParamTypes( std::vector< Symbol >, std::vector< Symbol > );

    std::vector< Symbol >
    getArgsTypes( std::vector< titaniaParser::ExpressionContext* > );

    Symbol
    resolveType( std::string );

    bool
    isType( Symbol );

    bool
    validType( std::string );

    bool
    validateType( std::string, std::string, size_t );

    std::pair<bool, Symbol>
    lookUp( std::string );

    std::string
    mkLabel( std::string );

    size_t
    txNumber( std::string );

    std::vector<SymbolTable> scopes;
    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > symbolTables;

    SymbolTable moduleSymbolTable;
    std::stack< std::string >definingFunctionNames;
    std::vector< std::string > nonIdNames;

    int nameSuffix = 0;
    int errors = 0;

    // managing this with the way function decls sits between const decls and 
    // var decls needs some care.
    size_t runningArpOffset = 0;

    Symbol symInteger;
    Symbol symString;
    Symbol symBoolean;
};

#endif