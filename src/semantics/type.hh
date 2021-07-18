#ifndef TYPE_HH
#define TYPE_HH

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

    size_t sizeInBytes;

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

    std::vector<SymbolTable> scopes;
    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > symbolTables;

    SymbolTable moduleSymbolTable;
    std::stack< std::string >definingFunctionNames;
    std::vector< std::string > nonIdNames;

    int nameSuffix = 0;
    int errors = 0;

    Symbol symInteger;
    Symbol symString;
    Symbol symBoolean;
};

#endif