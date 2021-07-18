
#include "type.hh"
#include <string>

// --------------------------------------------------------------------------------------

std::ostream&
Symbol::dump( std::ostream &os ) {

    if( constFlag ) {
        os << "constant ";
    }

    if( type == symbolType::function ) {
        os << "funciton ";
        if( fields.size() ) {
            os << "of ";
            for( auto x : fields ) {
                os << x.base << " ";
            }
        }
        os << "to ";
    }
    else if( type == symbolType::fnpointer || type == symbolType::forwardFn ) {
        os << "funciton ";
        if( fnPointerParameters.size() ) {
            os << "of ";
            for( auto x : fnPointerParameters ) {
                os << x << " ";
            }
        }
        os << "to ";
    }
    else if( type == symbolType::array ) {
        os << "array of ";
    }
    else if( type == symbolType::record ) {
        os << "record";
    }

    os << base;

    return os;
}

// --------------------------------------------------------------------------------------

typeVisitor::typeVisitor() {
    SymbolTable root;
    scopes.push_back( std::move( root ) );

    std::pair< std::string, Symbol > baseType;
    
    baseType.first = baseType.second.name = baseType.second.base = "integer";
    baseType.second.type = symbolType::builtin;
    baseType.second.sizeInBytes = 8;
    scopes.back().insert( std::move( baseType ) );

    baseType.first = baseType.second.name = baseType.second.base = "string";
    baseType.second.type = symbolType::builtin;
    baseType.second.sizeInBytes = 16;  // a 64-bit length and a 64-bit pointer
    scopes.back().insert( std::move( baseType ) );

    baseType.first = baseType.second.name = baseType.second.base = "boolean";
    baseType.second.type = symbolType::builtin;
    baseType.second.sizeInBytes = 1;
    scopes.back().insert( std::move( baseType ) );

    for( auto i : scopes.back() ) {
        if( i.second.type == symbolType::builtin ) {
            nonIdNames.push_back( i.second.name );
        }
    }

    symInteger = lookUp( "integer" ).second;
    symBoolean = lookUp( "boolean" ).second;
    symString = lookUp( "string" ).second;
}

// --------------------------------------------------------------------------------------

Any
typeVisitor::visitFile( titaniaParser::FileContext* ctx ) {
    SymbolTable fileLocal;

    std::string moduleName;

    for( auto i = 0; i < ctx->name.size() - 1; i++ ) {
        moduleName += ctx->name[ i ]->getText() + ".";
    }
    moduleName += ctx->name.back()->getText();

    scopes.push_back( std::move( fileLocal ) );

    visitChildren( ctx );

    moduleSymbolTable = std::move( scopes.back() );
    symbolTables[ ctx ] = moduleSymbolTable;
    scopes.pop_back();

    return 0;        
}

Any
typeVisitor::visitImport( titaniaParser::ImportContext* ctx ) {

    std::vector< std::string > uses;

    for( auto i : ctx->uses ) {
        uses.push_back( i->getText() );
    }

    auto moduleName = ctx->name->getText();
    moduleName += ".titania"s;

    std::ifstream moduleIfs{ moduleName };
    antlr4::ANTLRInputStream input( moduleIfs );
    titaniaLexer lexer( &input );
    antlr4::CommonTokenStream tokens( &lexer );
    tokens.fill();
    titaniaParser parser( &tokens );
    antlr4::tree::ParseTree* tree = parser.file();

    typeVisitor recursiveTypeCheck;
    recursiveTypeCheck.visit( tree );

    auto mSymTab = recursiveTypeCheck.moduleSymbolTable;

    for( auto u : uses ) {
        auto someU = mSymTab.find( u );
        if(  someU != mSymTab.end() ) {
            if( isType( someU->second ) || 
                    someU->second.type == symbolType::function || 
                    someU->second.type == symbolType::forwardFn ||
                    someU->second.constFlag  ) {
                scopes.back().insert( std::move( *someU ) );                
            }
            else {
                std::cerr << "symbol " << u << " is not a type, constant, or funciton" <<
                    " in module " << moduleName << "; cannot use it (on line " <<
                    sourceLine( ctx ) << ")" << std::endl;
                errors++;
            }
        }
        else {
            std::cerr << "unable to find symbol " << u << " to import from module " <<
                moduleName << " on line " << sourceLine( ctx ) << std::endl;
            errors++;
        }
    }

    return 0;
}

