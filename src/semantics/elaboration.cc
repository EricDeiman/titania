
#include "elaboration.hh"
#include "type.hh"

#include <cassert>
#include <string>


elaborationVisitor::elaborationVisitor( typeVisitor &type ) {
    scopes = type.moveScopes();
    symbolTables = type.moveSymbolTables();

    theCodeBuffer = &globalCodeBuffer;
}

Any
elaborationVisitor::visitFile( titaniaParser::FileContext* ctx ) {

    scopes.push_back( symbolTables[ ctx ] );

    // set up the stack to look like a function call has been made even though there
    // isn't one

    theCodeBuffer->writeCodeBuffer( { "pushi 0  # phoney global access link" } );
    theCodeBuffer->writeCodeBuffer( { "pushi 0  # phoney global ARP" } );
    theCodeBuffer->writeCodeBuffer( { "pushi 0  # phoney global return address" } );
    theCodeBuffer->writeCodeBuffer( { "pushi 0  # phoney global return value" } );

    theCodeBuffer->writeCodeBuffer( { "i2i tos => rarp  # set up initial rarp" } );

    visitChildren( ctx );

    theCodeBuffer->writeCodeBuffer( { "hlt  # end the program" } );
    scopes.pop_back();

    return "";
}

Any
elaborationVisitor::visitIdentifier( titaniaParser::IdentifierContext *ctx ) {
    std::string result;
    std::string id{ ctx->ID()->getText() };

    std::string reg1;

    auto idSymbol = lookupId( id );
    theCodeBuffer->writeCodeBuffer( { "# . . . . . . . .  offset of ", id, " from ARP is ",
        std::to_string( idSymbol.second.arpOffset ), " and lexical level is ", 
        std::to_string( idSymbol.second.lexicalNest), " (current lexical level is ",
        std::to_string( scopes.size() ), ")"
    } );

    auto framesBack = scopes.size() - idSymbol.second.lexicalNest;

    if( theCodeBuffer->valuesScopesCount( "@" + id ) > 0 ) {
        reg1 = theCodeBuffer->valuesScopesLookup( "@" + id );
    }
    else {
        reg1 = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "addi rapr, ", std::to_string( idSymbol.second.arpOffset ), 
            " => ", reg1 } );
        theCodeBuffer->valuesScopes.back()[ "@"+ id ] = reg1;
    }

    if( asAddress ) {
        result = reg1;
    }
    else {
        std::string reg2 = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", reg1, " => ", reg2 } );
        result = reg2;
    }

    return result;
}

Any
elaborationVisitor::visitNumberLit( titaniaParser::NumberLitContext* ctx ) {
    std::string result;
    std::string lit{ ctx->NUMBER()->getText() };

    if( theCodeBuffer->valuesScopesCount( lit ) > 0 ) {
        result = theCodeBuffer->valuesScopesLookup( lit );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", lit, " => ", reg } );
        theCodeBuffer->valuesScopes.back()[ lit ] = reg;
        result = reg;
    }

    return result;
}

Any
elaborationVisitor::visitAddOp( titaniaParser::AddOpContext* ctx ) {
    auto left = static_cast< std::string >( visit( ctx->left ) );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    auto op = ctx->op->getText();
    std::string result, insr;

    switch( op[ 0 ] ) {
        case '+':
            insr = "add ";
            break;
        case '-':
            insr = "sub ";
            break;
    }

    auto key = left + insr + right;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        result = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitMultOp( titaniaParser::MultOpContext* ctx ) {
    auto left = static_cast< std::string >( visit( ctx->left ) );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    auto op = ctx->op->getText();
    std::string result, insr;

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
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        result = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
         theCodeBuffer->valuesScopes.back()[ key ] = reg;
     }
    }

    return result;
}

