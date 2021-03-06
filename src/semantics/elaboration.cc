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


#include "elaboration.hh"
#include "ir.hh"
#include "mustache.hh"
#include "type.hh"

#include <cassert>
#include <fstream>
#include <string>

using namespace std;

elaborationVisitor::elaborationVisitor( typeVisitor &type, bool lineNo, bool arpOffset ) {
    scopes = type.moveScopes();
    symbolTables = type.moveSymbolTables();

    cb = &globalCodeBuffer;

    lineNo = lineNo;
    arpOffset = arpOffset;
}

Any
elaborationVisitor::visitFile( titaniaParser::FileContext* ctx ) {

    scopes.push_back( symbolTables[ ctx ] );

    // set up the stack to look like a function call has been made even though there
    // isn't one
    cb->writeCodeBuffer( { "pushi 0  # phoney global access link" } );
    cb->writeCodeBuffer( { "pushi 0  # phoney global ARP" } );
    cb->writeCodeBuffer( { "pushi 0  # phoney global return address" } );
    cb->writeCodeBuffer( { "pushi 0  # phoney global return value" } );

    cb->writeCodeBuffer( { "i2i rtos => rarp  # set up initial rarp" } );

    visitChildren( ctx );

    cb->writeCodeBuffer( { "hlt  # end the program" } );
    scopes.pop_back();

    fnCodeBuffers.push_back( move( globalCodeBuffer ) );

    return "";
}

Any
elaborationVisitor::visitIdentifier( titaniaParser::IdentifierContext *ctx ) {
    string result;
    string id{ ctx->ID()->getText() };

    string reg1;

    // If we see the current function id as the LHS of an assignment statement, the 
    // statement acts like a return statement
    if( id == currentFnName && inAssignStmnt && asAddress ) {
        seeCurrentFnId = true;
        return fnReturnReg;
    }
    else {
        seeCurrentFnId = false;
    }

    auto idSymbol = lookupId( id );

    if( arpOffset ) {
        cb->writeCodeBuffer( { "# . . . . . . . .  offset of ", id, " from ARP is ",
            to_str( idSymbol.second.arpOffset ), " and lexical level is ", 
            to_str( idSymbol.second.lexicalNest), " (current lexical level is ",
            to_str( scopes.size() ), ")"
        } );
    }

    auto framesBack = scopes.size() - idSymbol.second.lexicalNest;

    if( cb->valuesScopesCount( "@" + id ) > 0 ) {
        reg1 = cb->valuesScopesLookup( "@" + id );
    }
    else {
        reg1 = cb->getFreshRegister();
        cb->writeCodeBuffer( { "addi rarp, ", to_str( idSymbol.second.arpOffset ), 
            " => ", reg1 } );
        cb->valuesScopes.back()[ "@"+ id ] = reg1;
    }

    if( asAddress ) {
        result = reg1;
    }
    else {
        string reg2 = cb->getFreshRegister();
        cb->writeCodeBuffer( { "load ", reg1, " => ", reg2 } );
        result = reg2;
    }

    return result;
}

Any
elaborationVisitor::visitNumberLit( titaniaParser::NumberLitContext* ctx ) {
    string result;
    string lit{ ctx->NUMBER()->getText() };

    if( cb->valuesScopesCount( lit ) > 0 ) {
        result = cb->valuesScopesLookup( lit );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi ", lit, " => ", reg } );
        cb->valuesScopes.back()[ lit ] = reg;
        result = reg;
    }

    return result;
}

Any
elaborationVisitor::visitAddOp( titaniaParser::AddOpContext* ctx ) {
    auto left = static_cast< string >( visit( ctx->left ) );
    auto right = static_cast< string >( visit( ctx->right ) );
    auto op = ctx->op->getText();
    string result, insr;

    switch( op[ 0 ] ) {
        case '+':
            insr = "add ";
            break;
        case '-':
            insr = "sub ";
            break;
    }

    auto key = left + insr + right;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        result = cb->valuesScopesLookup( key );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitMultOp( titaniaParser::MultOpContext* ctx ) {
    auto left = static_cast< string >( visit( ctx->left ) );
    auto right = static_cast< string >( visit( ctx->right ) );
    auto op = ctx->op->getText();
    string result, insr;

    switch( op[ 0 ] ) {
        case '*':
            insr = "mult ";
            break;
        case '/':
            insr = "div ";
            break;
        case '%':
            insr = "mod ";
            break;
    }

    auto key = left + insr + right;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        result = cb->valuesScopesLookup( key );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
         cb->valuesScopes.back()[ key ] = reg;
     }
    }

    return result;
}