Any
typeVisitor::visitTypeRecord( titaniaParser::TypeRecordContext* ctx ) {

    std::pair< std::string, Symbol > recordType;
    recordType.second.type = symbolType::record;

    recordType.first = recordType.second.name = ctx->typeName->getText();

    if( validateId( recordType.first, 
            "cannot use symbol {name} to identify a record type", 
            sourceLine( ctx) ) ) {

        Symbol field;
        field.type = symbolType::variable;

        for(auto i : ctx->idDecl() ) {
            field.name = i->name->getText();

            if( validateId( field.name,
                     "cannot use symbol {name} to identify a field",
                     sourceLine( ctx ) ) ) {

                field.base = i->type->getText();

                if( validateType( field.base, 
                        "invalid type ({name}) for record field", 
                        sourceLine( ctx ) ) ) {

                    recordType.second.sizeInBytes += field.sizeInBytes;
                    recordType.second.fields.push_back( std::move( field ) );
                }
            }
        }

        scopes.back().insert( std::move( recordType ) );
    }
    return 0;
}

Any
typeVisitor::visitTypeAlias(titaniaParser::TypeAliasContext* ctx ) {

    std::pair< std::string, Symbol > aliasType;
    aliasType.second.type = symbolType::alias;

    aliasType.second.name = aliasType.first = ctx->typeName->getText();

    if( validateId( aliasType.first,
            "cannot use symbol {name} to identify an alias",
            sourceLine( ctx ) ) ) {

        aliasType.second.base = ctx->base->getText();

        if( validateType( aliasType.second.base,
                "invaid type ({name}) for alias",
                sourceLine( ctx ) ) ) {

            scopes.back().insert( std::move( aliasType ) );
        }
    }

    return 0;
}

Any
typeVisitor::visitTypePointer( titaniaParser::TypePointerContext* ctx ) {

    std::pair< std::string, Symbol > pointerType;
    pointerType.second.type = symbolType::datapointer;

    pointerType.second.name = pointerType.first = ctx->typeName->getText();

    if( validateId( pointerType.first,
            "cannot use symbol {name} to identify a pointer",
            sourceLine( ctx ) ) ) {

        pointerType.second.base = ctx->base->getText();

        if( validateType( pointerType.second.base,
                "invaid type ({name}) for pointer " + pointerType.first,
                sourceLine( ctx ) ) ) {

            scopes.back().insert( std::move( pointerType ) );
        }
    }

    return 0;
}

Any
typeVisitor::visitTypeFunctionPointer( titaniaParser::TypeFunctionPointerContext* ctx ) {

    std::pair< std::string, Symbol > fnPointerType;
    fnPointerType.second.type = symbolType::fnpointer;

    fnPointerType.second.name = fnPointerType.first = ctx->typeName->getText();

    if( validateId( fnPointerType.first,
            "cannot use symbol {name} to identify a function pointer", 
            sourceLine( ctx ) ) ) {

        fnPointerType.second.base = ctx->base->getText();

        if( validateType( fnPointerType.second.base,
                "invalid return type ({name}) for function pointer " + fnPointerType.first,
                sourceLine( ctx ) ) ) {

            for( auto x : ctx->params ) {

                if( validateType( x->getText(),
                        "invalid type ({name}) for function argument",
                        sourceLine( ctx ) ) ) {

                    fnPointerType.second.fnPointerParameters.push_back( x->getText() );
                }
            }

            scopes.back().insert( std::move( fnPointerType ) );
        }
    }

    return 0;
}

Any
typeVisitor::visitTypeArray( titaniaParser::TypeArrayContext* ctx ) {

    std::pair< std::string, Symbol > arrayType;
    arrayType.second.type = symbolType::array;

    arrayType.second.name = arrayType.first = ctx->typeName->getText();

    if( validateId( arrayType.first,
            "cannot use symbol {name} to identify array type",
            sourceLine( ctx ) ) ) {

        arrayType.second.base = ctx->base->getText();

        if( validateType( arrayType.second.base,
                "invalid type ({name}) for array " + arrayType.first,
                sourceLine( ctx ) ) ) {

            scopes.back().insert( std::move( arrayType ) );
        }
    }

    return 0;
}

