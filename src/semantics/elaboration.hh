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

    elaborationVisitor( typeVisitor &, bool = false, bool = false );

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
    dumpCodeBuffers( std::ostream & );

    std::vector< CodeBuffer >&&
    getCodeBuffers();

private:

    CodeBuffer globalCodeBuffer{ "!", true };
    std::vector< CodeBuffer > fnCodeBuffers;
    CodeBuffer* cb;

    std::vector<SymbolTable> 
    scopes;

    std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > 
    symbolTables;

    std::pair< bool, Symbol >
    lookupId( std::string );

    std::string
    to_str( size_t );

    // look up an ID and return the address of it, rather than the value of it
    bool asAddress = false;

    bool memoizeExprs = true;

    std::string currentFnName;
    std::string currentFnExitLabel;
    std::string fnReturnReg;
    bool seeCurrentFnId = false;
    bool inAssignStmnt = false;

    bool lineNo;
    bool arpOffset;
};

#endif