Any
elaborationVisitor::visitPrefixNegative( titaniaParser::PrefixNegativeContext* ctx ) {
    auto expr = static_cast< string >( visit( ctx->expression() ) );
    string result;

    auto key = "-1*" + expr;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        result = cb->valuesScopesLookup( key );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { "multi ", expr, " -1 => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            cb->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitFunctionCall( titaniaParser::FunctionCallContext* ctx ) {
    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ function call on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    mustache passLocalAccessLink{
        {
            "subi rarp, 32 => {treg1} #  use my access link",
            "load {treg1} => {treg2}",
            "push {treg2}",
        }
    };

    mustache manipulateARP {
        {
            "push rarp",
            "pushi @{returnAddr}",
            "i2i rtos => rarp  # set up callee arp",
        }
    }; 

    mustache callPostcall{
        {
            "{returnAddr}:",
            "pop {fnResultReg}  # return value",
            "pop  # return address",
            "pop rarp",
            "pop  # access link",
            // no saved registers yet
        }
    };

    vector< string >stdLib{ "showi", "showb", "shows" };

    auto fnId = ctx->name->getText();

    auto fnSymbol = lookupId( fnId );
    assert( fnSymbol.first );
    auto fnBaseSymbol = lookupId( fnSymbol.second.base );

    string fnReg;

    if( find( stdLib.begin(), stdLib.end(), fnId ) == stdLib.end() ) {
        if( memoizeExprs && cb->valuesScopesCount( "@" + fnId ) > 0 ) {
            fnReg = cb->valuesScopesLookup( "@" + fnId );
        }
        else {
            fnReg = cb->getFreshRegister();
            cb->writeCodeBuffer( { "loadi @", fnId, " => ", fnReg } );
            if( memoizeExprs ) { 
                cb->valuesScopes.back()[ "@" + fnId ] = fnReg;
            }
        }
    }

    auto returnAddr = cb->makeLabel( fnId + "_return" );

    // ------- precall
    // registers saved
    // access link
    // caller's ARP
    // return address
    // return value
    // paramaters

    // no saved registers yet

    if( scopes.size() == 2 ) {
        cb->writeCodeBuffer ( { "push rarp  # global access link" } );
    }
    else {
        dictionary replaceAccessLink {
            { "treg1", cb->getFreshRegister() },
            { "treg2", cb->getFreshRegister() },
        };

        passLocalAccessLink.populate( replaceAccessLink, *cb );
    }

    dictionary replaceManipulateARP {
        { "returnAddr", returnAddr },
    };

    manipulateARP.populate( replaceManipulateARP, *cb );

    for( auto x = ctx->args.rbegin(); x != ctx->args.rend(); x++ ) {
        auto res = static_cast< string >( visit( *x ) );
        cb->writeCodeBuffer( { "push ", res } );
    }

    if( find( stdLib.begin(), stdLib.end(), fnId ) != stdLib.end() ) {
        // handle the primative functions here 
        cb->writeCodeBuffer( { fnId } );
    }
    else {
        cb->writeCodeBuffer( { "call ", fnReg } );
    }

    auto fnResultReg = cb->getFreshRegister();
    dictionary replaceCallReturn{
        { "fnResultReg", fnResultReg },
        { "returnAddr", returnAddr },
    };

    callPostcall.populate( replaceCallReturn, *cb );

   return fnResultReg;
}


Any
elaborationVisitor::visitGrouping( titaniaParser::GroupingContext* ctx ) {
    return( visit( ctx->expression() ) );
}

Any
elaborationVisitor::visitRecusive( titaniaParser::RecusiveContext* ctx ) {
    return visit( ctx->arithExpression() );
}

Any
elaborationVisitor::visitNotOp( titaniaParser::NotOpContext* ctx ) {
    auto expr = static_cast< string >( visit( ctx->logicExpression() ) );

    string result;

    auto key = "not" + expr;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        result = cb->valuesScopesLookup( key );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { "not ", expr, " => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            cb->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

// false is zero, true is any non-zero value
// Short-circut evaluation: evaluate the LHS, if the result of the expression is known,
// don't evaluate the RHS
Any
elaborationVisitor::visitAndOp( titaniaParser::AndOpContext* ctx ) {

    // Evaluate the LHS
    // if the result of the LHS is false, the entire expression is false
    mustache lhsCode {
        {
            "i2i {left} => {result}",
            "comp {r_false}, {left} => {cc}",
            "cbreq {cc} -> @{end}, @{rhs}",
            "{rhs}:", // Otherwise, the entire expression is the result of the RHS
        }
    };

    // Otherwise, the entire expression is the result of the RHS
    mustache rhsCode {
        {
            "i2i {right} => {result}",
            "{end}:"
        }
    };

    auto labels = cb->makeLabel( { "andEnd", "rhs", "" } );
    auto result = cb->getFreshRegister();

    dictionary replacements {
        { "left", static_cast< string >( visit( ctx->left ) ) },
        { "result", result },
        { "r_false", cb->valuesScopesLookup( "false" ) },
        { "cc", cb->getFreshCCRegister() },
        { "end", labels[ 0 ] },
        { "rhs", labels[ 1 ] },
    };
    
    lhsCode.populate( replacements, *cb );

    replacements[ "right" ] = static_cast< string >( visit( ctx->right ) );

    rhsCode.populate( replacements, *cb );

    return result;
}

Any
elaborationVisitor::visitOrOp( titaniaParser::OrOpContext* ctx ) {
 
    mustache lhsCode {
        {
            "i2i {left} => {result}",
            "comp {r_false}, {left} => {cc}",
            "cbrneq {cc} -> @{end}, @{rhs}",
            "{rhs}:", // Otherwise, the entire expression is the result of the RHS
        }
    };

    mustache rhsCode {
        {
            "i2i {right} => {result}",
            "{end}:"
        }
    };

    auto labels = cb->makeLabel( { "orEnd", "rhs", "" } );
    auto result = cb->getFreshRegister();

    dictionary replacements {
        { "left", static_cast< string >( visit( ctx->left ) ) },
        { "result", result },
        { "r_false", cb->valuesScopesLookup( "false" ) },
        { "cc", cb->getFreshCCRegister() },
        { "end", labels[ 0 ] },
        { "rhs", labels[ 1 ] },
    };
 
    lhsCode.populate( replacements, *cb );

    replacements[ "right" ] = static_cast< string >( visit( ctx->right ) );

    rhsCode.populate( replacements, *cb );

    return result;
}

Any
elaborationVisitor::visitCompOp( titaniaParser::CompOpContext* ctx ) {
    auto left = static_cast< string >( visit( ctx->left ) );
    auto right = static_cast< string >( visit( ctx->right ) );
    auto op = ctx->op->getText();

    string insr;

    if( op == "<" ) {
        insr = "cmplt";
    }
    else if( op == "<=" ) {
        insr = "cmple";
    }
    else if( op == "?=" ) {
        insr = "cmpeq";
    }
    else if( op == "!=" ) {
        insr = "cmpne";
    }
    else if( op == ">=" ) {
        insr = "cmpge";
    }
    else {
        insr = "cmpgt";
    }

    string result;

    auto key = left + insr + right;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        result = cb->valuesScopesLookup( key );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { insr, " ", left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitBoolLit( titaniaParser::BoolLitContext* ctx ) {
    auto value = ctx->getText();
    string result;

    if( memoizeExprs && cb->valuesScopesCount( value ) > 0 ) {
        result = cb->valuesScopesLookup( value );
    }
    else {
        auto reg = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi 1 => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ value ] = reg;
        }
    }

    return result;
}

Any 
elaborationVisitor::visitArithmaticIf( titaniaParser::ArithmaticIfContext *ctx ) {

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ arithmatic if on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto test = static_cast< string >( visit( ctx->test ) );

    auto r_false = cb->valuesScopesLookup( "false" );
    auto labels = cb->makeLabel( { "consqExpr", "altrnExpr", "endAIf" } );
    auto consqPart = labels[ 0 ];
    auto altrnPart = labels[ 1 ];
    auto endAIf = labels[ 2 ];
    auto result = cb->getFreshRegister();

    auto cc = cb->getFreshCCRegister();

    cb->writeCodeBuffer( { "comp ", test, ", ",  r_false, " => ", cc } );
    cb->writeCodeBuffer( { "cbrneq ", cc, " -> @", consqPart, ", @", altrnPart } );

    cb->writeCodeBuffer( { consqPart, ":" } );

    auto consqReg = static_cast< string >( visit( ctx->consq ) );
    cb->writeCodeBuffer( { "i2i ", consqReg, " => ", result } );

    cb->writeCodeBuffer( { "jumpi @", endAIf } );

    cb->writeCodeBuffer( { altrnPart, ":" } );

    auto altrnReg = static_cast< string >( visit( ctx->altrn ) );
    cb->writeCodeBuffer( { "i2i ", altrnReg, " => ", result } );

    cb->writeCodeBuffer( { "jumpi @", endAIf } );

    cb->writeCodeBuffer( { endAIf, ":" } );

    return result;
}


Any
elaborationVisitor::visitFieldAccess(titaniaParser::FieldAccessContext *ctx ) {

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ---------------- Field access on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto oldAsAddress = asAddress;
    asAddress = true;
    auto base = static_cast< string >( visit( ctx->base ) );
    asAddress = oldAsAddress;

    auto symbols = symbolTables[ ctx ];
    auto record = symbols[ "recordField" ];
    auto fieldName = ctx->field->getText();
    Symbol field;

    for( auto f : record.fields ) {
        if( f.name == fieldName ) {
            field = f;
        }
    }

    if( arpOffset ) {
        cb->writeCodeBuffer( {  "# ................... looking up field ", fieldName, 
            " in record ", record.name, " with offset of ", to_str( field.fieldOffset ), 
            " and size ", to_str( field.sizeInBytes ) } );
    }

    auto offset = to_str( field.fieldOffset );
    string r_offset;
    if( memoizeExprs && cb->valuesScopesCount( offset ) > 0 ) {
        r_offset = cb->valuesScopesLookup( offset );
    }
    else {
        r_offset = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi ", offset, " => ", r_offset,
            "  # offset of field ", fieldName } );
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ offset ] = r_offset;
        }
    }

    auto fieldLocation = cb->getFreshRegister();
    cb->writeCodeBuffer( { "add ", base, ", ", r_offset, " => ", fieldLocation } );

    string result;

    if( asAddress ) {
        result = fieldLocation;
    }
    else {
        result = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi ", fieldLocation, " => ", result } );
    }


    return result;
}