Any
typeVisitor::visitForwardFunction( titaniaParser::ForwardFunctionContext* ctx ) {

    std::pair< std::string, Symbol > forwardFnType;
    forwardFnType.second.type = symbolType::forwardFn;

    forwardFnType.second.name = forwardFnType.first = ctx->fnName->getText();

    if( validateId( forwardFnType.first,
            "cannot use symbol {name} to identify forward function",
            sourceLine( ctx ) ) ) {

        forwardFnType.second.base = ctx->fnType->getText();

        if( validateType( forwardFnType.second.base,
                "invalid return type ({name}) for forward function " + forwardFnType.first,
                sourceLine( ctx ) ) ) {

            for( auto x : ctx->params ) {

                if( validateType( x->getText(),
                        "invalid type ({name}) for argument to forward function " + forwardFnType.first,
                        sourceLine( ctx ) ) ) {

                    forwardFnType.second.fnPointerParameters.push_back( x->getText() );
                }
            }

            scopes.back().insert( std::move( forwardFnType ) );
        }
    }

    return 0;
}

Any
typeVisitor::visitFunctionDefinition( titaniaParser::FunctionDefinitionContext* ctx ) {

    std::pair< std::string, Symbol > functionType;
    functionType.second.type = symbolType::function;

    functionType.second.name = functionType.first = ctx->fnName->getText();

    if( validateId( functionType.first,
            "cannot use symbol {name} to identify a function",
            sourceLine( ctx ) ) ) {

        functionType.second.base = ctx->fnType->getText();

        if( validateType( functionType.second.base,
                "invalid return type ({name}) for function " + functionType.first,
                sourceLine( ctx ) ) ) {

            Symbol field;
            field.type = symbolType::variable;

            for(auto i : ctx->idDecl() ) {
                field.name = i->name->getText();

                if( validateId( field.name,
                        "cannot use symbol {name} to identify paramater to function " + functionType.first,
                        sourceLine( ctx )) ) {

                    field.base = i->type->getText();
                    
                    if( validateType( field.base,
                            "invalid type ({name}) for argument " + field.name + " for function " + functionType.first,
                            sourceLine( ctx ) ) ) {

                        functionType.second.fields.push_back( std::move( field ) );
                    }
                }
            }

            scopes.back().insert( std::move( functionType ) );

            // Set up environment local to function

            SymbolTable functionLocal;

            std::pair< std::string, Symbol > param;
            for(auto i : ctx->idDecl() ) {
                param.first = param.second.name = i->name->getText();
                param.second.base = i->type->getText();
                param.second.type = symbolType::variable;

                functionLocal.insert( std::move( param ) );
            }

            scopes.push_back( std::move( functionLocal ) );
            definingFunctionNames.push( ctx->fnName->getText() );

            auto result = visitChildren( ctx );

            definingFunctionNames.pop();
            symbolTables[ ctx ] = scopes.back();
            scopes.pop_back();

            return result;            
        }
    }

    return 0;

}

Any
typeVisitor::visitConstElem( titaniaParser::ConstElemContext* ctx ) {

    std::pair< std::string, Symbol > constDecl;
    constDecl.second.type = symbolType::variable;

    constDecl.second.name = constDecl.first = ctx->idDecl()->name->getText();

    if( validateId( constDecl.first,
            "cannot use symbol {name} to identify constant",
            sourceLine( ctx ) ) ) {

        auto constTypeName = ctx->idDecl()->type->getText();

        constDecl.second.base = constTypeName;
        constDecl.second.constFlag = true;

        if( validateType( constDecl.second.base,
                "invalid type {name} for constant " + constDecl.second.name,
                sourceLine( ctx ) ) ) {

            scopes.back().insert( std::move( constDecl ) );
        }

        auto exprType = static_cast< Symbol >( visit( ctx->expression() ) );
        auto constType = lookUp( constTypeName );

        if( !sameType( constType.second, exprType ) ) {
            std::cerr << "attempt to assign a " << exprType.base << " to a constant " <<
                constType.second.base << " on line " << sourceLine( ctx ) <<
                std::endl;
            errors++;
        }
    }

    return 0;
}