Any
elaborationVisitor::visitPrefixNegative( titaniaParser::PrefixNegativeContext* ctx ) {
    auto expr = static_cast< std::string >( visit( ctx->expression() ) );
    std::string result;

    auto key = "-1*" + expr;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        result = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "multi ", expr, " -1 => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            theCodeBuffer->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitFunctionCall( titaniaParser::FunctionCallContext* ctx ) {
    theCodeBuffer->writeCodeBuffer( { "# ................ function call on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto fnId = ctx->name->getText();

    auto fnSymbol = lookupId( fnId );
    assert( fnSymbol.first );
    auto fnBaseSymbol = lookupId( fnSymbol.second.base );

    std::string fnReg;

    if( memoizeExprs && theCodeBuffer->valuesScopesCount( "@" + fnId ) > 0 ) {
        fnReg = theCodeBuffer->valuesScopesLookup( "@" + fnId );
    }
    else {
        fnReg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi @", fnId, " => ", fnReg } );
        if( memoizeExprs ) { 
            theCodeBuffer->valuesScopes.back()[ "@" + fnId ] = fnReg;
        }
    }

    auto returnAddr = theCodeBuffer->makeLabel( fnId + "_return" );

    // ------- precall
    // registers saved
    // access link
    // caller's ARP
    // return address
    // return value
    // paramaters

    // no saved registers yet

    if( scopes.size() == 2 ) {
        theCodeBuffer->writeCodeBuffer ( { "push rarp  # global access link" } );
    }
    else {
        auto treg1 = theCodeBuffer->getFreshRegister();
        auto treg2 = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "subi rarp, 32 => ", treg1, " #  use my access link" } );
        theCodeBuffer->writeCodeBuffer( { "loadi ", treg1, " => ", treg2 } );
        theCodeBuffer->writeCodeBuffer( { "pushi ", treg2 } );
    }

    theCodeBuffer->writeCodeBuffer( { "push rarp" } );  // caller's rarp

    theCodeBuffer->writeCodeBuffer( { "pushi @", returnAddr } );  // return address

    theCodeBuffer->writeCodeBuffer( { "inctos ", std::to_string( fnBaseSymbol.second.sizeInBytes ), 
        "  # space for return value" } );  // return value

    theCodeBuffer->writeCodeBuffer( { "i2i tos => rarp  # set up callee arp" } );

    for( auto x = ctx->args.rbegin(); x != ctx->args.rend(); x++ ) {
        auto res = static_cast< std::string >( visit( *x ) );
        theCodeBuffer->writeCodeBuffer( { "push ", res } );
    }

    theCodeBuffer->writeCodeBuffer( { "call ", fnReg } );

    // -------- postreturn
    theCodeBuffer->writeCodeBuffer( { returnAddr, ":" } );

    // the callee will remove the parameters from the stack
    
    auto fnResultReg = theCodeBuffer->getFreshRegister();
    theCodeBuffer->writeCodeBuffer( { "pop ", fnResultReg, "  # return value" } );  // what to do with non-interger sized results?

    theCodeBuffer->writeCodeBuffer( { "pop", "  # return address" } );  // remove and ignore the return address

    theCodeBuffer->writeCodeBuffer( { "pop rarp" } );  // get our own ARP

    theCodeBuffer->writeCodeBuffer( { "pop", "  # access link" } );  // access link yet

    // no saved registers yet

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
    auto expr = static_cast< std::string >( visit( ctx->logicExpression() ) );

    std::string result;

    auto key = "not" + expr;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        result = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "not ", expr, " => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            theCodeBuffer->valuesScopes.back()[ key ] = reg;
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
    auto left = static_cast< std::string >( visit( ctx->left ) );
    
    auto r_false = theCodeBuffer->valuesScopesLookup( "false" );
    auto labels = theCodeBuffer->makeLabel( { "andEnd", "rhs", "" } );
    auto end = labels[ 0 ];
    auto rhs = labels[ 1 ];
    std::string result = theCodeBuffer->getFreshRegister();

    // if the result of the LHS is false, the entire expression is false
    auto cc = theCodeBuffer->getFreshCCRegister();
    theCodeBuffer->writeCodeBuffer( { "i2i ", left, " => ", result } );
    theCodeBuffer->writeCodeBuffer( { "comp ", r_false, ", ", left, " => ", cc } );
    theCodeBuffer->writeCodeBuffer( { "cbr_eq ", cc, " -> ", end, ", ", rhs } );

    // Otherwise, the entire expression is the result of the RHS
    theCodeBuffer->writeCodeBuffer( { rhs, ":" } );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    theCodeBuffer->writeCodeBuffer( { "i2i ", right, " => ", result } );

    theCodeBuffer->writeCodeBuffer( { end, ":" } );

    return result;
}