Any
elaborationVisitor::visitArrayAccess( titaniaParser::ArrayAccessContext *ctx ) {
    if( lineNo ) {
        cb->writeCodeBuffer( { "# ---------------- Array access on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto oldAsAddress = asAddress;
    asAddress = true;
    auto base = static_cast< string >( visit( ctx->base ) );
    asAddress = oldAsAddress;

    auto index = static_cast< string >( visit( ctx->index ) );

    auto symbols = symbolTables[ ctx ];
    auto arrayBaseType = symbols[ "arrayBaseType" ];

    if( arpOffset ) {
        // base is a register that holds the offset in the ARP of the base of the array
        cb->writeCodeBuffer( { "# ---------------- base is ", base, " and index is ", index } );
    }

    // index is a register that holds an integer indicating the offset into the array to 
    // look.  the actual offset will be the size of the elements of the array times the index
    auto sizeOf = to_str( arrayBaseType.sizeInBytes );
    string r_sizeOf;
    if( memoizeExprs && cb->valuesScopesCount( sizeOf ) > 0 ) {
        r_sizeOf = cb->valuesScopesLookup( sizeOf );
    }
    else {
        r_sizeOf = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi ", sizeOf, " => ", r_sizeOf,
            "  # size of array element (", arrayBaseType.base, ")" } );
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ sizeOf ] = r_sizeOf;
        }
    }

    string r_offset;
    auto key = index + "mult " + r_sizeOf;
    if( memoizeExprs && cb->valuesScopesCount( key ) ) {
        r_offset = cb->valuesScopesLookup( key );
    }
    else {
        r_offset = cb->getFreshRegister();
        cb->writeCodeBuffer( { "mult ", index, ", ", r_sizeOf, " => ", r_offset,
            "  # offset into array of indexed element" } );
        if( memoizeExprs ) {
            cb->valuesScopes.back()[ key ] = r_offset;
        }
    }

    auto arrayLocation = cb->getFreshRegister();
    cb->writeCodeBuffer( { "add ", base, ", ", r_offset, " => ", arrayLocation } );

    string result;

    if( asAddress ) {
        result = arrayLocation;
    }
    else {
        result = cb->getFreshRegister();
        cb->writeCodeBuffer( { "loadi ", arrayLocation, " => ", result } );
    }

    return result;
}