Any
typeVisitor::visitVarElem( titaniaParser::VarElemContext* ctx ) {

    std::pair< std::string, Symbol > varDecl;
    varDecl.second.type = symbolType::variable;

    varDecl.second.name = varDecl.first = ctx->idDecl()->name->getText();

    if( validateId( varDecl.first,
            "cannot use symbol {name} to identify variable ",
            sourceLine( ctx ) ) ) {

        auto varTypeName = ctx->idDecl()->type->getText();
        varDecl.second.base = varTypeName;
        varDecl.second.constFlag = false;

        // validate type of constant
        if( validateType( varDecl.second.base,
                "invalid type ({name}) for variable " + varDecl.second.name,
                sourceLine( ctx ) ) ) {

            scopes.back().insert( std::move( varDecl ) );
        } 

        if( ctx->expression() ) {
            auto exprType = static_cast< Symbol >( visit( ctx->expression() ) );
            auto varType = lookUp( varTypeName );

            if( !sameType( exprType, varType.second ) ) {
                std::cerr << "attempt to assign a " << exprType.base << " to a " <<
                    varType.second.base << " on line " << sourceLine( ctx ) <<
                    std::endl;
            errors++;
            }
        }
    }

    return 0;
}

// --------------------------------------------------------------------------------------

Any
typeVisitor::visitAssignment( titaniaParser::AssignmentContext* ctx ) {
    auto lval = static_cast< Symbol >( visit( ctx->lval ) );
    auto rval = static_cast< Symbol >( visit( ctx->rval ) );

    if( !lval.constFlag ) {
        if( sameType( lval, rval ) ) {
            if( lval.type == symbolType::function && 
                    lval.name != definingFunctionNames.top() ) {
                std::cerr << "attempt to assign to function " << lval.name <<
                    " on line " << sourceLine( ctx ) << std::endl;
                errors++;
            }
            // allow the assignment
            return 0;
        }
        else {
            std::cerr << "attempt to assign a " << rval.base << " to a " << lval.base << 
                " on line " << sourceLine( ctx ) << std::endl;
            errors++;
        }
    }
    else {
        std::cerr << "attempt to assign to a constant " << lval.name << " on line " << 
            sourceLine( ctx ) << std::endl;
        errors++;
    }

    return 0;
}

Any
typeVisitor::visitWhileDo( titaniaParser::WhileDoContext* ctx ) {

    auto testType = static_cast< Symbol >( visit( ctx->test ) );

    if( !sameType( testType, symBoolean ) ) {
        std::cerr << "attempt to control do/while loop with something other than a " <<
            "boolean expression (" << testType.base << ") on line " << 
            sourceLine( ctx ) << std::endl;
        errors++;
    }

    visit( ctx->whileBody );

    return 0;
}

Any
typeVisitor::visitIfThen( titaniaParser::IfThenContext* ctx ) {

    auto testType = static_cast< Symbol >( visit( ctx->test ) );
    auto hasElseBody = ctx->elseBody != nullptr;

    if( !sameType( testType, symBoolean ) ) {
        std::cerr << "attempt to control if/then/else with something other than a " <<
            "boolean expression (" << testType.base << ") on line " << 
            sourceLine( ctx ) << std::endl;
        errors++;
    }

    visit( ctx->thenBody );

    if( hasElseBody ) {
        visit( ctx->elseBody );
}


    return 0;
}

// --------------------------------------------------------------------------------------

Any
typeVisitor::visitArithExp( titaniaParser::ArithExpContext* ctx ) {
    return visit( ctx->arithExpression() );
}

Any
typeVisitor::visitLogicExp( titaniaParser::LogicExpContext* ctx ) {
    return visit( ctx->logicExpression() );
}

Symbol
typeVisitor::lookUpField( Symbol type, std::string name, bool isConst ) {
    Symbol result;

    for( auto f : type.fields ) {
        if( f.name == name ) {
            auto symbol = lookUp( f.base ) ;
            if( symbol.first ) {
                result = symbol.second;
                result.constFlag = isConst;
                break;
            }
        }
    }

    return result;
}

