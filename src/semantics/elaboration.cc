
#include "elaboration.hh"
#include "type.hh"
#include <string>

elaborationVisitor::elaborationVisitor( typeVisitor &type ) {
    scopes = type.moveScopes();
    symbolTables = type.moveSymbolTables();

    // since zero is false, it will be handy to have in a register
    auto reg = getFreshRegister();
    codeBuffer.push_back( "loadi 0 => " + reg );

    valuesScopes.push_back( valuesMap );

    valuesScopes.back()[ "0" ] = reg;
    valuesScopes.back()[ "false" ] = reg;

}

Any
elaborationVisitor::visitIdentifier( titaniaParser::IdentifierContext *ctx ) {
    std::string result;
    std::string id{ ctx->ID()->getText() };

    std::string reg1;

    if( valuesScopesCount( "@" + id ) > 0 ) {
        reg1 = valuesScopesLookup( "@" + id );
    }
    else {
        reg1 = getFreshRegister();
        writeCodeBuffer( { "loadi @", id, " => ", reg1 } );
        valuesScopes.back()[ "@"+ id ] = reg1;
    }

    if( asLvalue ) {
        result = reg1;
    }
    else {
        std::string reg2 = getFreshRegister();
        writeCodeBuffer( { "loadao rarp, ", reg1, " => ", reg2 } );
        result = reg2;
    }

    return result;
}