// don't memoize statements
Any
elaborationVisitor::visitAssignment( titaniaParser::AssignmentContext* ctx ) {

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ assignment on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto rvalue = static_cast< string >( visit( ctx->rval ) );

    auto oldAsAddress = asAddress;
    asAddress = true;
    inAssignStmnt = true;
    auto lvalue = static_cast< string >( visit( ctx->lval ) );
    asAddress = oldAsAddress;
    inAssignStmnt = false;

    if( seeCurrentFnId ) {
        cb->writeCodeBuffer( { "i2i ", rvalue, " => ", fnReturnReg } );
        cb->writeCodeBuffer( { "jumpi @", currentFnExitLabel } );
        seeCurrentFnId = false;
    }
    else {
        cb->writeCodeBuffer( { "store ", rvalue, " => ", lvalue } );
    }

    return "0";

}

Any
elaborationVisitor::visitConstElem( titaniaParser::ConstElemContext* ctx ) {
    auto expr = static_cast< string >( visit( ctx->expression() ) );
    auto id = ctx->idDecl()->name->getText();

    string result;

    if( memoizeExprs && cb->valuesScopesCount( id ) > 0 ) {
        result = cb->valuesScopesLookup( id );
    }
    else {
        auto reg1 = cb->getFreshRegister();

        auto idSymbol = lookupId( id );
        cb->writeCodeBuffer( { "incrtos ", to_str( idSymbol.second.sizeInBytes ),
            "  # make space for ", id
        });

        cb->writeCodeBuffer( { "addi rarp, ", to_str( idSymbol.second.arpOffset), 
            " => ", reg1, "  # @", id, " = ", to_str( idSymbol.second.arpOffset)
        } );
        cb->writeCodeBuffer( { "store ", expr, " => ", reg1 } );

        if( memoizeExprs ) {
            cb->valuesScopes.back()[ "@" + id ] = reg1;
        }
    }

    return result;
}