Any
elaborationVisitor::visitOrOp( titaniaParser::OrOpContext* ctx ) {
 
    auto left = static_cast< std::string >( visit( ctx->left ) );
 
    auto r_false = theCodeBuffer->valuesScopesLookup( "false" );
    auto labels = theCodeBuffer->makeLabel( { "orEnd", "rhs", "" } );
    auto l_end = labels[ 0 ];
    auto l_rhs = labels[ 1 ];
    std::string result = theCodeBuffer->getFreshRegister();

    // if the result of the LHS is true, the entire expression is true
    auto cc = theCodeBuffer->getFreshCCRegister();
    theCodeBuffer->writeCodeBuffer( { "i2i ", left, " => ", result } );
    theCodeBuffer->writeCodeBuffer( { "comp ", r_false, ", ", left, " => ", cc } );
    theCodeBuffer->writeCodeBuffer( { "cbr_neq ", cc, " -> ", l_end, ", ", l_rhs } );

    // Otherwise, the entire expression is the result of the RHS
    theCodeBuffer->writeCodeBuffer( { l_rhs, ":" } );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    theCodeBuffer->writeCodeBuffer( { "i2i ", right, " => ", result } );

    theCodeBuffer->writeCodeBuffer( { l_end, ":" } );

    return result;
}

Any
elaborationVisitor::visitCompOp( titaniaParser::CompOpContext* ctx ) {
    auto left = static_cast< std::string >( visit( ctx->left ) );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    auto op = ctx->op->getText();

    std::string insr;

    if( op == "<" ) {
        insr = "cmpLT";
    }
    else if( op == "<=" ) {
        insr = "cmpLE";
    }
    else if( op == "?=" ) {
        insr = "cmpEQ";
    }
    else if( op == "!=" ) {
        insr = "cmpNE";
    }
    else if( op == ">=" ) {
        insr = "cmpGE";
    }
    else {
        insr = "cmpGT";
    }

    std::string result;

    auto key = left + insr + right;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        result = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { insr, " ", left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitBoolLit( titaniaParser::BoolLitContext* ctx ) {
    auto value = ctx->getText();
    std::string result;

    if( memoizeExprs && theCodeBuffer->valuesScopesCount( value ) > 0 ) {
        result = theCodeBuffer->valuesScopesLookup( value );
    }
    else {
        auto reg = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadb ", value, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ value ] = reg;
        }
    }

    return result;
}

Any 
elaborationVisitor::visitArithmaticIf( titaniaParser::ArithmaticIfContext *ctx ) {

    theCodeBuffer->writeCodeBuffer( { "# ................ arithmatic if on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto test = static_cast< std::string >( visit( ctx->test ) );

    auto r_false = theCodeBuffer->valuesScopesLookup( "false" );
    auto labels = theCodeBuffer->makeLabel( { "consqExpr", "altrnExpr", "endAIf" } );
    auto consqPart = labels[ 0 ];
    auto altrnPart = labels[ 1 ];
    auto endAIf = labels[ 2 ];
    auto result = theCodeBuffer->getFreshRegister();

    auto cc = theCodeBuffer->getFreshCCRegister();

    theCodeBuffer->writeCodeBuffer( { "comp ", test, ", ",  r_false, " => ", cc } );
    theCodeBuffer->writeCodeBuffer( { "cbr_neq ", cc, " -> ", consqPart, ", ", altrnPart } );

    theCodeBuffer->writeCodeBuffer( { consqPart, ":" } );

    auto consqReg = static_cast< std::string >( visit( ctx->consq ) );
    theCodeBuffer->writeCodeBuffer( { "i2i ", consqReg, " => ", result } );

    theCodeBuffer->writeCodeBuffer( { "jumpI ", endAIf } );

    theCodeBuffer->writeCodeBuffer( { altrnPart, ":" } );

    auto altrnReg = static_cast< std::string >( visit( ctx->altrn ) );
    theCodeBuffer->writeCodeBuffer( { "i2i ", altrnReg, " => ", result } );

    theCodeBuffer->writeCodeBuffer( { "jumpI ", endAIf } );

    theCodeBuffer->writeCodeBuffer( { endAIf, ":" } );

    return result;
}


Any
elaborationVisitor::visitFieldAccess(titaniaParser::FieldAccessContext *ctx ) {
    theCodeBuffer->writeCodeBuffer( { "# ---------------- Field access on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto oldAsAddress = asAddress;
    asAddress = true;
    auto base = static_cast< std::string >( visit( ctx->base ) );
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

    theCodeBuffer->writeCodeBuffer( {  "# ................... looking up field ", fieldName, 
        " in record ", record.name, " with offset of ", std::to_string( field.fieldOffset ), 
        " and size ", std::to_string( field.sizeInBytes ) } );

    auto offset = std::to_string( field.fieldOffset );
    std::string r_offset;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( offset ) > 0 ) {
        r_offset = theCodeBuffer->valuesScopesLookup( offset );
    }
    else {
        r_offset = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", offset, " => ", r_offset,
            "  # offset of field ", fieldName } );
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ offset ] = r_offset;
        }
    }

    auto fieldLocation = theCodeBuffer->getFreshRegister();
    theCodeBuffer->writeCodeBuffer( { "add ", base, ", ", r_offset, " => ", fieldLocation } );

    std::string result;


    if( asAddress ) {
        result = fieldLocation;
    }
    else {
        result = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", fieldLocation, " => ", result } );
    }


    return result;
}