Any
elaborationVisitor::visitNumberLit( titaniaParser::NumberLitContext* ctx ) {
    std::string result;
    std::string lit{ ctx->NUMBER()->getText() };

    if( valuesScopesCount( lit ) > 0 ) {
        result = valuesScopesLookup( lit );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { "loadi ", lit, " => ", reg } );
        valuesScopes.back()[ lit ] = reg;
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
    if( memoizeExprs && valuesScopesCount( key ) ) {
        result = valuesScopesLookup( key );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            valuesScopes.back()[ key ] = reg;
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
    if( memoizeExprs && valuesScopesCount( key ) ) {
        result = valuesScopesLookup( key );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { insr, left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
         valuesScopes.back()[ key ] = reg;
     }
    }

    return result;
}

Any
elaborationVisitor::visitPrefixNegative( titaniaParser::PrefixNegativeContext* ctx ) {
    auto expr = static_cast< std::string >( visit( ctx->expression() ) );
    std::string result;

    auto key = "-1*" + expr;
    if( memoizeExprs && valuesScopesCount( key ) ) {
        result = valuesScopesLookup( key );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { "multi ", expr, " -1 => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitFunctionCall( titaniaParser::FunctionCallContext* ctx ) {
    auto fnId = ctx->name->getText();

    std::string fnReg;

    if( memoizeExprs && valuesScopesCount( "@" + fnId ) > 0 ) {
        fnReg = valuesScopesLookup( "@" + fnId );
    }
    else {
        fnReg = getFreshRegister();
        writeCodeBuffer( { "loadi @", fnId, " => ", fnReg } );
        if( memoizeExprs ) { 
            valuesScopes.back()[ "@" + fnId ] = fnReg;
        }
    }

    for( auto x = ctx->args.rbegin(); x != ctx->args.rend(); x++ ) {
        auto res = static_cast< std::string >( visit( *x ) );
        codeBuffer.push_back( "push " + res );
    }

    codeBuffer.push_back( "call " + fnReg );
    auto fnResultReg = getFreshRegister();

    if( memoizeExprs ) {
        codeBuffer.push_back( "pop " + fnResultReg );
    }

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
    if( memoizeExprs && valuesScopesCount( key ) ) {
        result = valuesScopesLookup( key );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { "not ", expr, " => ", reg } );
        result = reg;
        if( memoizeExprs ) { 
            valuesScopes.back()[ key ] = reg;
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
    
    auto r_false = valuesScopesLookup( "false" );
    auto labels = makeLabel( { "andEnd", "rhs", "" } );
    auto end = labels[ 0 ];
    auto rhs = labels[ 1 ];
    std::string result = getFreshRegister();

    // if the result of the LHS is false, the entire expression is false
    auto cc = getFreshCCRegister();
    writeCodeBuffer( { "i2i ", left, " => ", result } );
    writeCodeBuffer( { "comp ", r_false, ", ", left, " => ", cc } );
    writeCodeBuffer( { "cbr_eq ", cc, " -> ", end, ", ", rhs } );

    // Otherwise, the entire expression is the result of the RHS
    writeCodeBuffer( { rhs, ":" } );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    writeCodeBuffer( { "i2i ", right, " => ", result } );

    codeBuffer.push_back( end + ":" );

    return result;
}

Any
elaborationVisitor::visitOrOp( titaniaParser::OrOpContext* ctx ) {
 
    auto left = static_cast< std::string >( visit( ctx->left ) );
 
    auto r_false = valuesScopesLookup( "false" );
    auto labels = makeLabel( { "orEnd", "rhs", "" } );
    auto l_end = labels[ 0 ];
    auto l_rhs = labels[ 1 ];
    std::string result = getFreshRegister();

    // if the result of the LHS is true, the entire expression is true
    auto cc = getFreshCCRegister();
    writeCodeBuffer( { "i2i ", left, " => ", result } );
    writeCodeBuffer( { "comp ", r_false, ", ", left, " => ", cc } );
    writeCodeBuffer( { "cbr_neq ", cc, " -> ", l_end, ", ", l_rhs } );

    // Otherwise, the entire expression is the result of the RHS
    codeBuffer.push_back( l_rhs + ":" );
    auto right = static_cast< std::string >( visit( ctx->right ) );
    writeCodeBuffer( { "i2i ", right, " => ", result } );

    codeBuffer.push_back( l_end + ":" );

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
    if( memoizeExprs && valuesScopesCount( key ) ) {
        result = valuesScopesLookup( key );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { insr, " ", left, ", ", right, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            valuesScopes.back()[ key ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitBoolLit( titaniaParser::BoolLitContext* ctx ) {
    auto value = ctx->getText();
    std::string result;

    if( memoizeExprs && valuesScopesCount( value ) > 0 ) {
        result = valuesScopesLookup( value );
    }
    else {
        auto reg = getFreshRegister();
        writeCodeBuffer( { "loadb ", value, " => ", reg } );
        result = reg;
        if( memoizeExprs ) {
            valuesScopes.back()[ value ] = reg;
        }
    }

    return result;
}

Any
elaborationVisitor::visitArrayAccess( titaniaParser::ArrayAccessContext *ctx ) {

    auto base = static_cast< std::string >( visit( ctx->base ) );
    auto index = static_cast< std::string >( visit( ctx->index ) );
    auto result = getFreshRegister();
    auto symbolTable = symbolTables[ ctx ];

    // base is a register that holds the offset in the ARP of the base of the array
    writeCodeBuffer( { "# ---------------- base is ", base, " and index is ", index } );

    // index is a register that holds an integer indicating the offse into the array to 
    // look

    // the actual offset will be the size of the elements of the array times the index

    return result;
}


// don't memoize statements
Any
elaborationVisitor::visitAssignment( titaniaParser::AssignmentContext* ctx ) {

    writeCodeBuffer( { "# ................ assignment on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto rvalue = static_cast< std::string >( visit( ctx->rval ) );

    asLvalue = true;
    auto lvalue = static_cast< std::string >( visit( ctx->lval ) );
    asLvalue = false;

    writeCodeBuffer( { "storeao ", rvalue, " => rarp, ", lvalue } );

    dumpCodeBuffer( "assignment", ctx );

    return "0";

}

Any
elaborationVisitor::visitConstElem( titaniaParser::ConstElemContext* ctx ) {
    auto expr = static_cast< std::string >( visit( ctx->expression() ) );
    auto id = ctx->idDecl()->name->getText();

    std::string result;

    if( memoizeExprs && valuesScopesCount( id ) > 0 ) {
        result = valuesScopesLookup( id );
    }
    else {
        auto reg1 = getFreshRegister();
        writeCodeBuffer( { "loadi @", id, " => ", reg1 } );

        writeCodeBuffer( { "storeao ", expr, " => rarp, ", reg1 } );

        if( memoizeExprs ) {
            valuesScopes.back()[ "@" + id ] = reg1;
        }
    }

    return result;
}

// Since this is a statement rather than an expression, there is no register to return.
// Return the empty string.
Any
elaborationVisitor::visitIfThen( titaniaParser::IfThenContext *ctx ) {

    writeCodeBuffer( { "# ................ if/then/else on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto test = static_cast< std::string >( visit( ctx->test ) );

    auto r_false = valuesScopesLookup( "false" );
    auto labels = makeLabel( { "thenBody", "elseBody", "endIf" } );
    auto thenPart = labels[ 0 ];
    auto elsePart = labels[ 1 ];
    auto endIf = labels[ 2 ];
    auto cc = getFreshCCRegister();
    auto hasElseBody = ctx->elseBody != nullptr;


    writeCodeBuffer( { "comp ", test, ", ",  r_false, " => ", cc } );
    writeCodeBuffer( { "cbr_neq ", cc, " -> ", thenPart, ", ", 
        ( hasElseBody ? elsePart : endIf ) } );

    writeCodeBuffer( { thenPart, ":" } );

    scopes.push_back( symbolTables[ ctx ] );
    std::unordered_map< std::string, std::string > valuesMap;
    valuesScopes.push_back( valuesMap );

    visit( ctx->thenBody );

    valuesScopes.pop_back();
    scopes.pop_back();

    writeCodeBuffer( { "jumpI ", endIf } );

    if( hasElseBody ) {
        writeCodeBuffer( { elsePart, ":" } );

        scopes.push_back( symbolTables[ ctx + 1 ] );
        std::unordered_map< std::string, std::string > valuesMap;
        valuesScopes.push_back( valuesMap );

        visit( ctx->elseBody );

        valuesScopes.pop_back();
        scopes.pop_back();

        writeCodeBuffer( { "jumpI ", endIf } );
    }

    codeBuffer.push_back( endIf + ":" );

    dumpCodeBuffer( "if statement", ctx );

    return "";
}

Any
elaborationVisitor::visitWhileDo( titaniaParser::WhileDoContext *ctx ) {

    writeCodeBuffer( { "# ................ while/do on line ", 
        std::to_string( ctx->getStart()->getLine() ) } );

    auto labels = makeLabel( { "whileTest", "whileBody", "whileEnd" } );
    auto whileBody = labels[ 1 ];
    auto whileEnd = labels[ 2 ];
    auto r_false = valuesScopesLookup( "false" );

    auto oldMemoize = memoizeExprs;
    memoizeExprs = false;

    auto test1 = static_cast< std::string >( visit( ctx->test ) );
    auto cc1 = getFreshCCRegister();
    writeCodeBuffer( { "comp ", test1, ", ",  r_false, " => ", cc1 } );
    writeCodeBuffer( { "cbr_neq ", cc1, " -> ", whileBody, ", ", whileEnd } ); 
    writeCodeBuffer( { whileBody, ":" } );

    scopes.push_back( symbolTables[ ctx ] );
    std::unordered_map< std::string, std::string > valuesMap;
    valuesScopes.push_back( valuesMap );
    memoizeExprs = true;

    visit( ctx->whileBody );

    memoizeExprs = false;
    valuesScopes.pop_back();
    scopes.pop_back();

    auto test2 = static_cast< std::string >( visit( ctx->test ) );
    auto cc2 = getFreshCCRegister();
    writeCodeBuffer( { "comp ", test2, ", ",  r_false, " => ", cc2 } );
    writeCodeBuffer( { "cbr_neq ", cc2, " -> ", whileBody, ", ", whileEnd } ); 
    writeCodeBuffer( { whileEnd, ":" } );

    memoizeExprs = oldMemoize;

    dumpCodeBuffer( "while statement", ctx );

    return "";
}

Any
elaborationVisitor::visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ctx ) {
    // The stack will have the return address at the top, followed by argument 1, 
    // argument 2, ... argument n

    return "0";
}


std::string
elaborationVisitor::getFreshRegister() {
    return "r" + std::to_string( registerNum++ );
}

std::string
elaborationVisitor::getFreshCCRegister() {
    return "cc" + std::to_string( ccNum++ );
}

std::string
elaborationVisitor::makeLabel( std::string base ) {
    return base + std::to_string( labelSuffix++ );
}

std::vector< std::string >
elaborationVisitor::makeLabel( std::vector< std::string > prefixes ) {
    std::vector< std::string >labels;

    for( auto p : prefixes ) {
        labels.push_back( p + std::to_string( labelSuffix ) );
    }

    labelSuffix++;

    return std::move( labels );
}

void
elaborationVisitor::writeCodeBuffer( std::vector< std::string > data ) {

    std::string buffer;
    for( auto s : data ) {
        buffer += s;
    }

    codeBuffer.push_back( buffer );
}

void
elaborationVisitor::dumpCodeBuffer( std::string description, antlr4::ParserRuleContext *ctx ) {
    std::cout << "debug: at " << description << " on line " << ctx->getStart()->getLine() << 
        std::endl;
    std::cout << "ir code so far: " << std::endl;

    for( auto s : codeBuffer ) {
        std::cout << "\t" << s << std::endl;
    }

    std::cout << std::endl << std::endl;

}

size_t
elaborationVisitor::valuesScopesCount( std::string id ) {

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            return m.count( id );
        }
    }

    return 0;
}

std::string
elaborationVisitor::valuesScopesLookup( std::string id ) {

    std::string tmp;

    for( auto m : valuesScopes ) {
        if( m.count( id ) > 0 ) {
            tmp = m[ id ];
        }
    }

    return tmp;
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
        }
    }

    return 0;
}