Any
elaborationVisitor::visitVarElem( titaniaParser::VarElemContext* ctx ) {
    auto id = ctx->idDecl()->name->getText();

    string result;

    if( memoizeExprs && cb->valuesScopesCount( id ) > 0 ) {
        result = cb->valuesScopesLookup( id );
    }
    else {
        auto reg1 = cb->getFreshRegister();

        auto idSymbol = lookupId( id );
        cb->writeCodeBuffer( { "incrtos ", to_str( idSymbol.second.sizeInBytes ),
            "  # make space for ", id
        });

        if( ctx->expression() ) {
            auto expr = static_cast< string >( visit( ctx->expression() ) );
            
            cb->writeCodeBuffer( { "addi rarp, ", to_str( idSymbol.second.arpOffset ),
                 " => ", reg1, "  # @", id, " = ", to_str( idSymbol.second.arpOffset )
            } );
            cb->writeCodeBuffer( { "store ", expr, " => ", reg1 } );

            if( memoizeExprs ) {
                cb->valuesScopes.back()[ "@" + id ] = reg1;
            }
        }
    }

    return result;
}

// Since this is a statement rather than an expression, there is no register to return.
// Return the empty string.
Any
elaborationVisitor::visitIfThen( titaniaParser::IfThenContext *ctx ) {

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ if/then/else on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto test = static_cast< string >( visit( ctx->test ) );

    auto r_false = cb->valuesScopesLookup( "false" );
    auto labels = cb->makeLabel( { "thenBody", "elseBody", "endIf" } );
    auto thenPart = labels[ 0 ];
    auto elsePart = labels[ 1 ];
    auto endIf = labels[ 2 ];
    auto cc = cb->getFreshCCRegister();
    auto hasElseBody = ctx->elseBody != nullptr;


    cb->writeCodeBuffer( { "comp ", test, ", ",  r_false, " => ", cc } );
    cb->writeCodeBuffer( { "cbrneq ", cc, " -> @", thenPart, ", @", 
        ( hasElseBody ? elsePart : endIf ) } );

    cb->writeCodeBuffer( { thenPart, ":" } );

    unordered_map< string, string > valuesMap;
    cb->valuesScopes.push_back( valuesMap );

    visit( ctx->thenBody );

    cb->valuesScopes.pop_back();

    cb->writeCodeBuffer( { "jumpi @", endIf } );

    if( hasElseBody ) {
        cb->writeCodeBuffer( { elsePart, ":" } );

        unordered_map< string, string > valuesMap;
        cb->valuesScopes.push_back( valuesMap );

        visit( ctx->elseBody );

        cb->valuesScopes.pop_back();

        cb->writeCodeBuffer( { "jumpi @", endIf } );
    }

    cb->writeCodeBuffer( { endIf, ":" } );

    return "";
}