Any
elaborationVisitor::visitArrayAccess( titaniaParser::ArrayAccessContext *ctx ) {
    theCodeBuffer->writeCodeBuffer( { "# ---------------- Array access on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto oldAsAddress = asAddress;
    asAddress = true;
    auto base = static_cast< std::string >( visit( ctx->base ) );
    asAddress = oldAsAddress;

    auto index = static_cast< std::string >( visit( ctx->index ) );

    auto symbols = symbolTables[ ctx ];
    auto arrayBaseType = symbols[ "arrayBaseType" ];

    // base is a register that holds the offset in the ARP of the base of the array
    theCodeBuffer->writeCodeBuffer( { "# ---------------- base is ", base, " and index is ", index } );

    // index is a register that holds an integer indicating the offset into the array to 
    // look.  the actual offset will be the size of the elements of the array times the index
    auto sizeOf = std::to_string( arrayBaseType.sizeInBytes );
    std::string r_sizeOf;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( sizeOf ) > 0 ) {
        r_sizeOf = theCodeBuffer->valuesScopesLookup( sizeOf );
    }
    else {
        r_sizeOf = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", sizeOf, " => ", r_sizeOf,
            "  # size of array element (", arrayBaseType.base, ")" } );
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ sizeOf ] = r_sizeOf;
        }
    }

    std::string r_offset;
    auto key = index + "mult " + r_sizeOf;
    if( memoizeExprs && theCodeBuffer->valuesScopesCount( key ) ) {
        r_offset = theCodeBuffer->valuesScopesLookup( key );
    }
    else {
        r_offset = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "mult ", index, ", ", r_sizeOf, " => ", r_offset,
            "  # offset into array of indexed element" } );
        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ key ] = r_offset;
        }
    }

    auto arrayLocation = theCodeBuffer->getFreshRegister();
    theCodeBuffer->writeCodeBuffer( { "add ", base, ", ", r_offset, " => ", arrayLocation } );

    std::string result;

    if( asAddress ) {
        result = arrayLocation;
    }
    else {
        result = theCodeBuffer->getFreshRegister();
        theCodeBuffer->writeCodeBuffer( { "loadi ", arrayLocation, " => ", result } );
    }

    return result;
}


