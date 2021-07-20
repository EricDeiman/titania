#ifndef ELABORATION_HH
#define ELABORATION_HH

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>

#include "antlr4-runtime.h"
#include "ParserRuleContext.h"

#include "../include/titaniaLexer.h"
#include "../include/titaniaParser.h"
#include "../include/titaniaBaseVisitor.h"

#include "type.hh"

using Any = antlrcpp::Any;

class elaborationVisitor : public titaniaBaseVisitor {
public:

    elaborationVisitor( typeVisitor & );

    Any
    visitIdentifier( titaniaParser::IdentifierContext* ) override;

    Any
    visitNumberLit( titaniaParser::NumberLitContext* ) override;

    Any
    visitAddOp( titaniaParser::AddOpContext* ) override;

    Any
    visitMultOp( titaniaParser::MultOpContext* ) override;

    Any
    visitPrefixNegative( titaniaParser::PrefixNegativeContext* ) override;

    Any
    visitGrouping( titaniaParser::GroupingContext* ) override;

    Any
    visitFunctionCall( titaniaParser::FunctionCallContext* ) override;

    Any
    visitAssignment( titaniaParser::AssignmentContext* ) override;

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

    Any
    visitConstElem( titaniaParser::ConstElemContext* ) override;

    Any
    visitArrayAccess( titaniaParser::ArrayAccessContext * ) override;

    Any
    visitFieldAccess(titaniaParser::FieldAccessContext * ) override;

    Any
    visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ) override;

    Any
    visitIfThen( titaniaParser::IfThenContext * ) override;

    Any
    visitWhileDo( titaniaParser::WhileDoContext * ) override;

private:

    std::string
    getFreshRegister();

    // CC stands for condition code
    std::string
    getFreshCCRegister();

    std::string
    makeLabel( std::string );

    std::vector< std::string >
    makeLabel( std::vector< std::string > );

    void
    writeCodeBuffer( std::vector< std::string > );

    void
    dumpCodeBuffer( std::string, antlr4::ParserRuleContext * );

    std::vector< std::string > codeBuffer;
    std::unordered_map< std::string, std::string > valuesMap;
    std::vector< std::unordered_map< std::string, std::string > >valuesScopes;

    size_t
    valuesScopesCount( std::string );

    std::string
    valuesScopesLookup( std::string );

    std::vector<SymbolTable> 
    scopes;

    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > 
    symbolTables;

    std::pair< bool, Symbol >
    lookupId( std::string );


    int registerNum = 0;

    // look up an ID and return the address of it, rather than the value of it
    bool asAddress = false;

    int ccNum = 0;
    int labelSuffix = 0;

    bool memoizeExprs = true;
};

#endif