Any
elaborationVisitor::visitWhileDo( titaniaParser::WhileDoContext *ctx ) {

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ while/do on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    auto labels = cb->makeLabel( { "whileTest", "whileBody", "whileEnd" } );
    auto whileBody = labels[ 1 ];
    auto whileEnd = labels[ 2 ];
    auto r_false = cb->valuesScopesLookup( "false" );

    auto oldMemoize = memoizeExprs;
    memoizeExprs = false;

    auto test1 = static_cast< string >( visit( ctx->test ) );
    auto cc1 = cb->getFreshCCRegister();
    cb->writeCodeBuffer( { "comp ", test1, ", ",  r_false, " => ", cc1 } );
    cb->writeCodeBuffer( { "cbrneq ", cc1, " -> @", whileBody, ", @", whileEnd } ); 
    cb->writeCodeBuffer( { whileBody, ":" } );

    unordered_map< string, string > valuesMap;
    cb->valuesScopes.push_back( valuesMap );
    memoizeExprs = true;

    visit( ctx->whileBody );

    memoizeExprs = false;
    cb->valuesScopes.pop_back();

    auto test2 = static_cast< string >( visit( ctx->test ) );
    auto cc2 = cb->getFreshCCRegister();
    cb->writeCodeBuffer( { "comp ", test2, ", ",  r_false, " => ", cc2 } );
    cb->writeCodeBuffer( { "cbrneq ", cc2, " -> @", whileBody, ", @", whileEnd } ); 
    cb->writeCodeBuffer( { whileEnd, ":" } );

    memoizeExprs = oldMemoize;

    return "";
}