// don't memoize statements
Any
elaborationVisitor::visitAssignment( titaniaParser::AssignmentContext* ctx ) {

    theCodeBuffer->writeCodeBuffer( { "# ................ assignment on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto rvalue = static_cast< std::string >( visit( ctx->rval ) );

    auto oldAsAddress = asAddress;
    asAddress = true;
    auto lvalue = static_cast< std::string >( visit( ctx->lval ) );
    asAddress = oldAsAddress;

    theCodeBuffer->writeCodeBuffer( { "storei ", rvalue, " => ", lvalue } );

    return "0";

}

Any
elaborationVisitor::visitConstElem( titaniaParser::ConstElemContext* ctx ) {
    auto expr = static_cast< std::string >( visit( ctx->expression() ) );
    auto id = ctx->idDecl()->name->getText();

    std::string result;

    if( memoizeExprs && theCodeBuffer->valuesScopesCount( id ) > 0 ) {
        result = theCodeBuffer->valuesScopesLookup( id );
    }
    else {
        auto reg1 = theCodeBuffer->getFreshRegister();

        auto idSymbol = lookupId( id );
        theCodeBuffer->writeCodeBuffer( { "inctos ", std::to_string( idSymbol.second.sizeInBytes ),
            "  # make space for ", id
        });

        //theCodeBuffer->writeCodeBuffer( { "loadi @", id, " => ", reg1 } );
        theCodeBuffer->writeCodeBuffer( { "loadi ", std::to_string( idSymbol.second.arpOffset), 
            " => ", reg1, "  # @", id, " = ", std::to_string( idSymbol.second.arpOffset)
        } );
        theCodeBuffer->writeCodeBuffer( { "storeao ", expr, " => rarp, ", reg1 } );

        if( memoizeExprs ) {
            theCodeBuffer->valuesScopes.back()[ "@" + id ] = reg1;
        }
    }

    return result;
}

Any
elaborationVisitor::visitVarElem( titaniaParser::VarElemContext* ctx ) {
    auto id = ctx->idDecl()->name->getText();

    std::string result;

    if( memoizeExprs && theCodeBuffer->valuesScopesCount( id ) > 0 ) {
        result = theCodeBuffer->valuesScopesLookup( id );
    }
    else {
        auto reg1 = theCodeBuffer->getFreshRegister();

        auto idSymbol = lookupId( id );
        theCodeBuffer->writeCodeBuffer( { "inctos ", std::to_string( idSymbol.second.sizeInBytes ),
            "  # make space for ", id
        });

        if( ctx->expression() ) {
            auto expr = static_cast< std::string >( visit( ctx->expression() ) );
            
            //theCodeBuffer->writeCodeBuffer( { "loadi @", id, " => ", reg1 } );
            theCodeBuffer->writeCodeBuffer( { "loadi ", std::to_string( idSymbol.second.arpOffset ),
                 " => ", reg1, "  # @", id, " = ", std::to_string( idSymbol.second.arpOffset )
            } );
            theCodeBuffer->writeCodeBuffer( { "storeao ", expr, " => rarp, ", reg1 } );

            if( memoizeExprs ) {
                theCodeBuffer->valuesScopes.back()[ "@" + id ] = reg1;
            }
        }
    }

    return result;
}

// Since this is a statement rather than an expression, there is no register to return.
// Return the empty string.
Any
elaborationVisitor::visitIfThen( titaniaParser::IfThenContext *ctx ) {

    theCodeBuffer->writeCodeBuffer( { "# ................ if/then/else on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto test = static_cast< std::string >( visit( ctx->test ) );

    auto r_false = theCodeBuffer->valuesScopesLookup( "false" );
    auto labels = theCodeBuffer->makeLabel( { "thenBody", "elseBody", "endIf" } );
    auto thenPart = labels[ 0 ];
    auto elsePart = labels[ 1 ];
    auto endIf = labels[ 2 ];
    auto cc = theCodeBuffer->getFreshCCRegister();
    auto hasElseBody = ctx->elseBody != nullptr;


    theCodeBuffer->writeCodeBuffer( { "comp ", test, ", ",  r_false, " => ", cc } );
    theCodeBuffer->writeCodeBuffer( { "cbr_neq ", cc, " -> ", thenPart, ", ", 
        ( hasElseBody ? elsePart : endIf ) } );

    theCodeBuffer->writeCodeBuffer( { thenPart, ":" } );

    std::unordered_map< std::string, std::string > valuesMap;
    theCodeBuffer->valuesScopes.push_back( valuesMap );

    visit( ctx->thenBody );

    theCodeBuffer->valuesScopes.pop_back();

    theCodeBuffer->writeCodeBuffer( { "jumpI ", endIf } );

    if( hasElseBody ) {
        theCodeBuffer->writeCodeBuffer( { elsePart, ":" } );

        std::unordered_map< std::string, std::string > valuesMap;
        theCodeBuffer->valuesScopes.push_back( valuesMap );

        visit( ctx->elseBody );

        theCodeBuffer->valuesScopes.pop_back();

        theCodeBuffer->writeCodeBuffer( { "jumpI ", endIf } );
    }

    theCodeBuffer->writeCodeBuffer( { endIf, ":" } );

    return "";
}

Any
elaborationVisitor::visitWhileDo( titaniaParser::WhileDoContext *ctx ) {

    theCodeBuffer->writeCodeBuffer( { "# ................ while/do on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto labels = theCodeBuffer->makeLabel( { "whileTest", "whileBody", "whileEnd" } );
    auto whileBody = labels[ 1 ];
    auto whileEnd = labels[ 2 ];
    auto r_false = theCodeBuffer->valuesScopesLookup( "false" );

    auto oldMemoize = memoizeExprs;
    memoizeExprs = false;

    auto test1 = static_cast< std::string >( visit( ctx->test ) );
    auto cc1 = theCodeBuffer->getFreshCCRegister();
    theCodeBuffer->writeCodeBuffer( { "comp ", test1, ", ",  r_false, " => ", cc1 } );
    theCodeBuffer->writeCodeBuffer( { "cbr_neq ", cc1, " -> ", whileBody, ", ", whileEnd } ); 
    theCodeBuffer->writeCodeBuffer( { whileBody, ":" } );

    std::unordered_map< std::string, std::string > valuesMap;
    theCodeBuffer->valuesScopes.push_back( valuesMap );
    memoizeExprs = true;

    visit( ctx->whileBody );

    memoizeExprs = false;
    theCodeBuffer->valuesScopes.pop_back();

    auto test2 = static_cast< std::string >( visit( ctx->test ) );
    auto cc2 = theCodeBuffer->getFreshCCRegister();
    theCodeBuffer->writeCodeBuffer( { "comp ", test2, ", ",  r_false, " => ", cc2 } );
    theCodeBuffer->writeCodeBuffer( { "cbr_neq ", cc2, " -> ", whileBody, ", ", whileEnd } ); 
    theCodeBuffer->writeCodeBuffer( { whileEnd, ":" } );

    memoizeExprs = oldMemoize;

    return "";
}

Any
elaborationVisitor::visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ctx ) {
    theCodeBuffer->writeCodeBuffer( { "# ................ function definition on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    //saveGlobalState();
    CodeBuffer fnCodeBuffer;
    theCodeBuffer = &fnCodeBuffer;

    scopes.push_back( symbolTables[ ctx ] );

    // The stack will have the return address at the top, space for the return value,
    // followed by argument 1, argument 2, ... argument n

    theCodeBuffer->writeCodeBuffer( { ctx->fnName->getText(), ":" } );

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

    // ------- epilogue
    // remove stack space for the locals
    for( auto i = 0; i < localCnt; i++ ) {
        theCodeBuffer->writeCodeBuffer( { "pop  # local" } );
    }

    // remove stack space for the parameters
    for( auto i = 0; i < ctx->idDecl().size(); i++ ) {
        theCodeBuffer->writeCodeBuffer( { "pop  # parameter" } );
    }

    theCodeBuffer->writeCodeBuffer( { "ret" } ); 

    scopes.pop_back();

    theCodeBuffer = &globalCodeBuffer;
    fnCodeBuffers.push_back( std::move( fnCodeBuffer ) );

    return "0";
}

// -------------------------------------------------------------------------------------


std::ostream&
elaborationVisitor::dumpCodeBuffer( std::ostream &os ) {
    globalCodeBuffer.dumpCodeBuffer( os );
    
    os << std::endl << std::endl;

    for( auto cb : fnCodeBuffers ) {
        cb.dumpCodeBuffer( os );
    }

    return os;
}


std::pair< bool, Symbol>
elaborationVisitor::lookupId( std::string id ) {
    std::pair< bool, Symbol > result;

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

// --------------------------------------------------------------------------------------

int
main( int argc, char** argv ) {

    using namespace antlr4;

    typeVisitor typecheck;

    for( auto i = 1; i < argc; i++ ) {
        std::cout << "typechecking file " << argv[ i ] << std::endl;

        std::ifstream file{ argv[ i ] };
        ANTLRInputStream input( file );
        titaniaLexer lexer( &input );
        CommonTokenStream tokens( &lexer );

        tokens.fill();

        titaniaParser parser( &tokens );
        tree::ParseTree* tree = parser.file();

        typecheck.visit( tree );

        // typecheck.dumpSymbols( std::cout );

        if( typecheck.errorCount() == 0 ) {
            std::cout << "elaborating " << std::endl;
            elaborationVisitor irGen{ typecheck };
            irGen.visit( tree );
            std::cout << std::endl << std::endl;
            irGen.dumpCodeBuffer( std::cout );
            std::cout << std::endl;
        }
    }

    return 0;
}