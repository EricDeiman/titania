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

#include "codebuffer.hh"
#include "type.hh"

using Any = antlrcpp::Any;

class elaborationVisitor : public titaniaBaseVisitor {
public:

    elaborationVisitor( typeVisitor & );

    Any
    visitFile( titaniaParser::FileContext* ) override;

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
    visitVarElem( titaniaParser::VarElemContext* ) override;


    Any
    visitArrayAccess( titaniaParser::ArrayAccessContext * ) override;

    Any
    visitFieldAccess(titaniaParser::FieldAccessContext * ) override;

    Any 
    visitArithmaticIf( titaniaParser::ArithmaticIfContext * ) override;

    Any
    visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ) override;

    Any
    visitIfThen( titaniaParser::IfThenContext * ) override;

    Any
    visitWhileDo( titaniaParser::WhileDoContext * ) override;

    std::ostream&
    dumpCodeBuffer( std::ostream & );

private:

    CodeBuffer globalCodeBuffer{ true };
    std::vector< CodeBuffer > fnCodeBuffers;
    CodeBuffer* theCodeBuffer;

    std::vector<SymbolTable> 
    scopes;

    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > 
    symbolTables;

    std::pair< bool, Symbol >
    lookupId( std::string );


    // look up an ID and return the address of it, rather than the value of it
    bool asAddress = false;

    bool memoizeExprs = true;
};

#endif