Any
elaborationVisitor::visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ctx ) {
    currentFnName = ctx->fnName->getText();

    CodeBuffer fnCodeBuffer{ currentFnName };
    cb = &fnCodeBuffer;

    if( lineNo ) {
        cb->writeCodeBuffer( { "# ................ function definition on line ", 
            to_str( ctx->getStart()->getLine() ) } );
    }

    fnReturnReg = cb->getFreshRegister();

    scopes.push_back( symbolTables[ ctx ] );

    // The stack will have the return address at the top, space for the return value,
    // followed by argument 1, argument 2, ... argument n

    cb->writeCodeBuffer( { currentFnName, ":" } );
    currentFnExitLabel = cb->makeLabel( currentFnName + "_exit" );

    // ------- prologue
    // set up space on the stack for locals
    auto localCnt = 0;
    if( ctx->constSection() ) {
        auto constSect = ctx->constSection();
        localCnt += constSect->constElem().size();
    }

    if( ctx->varSection() ) {
        auto varSect = ctx->varSection();
        localCnt += varSect->varElem().size();
    }

    if( ctx->constSection() ) {
        visit( ctx->constSection() );
    }

    if( ctx->varSection() ) {
        visit( ctx->varSection() );
    }

    // TODO: do something for the body here
    visit( ctx->body() );

    cb->writeCodeBuffer( { currentFnExitLabel + ":" } );

    // ------- epilogue
    // remove stack space for the locals
    for( auto i = 0; i < localCnt; i++ ) {
        cb->writeCodeBuffer( { "pop  # local" } );
    }

    // remove stack space for the parameters
    for( auto i = 0; i < ctx->idDecl().size(); i++ ) {
        cb->writeCodeBuffer( { "pop  # parameter" } );
    }

    // push the return value onto the stack, somehow
    cb->writeCodeBuffer( { "push ", fnReturnReg, "  #  push return value" } );

    cb->writeCodeBuffer( { "ret" } ); 

    scopes.pop_back();

    cb = &globalCodeBuffer;
    fnCodeBuffers.push_back( move( fnCodeBuffer ) );

    return "0";
}

// -------------------------------------------------------------------------------------

vector< CodeBuffer >&&
elaborationVisitor::getCodeBuffers() {
    return move( fnCodeBuffers );
}

ostream&
elaborationVisitor::dumpCodeBuffers( ostream &os ) {

    sort( fnCodeBuffers.begin(), fnCodeBuffers.end(), 
        []( auto a, auto b ){ return a.getName() < b.getName(); } );

    for( auto cb : fnCodeBuffers ) {
        cb.dumpCodeBuffer( os );
    }

    return os;
}


pair< bool, Symbol>
elaborationVisitor::lookupId( string id ) {
    pair< bool, Symbol > result;

    result.first = false;

    for( auto i = scopes.rbegin(); i != scopes.rend(); i++ ) {
        auto x = i->find( id );
        if( x != i->end() ) {
            result.first = true;
            result.second = x->second;
            break;
        }            
    }

    return result;
}

string
elaborationVisitor::to_str( size_t i ) {
    return to_string( i );
}

// --------------------------------------------------------------------------------------

int
main( int argc, char** argv ) {

    using namespace antlr4;

    for( auto i = 1; i < argc; i++ ) {

        string arg{ argv[  1 ] };

        string baseFileName{ argv[ i ] };
        cout << "typechecking file " << baseFileName << endl;

        vector< CodeBuffer > ir;

        // put these in a block to manage memory
        {
            ifstream file{ argv[ i ] };
            ANTLRInputStream input( file );
            titaniaLexer lexer( &input );
            CommonTokenStream tokens( &lexer );

            tokens.fill();

            titaniaParser parser( &tokens );
            tree::ParseTree* tree = parser.file();

            typeVisitor typecheck;
            typecheck.visit( tree );

            // typecheck.dumpSymbols( cout );

            if( typecheck.errorCount() == 0 ) {
                cout << "elaborating " << endl;
     
                elaborationVisitor irGen{ typecheck };
                irGen.visit( tree );
     
                ofstream outFile{ baseFileName + ".iloc"s };
                irGen.dumpCodeBuffers( outFile );
    
                ir = irGen.getCodeBuffers(); 
            }
        }

        IR opt{ move( ir ), argv[ 1 ] };

        opt.doLocalValueNumbering();

        auto irFileName{ baseFileName + ".lvn.iloc"s };
        ofstream outFile{ irFileName };
        opt.dumpCfgBasicBlocks( outFile );

        cout << endl;

    }

    return 0;
}