Any
typeVisitor::visitFieldAccess( titaniaParser::FieldAccessContext* ctx ) {

    auto baseExpr = static_cast< Symbol >( visit( ctx->base ) );
    auto fieldName = ctx->field->getText();

    Symbol result;

    if( baseExpr.type == symbolType::variable ) {
        Symbol baseExprType = lookUp( baseExpr.base ).second;
        baseExprType.constFlag = baseExpr.constFlag;
        if( baseExprType.type == symbolType::record ) {
            result = lookUpField( baseExprType, fieldName, baseExprType.constFlag );
        }
        else {
            std::cerr << "attempt to look up a field on a non-record type (" <<
                baseExprType.name << ") on line " << sourceLine( ctx ) << 
                std::endl;
            errors++;
        }
    }
    else if( baseExpr.type == symbolType::record ) {
        result = lookUpField( baseExpr, fieldName, baseExpr.constFlag );
    }
    else {
        std::cerr << "attempt to look up a field on non-variable/non-record " << 
            baseExpr.name << " on line " << sourceLine( ctx ) << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitArrayAccess( titaniaParser::ArrayAccessContext* ctx ) {
    
    auto baseExpr = static_cast< Symbol >( visit( ctx->base ) );
    auto indexType = static_cast< Symbol >( visit( ctx->index ) );

    Symbol result;

    if( baseExpr.type == symbolType::variable ) {
        Symbol baseExprType = lookUp( baseExpr.base ).second;
        baseExprType.constFlag = baseExpr.constFlag;
        if( baseExprType.type == symbolType::array ) {
            if( indexType.base == "integer" ) {
                auto symbol = lookUp( baseExprType.base );
                if( symbol.first ) {
                    result = symbol.second;
                    result.constFlag = symbol.second.constFlag;

                    SymbolTable arraySymbols;
                    arraySymbols[ "arrayBaseType" ] = result;
                    symbolTables[ ctx ] = std::move( arraySymbols );
                }
            }
            else {
                std::cerr << "can only index into an array with an integer (not " << 
                    indexType.base << ") on line " << sourceLine( ctx ) << 
                    std::endl;
                errors++;
            }
        }
        else {
            std::cerr << "can only index into an array type (not " << 
                baseExprType.name << ") on line " << sourceLine( ctx ) <<
                std::endl;
            errors++;
        }
    }
    else {
        std::cerr << "attempt to index on non-variable " << baseExpr.name << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitArithmaticIf( titaniaParser::ArithmaticIfContext* ctx ) {
    
    auto testType = static_cast< Symbol >( visit( ctx->test ) );
    auto consqType = static_cast< Symbol >( visit( ctx->consq ) );
    auto altrnType = static_cast< Symbol >( visit( ctx->altrn ) );

    Symbol result;

    if( sameType( testType, symBoolean ) ) {
        if( sameType( consqType, altrnType ) ) {
            result = lookUp( consqType.base ).second;
        }
        else {
            std::cerr << "the consequence and alternative of an arithmatic if must" <<
                 " be of the same type (either " << consqType.base << " or " <<
                 altrnType.base << ") on line " << sourceLine( ctx ) <<
                 std::endl;
            errors++;
        }
    }
    else {
        std::cerr << "attempt to use an arithmatic if with a non-boolean type (" <<
            testType.name << ") on line " << sourceLine( ctx ) << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitStringLit( titaniaParser::StringLitContext* ctx ) {

     return symString;
}

Any
typeVisitor::visitDereferencePointer( titaniaParser::DereferencePointerContext* ctx ) {

    auto exprType = static_cast< Symbol >( visit( ctx->expression() ) );
    Symbol result;

    exprType = resolveType( exprType.base );

    if(  exprType.type == symbolType::datapointer || 
            exprType.type == symbolType::fnpointer ) {
        auto sym = lookUp( exprType.base );
        if( sym.first ) {
            result = sym.second;
        }
    }
    else {
        std::cerr << "attempt to dereference a non-pointer type (" << exprType.name <<
            ") at line " << sourceLine( ctx ) << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitTakeReference( titaniaParser::TakeReferenceContext* ctx ) {
    auto exprType = static_cast< Symbol >( visit( ctx->expression() ) );
    Symbol result;

    if( exprType.type == symbolType::variable ) {
        result.type = symbolType::datapointer;
        result.constFlag = exprType.constFlag;
        result.base = exprType.base;
        result.name = mkLabel( "ref" );
    }
    else if( exprType.type == symbolType::function ) {
        result.type = symbolType::fnpointer;
        for( auto i : exprType.fields ) {
            result.fnPointerParameters.push_back( i.base );
        }
        result.base = exprType.base;
        result.name = mkLabel( "fnref" );
    }

    return result;
}

Any
typeVisitor::visitGrouping( titaniaParser::GroupingContext* ctx ) {
    return visit( ctx->expression() );
}

Any
typeVisitor::visitPrefixNegative( titaniaParser::PrefixNegativeContext* ctx ) {

    auto exprType = static_cast< Symbol >( visit( ctx->expression() ) );
    Symbol result;

    if( exprType.type != symbolType::none && 
            exprType.type == symbolType::builtin &&
            exprType.base == "integer" ) {
        result = lookUp( exprType.base ).second;
    }

    return result;
}

Any
typeVisitor::visitFunctionCall( titaniaParser::FunctionCallContext* ctx ) {

    auto function = ctx->name->getText();
    auto functionType = lookUp( function );

    Symbol result;

    if( functionType.first ) {
        if( functionType.second.type == symbolType::function ) {
            if( ctx->args.size() == functionType.second.fields.size() ) {
                auto argTypes = getArgsTypes( ctx->args );
                if( compareArgParamTypes( argTypes, functionType.second.fields) ) {
                    auto sym = lookUp( functionType.second.base );
                    if( sym.first ) {
                        result = sym.second;                            
                    }
                }
                else {
                    std::cerr << "argument types don't match function parameter" <<
                        " types in call to " << function << " on line " << 
                        sourceLine( ctx ) << std::endl;
                    errors++;
                }
            }
            else {
                std::cerr << "attempt to call a function " << function << 
                    " with wrong number of arguments on line " << 
                    sourceLine( ctx ) << std::endl;
                errors++;
            }
        }
        else if( functionType.second.type == symbolType::forwardFn || 
                resolveType( functionType.second.base ).type == symbolType::fnpointer ) {
            if( ctx->args.size() == functionType.second.fnPointerParameters.size() ) {
                auto argTypes = getArgsTypes( ctx->args );
                std::vector< Symbol > paramTypes;
                for( auto i : functionType.second.fnPointerParameters ) {
                    paramTypes.push_back( lookUp( i ).second );
                }
                if( compareArgParamTypes( argTypes, paramTypes ) ) {
                    auto sym = lookUp( functionType.second.base );
                    if( sym.first ) {
                        result = sym.second;                            
                    }
                }
                else {
                    std::cerr << "argument types don't match function parameter" <<
                        " types in call to " << function << " on line " << 
                        sourceLine( ctx ) << std::endl;
                    errors++;
                }
            }
            else {
                auto fnType = functionType.second.type == symbolType::forwardFn ?
                                "forward function" : "function pointer";
                std::cerr << "attempt to call a " << fnType << " '" << function << 
                    "' with wrong number of arguments on line " << 
                    sourceLine( ctx ) << std::endl;
                errors++;
            }
        }
        else {
            std::cerr << "attempt to call a non-funtion symbol (" << function <<
                ") on line " << sourceLine( ctx ) << std::endl;
            errors++;
        }
    }
    else {
        std::cerr << "cannot find symbol named " << function << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitMultOp( titaniaParser::MultOpContext* ctx ) {
    auto leftType = static_cast< Symbol >( visit( ctx->left ) );
    auto rightType = static_cast< Symbol >( visit( ctx->right ) );

    Symbol result;

    if( sameType( leftType, rightType ) && sameType( leftType, symInteger ) ) {
        result = lookUp( leftType.base ).second;
    }
    else {
        std::cerr << "attempt to use a multiplicative operator on non-integer type " <<
            " on line " << sourceLine( ctx ) << " : " << std::endl;
        std::cerr << "\t" << leftType.base << " " << ctx->op->getText() << " " <<
            rightType.base << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitAddOp( titaniaParser::AddOpContext* ctx ) {
    auto leftType = static_cast< Symbol >( visit( ctx->left ) );
    auto rightType = static_cast< Symbol >( visit( ctx->right ) );

    Symbol result;

    if( sameType( leftType, rightType ) && sameType( leftType, symInteger ) ) {
        result = lookUp( leftType.base ).second;
    }
    else {
        std::cerr << "attempt to use an addative operator on non-integer type " <<
            " on line " << sourceLine( ctx ) << " : " << std::endl;
        std::cerr << "\t" << leftType.base << " " << ctx->op->getText() << " " <<
            rightType.base << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitNumberLit( titaniaParser::NumberLitContext* ctx ) {
    return symInteger;
}

Any
typeVisitor::visitIdentifier( titaniaParser::IdentifierContext* ctx ) {
    auto res = lookUp( ctx->ID()->getText() );
    Symbol result;

    if( res.first ) {
        result = res.second;
    }
    else {
        std::cerr << "unable to find identifier " << ctx->ID()->getText() <<
            " on line " << sourceLine( ctx ) << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitRecusive( titaniaParser::RecusiveContext* ctx ) {
    return visit( ctx->arithExpression() );
}

Any
typeVisitor::visitNotOp( titaniaParser::NotOpContext* ctx ) {
    auto res = static_cast< Symbol >( visit( ctx->logicExpression() ) );
    Symbol result;

    if( sameType( res, symBoolean ) ) {
        result = lookUp( res.base ).second;
    }
    else {
        std::cerr << "attempt to use a non-boolean type (" << res.base << 
            ") in a not expression on line " << sourceLine( ctx ) << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitAndOp( titaniaParser::AndOpContext* ctx ) {
    auto leftType = static_cast< Symbol >( visit( ctx->left ) );
    auto rightType = static_cast< Symbol >( visit( ctx->right ) );

    Symbol result;

    if( sameType( leftType, rightType ) && sameType( leftType, symBoolean ) ) {
        result = lookUp( leftType.base ).second; 
    }
    else {
        std::cerr << "attempt to use a non-boolean type in an and op on line " <<
            sourceLine( ctx ) << std::endl;
        std::cerr << "\t" << leftType.base << " and " << rightType.base << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitOrOp( titaniaParser::OrOpContext* ctx ) {
    auto leftType = static_cast< Symbol >( visit( ctx->left ) );
    auto rightType = static_cast< Symbol >( visit( ctx->right ) );

    Symbol result;

    if( sameType( leftType, rightType ) && sameType( leftType, symBoolean ) ) {
        result = lookUp( leftType.base ).second;
    }
    else {
        std::cerr << "attempt to use a non-boolean type in an or op on line " <<
            sourceLine( ctx ) << std::endl;
        std::cerr << "\t" << leftType.base << " or " << rightType.base << std::endl;
        errors++;
    }

    return result;
}

Any
typeVisitor::visitCompOp( titaniaParser::CompOpContext* ctx ) {

    auto leftType = static_cast< Symbol >( visit( ctx->left ) );
    auto rightType = static_cast< Symbol >( visit( ctx->right ) );

    Symbol result;

    if( sameType( leftType, rightType ) && sameType( leftType, symInteger ) ) {
        result = symBoolean;
    }
    else {
        std::cerr << "attempt to use a non-integer type in an comparison on line " <<
            sourceLine( ctx ) << std::endl;
        std::cerr << "\t" << leftType.base << " " << ctx->op->getText() << " " << 
            rightType.base << std::endl;
        errors++;
    }

    return result;        
}

Any
typeVisitor::visitBoolLit( titaniaParser::BoolLitContext* ctx ) {
    return symBoolean;
}

// --------------------------------------------------------------------------------------

std::ostream &
typeVisitor::dumpSymbols( std::ostream &os ) {
    for( auto i : symbolTables ) {
        os << "table contains:" << std::endl;
        for( auto j : i.second ) {
            os << "\t" << j.first << " of type ";
            j.second.dump( os ) << std::endl;
        }
    }

    return os;
}

std::vector<SymbolTable> &&
typeVisitor::moveScopes() {
    return std::move( scopes );
}

std::unordered_map< antlr4::ParserRuleContext*, SymbolTable > &&
typeVisitor::moveSymbolTables() {
    return std::move( symbolTables );
}

// --------------------------------------------------------------------------------------

size_t
typeVisitor::sourceLine( antlr4::ParserRuleContext * ctx ) {
    return ctx->getStart()->getLine();
}

bool
typeVisitor::validId( std::string name ) {
    auto result = find( nonIdNames.begin(), nonIdNames.end(), name );
    return result == nonIdNames.end();
}

bool
typeVisitor::validateId( std::string name, std::string message, size_t line ) {
    auto result = validId( name );

    if( !result ) {
        auto target = message.find( "{name}"s );
        if( target != std::string::npos ) {
            message.replace( target, 6, name );
        }
        std::cerr << message << " on line " << line << std::endl;
    }

    return result;
}

bool
typeVisitor::sameType( Symbol t1, Symbol t2 ) {
    if( !isType( t1 ) ) {
        t1 = lookUp( t1.base ).second;
    }

    if( !isType( t2 ) ) {
        t2 = lookUp( t2.base ).second;
    }

    while( t1.type == symbolType::alias ) {
        t1 = lookUp( t1.base ).second;
    }

    while( t2.type == symbolType::alias ) {
        t2 = lookUp( t2.base ).second;
    }

    return t1.base == t2.base && t1.type != symbolType::none && 
        t2.type != symbolType::none;        
}

bool
typeVisitor::compareArgParamTypes( std::vector< Symbol > args, argsTypes params ) {
    if( args.size() == params.size() ) {
        for( auto x = 0; x < args.size(); x++ ) {
            if( !sameType( args[ x ], lookUp( params[ x ].second ).second ) ) {
                return false;
            }
        }
    }
    else {
        return false;
    }

    return true;
}

bool
typeVisitor::compareArgParamTypes( std::vector< Symbol > args, std::vector< Symbol > params ) {
    if( args.size() == params.size() ) {
        for( auto x = 0; x < args.size(); x++ ) {
            if( !sameType( args[ x ], params[ x ] ) ) {
                return false;
            }
        }
    }
    else {
        return false;
    }

    return true;
}

std::vector< Symbol >
typeVisitor::getArgsTypes( std::vector< titaniaParser::ExpressionContext* > args ) {
    std::vector< Symbol > argTypes;

    for( auto i : args ) {
        auto exp = static_cast< Symbol >( visit( i ) );
        argTypes.push_back( exp );
    }

    return argTypes;
}

Symbol
typeVisitor::resolveType( std::string name ) {
    
    std::pair< bool, Symbol > res;
    do {
        res = lookUp( name );
        if( res.first && res.second.type == symbolType::alias ) {
            name = res.second.base;
        }
    } 
    while( res.first && res.second.type == symbolType::alias );

    return res.second;
}

bool
typeVisitor::isType( Symbol symbol ) {
        switch( symbol.type ) {
            case symbolType::record:
            case symbolType::alias:
            case symbolType::datapointer:
            case symbolType::fnpointer:
            case symbolType::array:
            case symbolType::builtin:
                return true;
                break;

            default:
                return false;
        }        
}

bool
typeVisitor::validType( std::string name ) {

    auto r = lookUp( name );
    if( r.first ) {
        return isType( r.second );
    }

    return false;
}

bool
typeVisitor::validateType( std::string name, std::string message, size_t line ) {
    auto result = validType( name );

    if( !result ) {
        auto target = message.find( "{name}"s );
        if( target != std::string::npos ) {
            message.replace( target, 6, name );
        }
        std::cerr << message << " on line " << line << std::endl;
    }

    return result;
}

std::pair<bool, Symbol>
typeVisitor::lookUp( std::string id ) {
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

std::string
typeVisitor::mkLabel( std::string prefix ) {
    return prefix + std::to_string( nameSuffix++ );
}

