
// Generated from titania.g4 by ANTLR 4.7.1

#pragma once


#include "antlr4-runtime.h"
#include "titaniaParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by titaniaParser.
 */
class  titaniaVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by titaniaParser.
   */
    virtual antlrcpp::Any visitFile(titaniaParser::FileContext *context) = 0;

    virtual antlrcpp::Any visitFromSection(titaniaParser::FromSectionContext *context) = 0;

    virtual antlrcpp::Any visitInstantiate(titaniaParser::InstantiateContext *context) = 0;

    virtual antlrcpp::Any visitImport(titaniaParser::ImportContext *context) = 0;

    virtual antlrcpp::Any visitTypeSection(titaniaParser::TypeSectionContext *context) = 0;

    virtual antlrcpp::Any visitTypeRecord(titaniaParser::TypeRecordContext *context) = 0;

    virtual antlrcpp::Any visitTypeAlias(titaniaParser::TypeAliasContext *context) = 0;

    virtual antlrcpp::Any visitTypePointer(titaniaParser::TypePointerContext *context) = 0;

    virtual antlrcpp::Any visitTypeFunctionPointer(titaniaParser::TypeFunctionPointerContext *context) = 0;

    virtual antlrcpp::Any visitTypeArray(titaniaParser::TypeArrayContext *context) = 0;

    virtual antlrcpp::Any visitIdDecl(titaniaParser::IdDeclContext *context) = 0;

    virtual antlrcpp::Any visitConstSection(titaniaParser::ConstSectionContext *context) = 0;

    virtual antlrcpp::Any visitConstElem(titaniaParser::ConstElemContext *context) = 0;

    virtual antlrcpp::Any visitFunctionSection(titaniaParser::FunctionSectionContext *context) = 0;

    virtual antlrcpp::Any visitForwardFunction(titaniaParser::ForwardFunctionContext *context) = 0;

    virtual antlrcpp::Any visitFunctionDefinition(titaniaParser::FunctionDefinitionContext *context) = 0;

    virtual antlrcpp::Any visitBody(titaniaParser::BodyContext *context) = 0;

    virtual antlrcpp::Any visitVarSection(titaniaParser::VarSectionContext *context) = 0;

    virtual antlrcpp::Any visitVarElem(titaniaParser::VarElemContext *context) = 0;

    virtual antlrcpp::Any visitModuleBodySection(titaniaParser::ModuleBodySectionContext *context) = 0;

    virtual antlrcpp::Any visitStatementList(titaniaParser::StatementListContext *context) = 0;

    virtual antlrcpp::Any visitAssignment(titaniaParser::AssignmentContext *context) = 0;

    virtual antlrcpp::Any visitWhileDo(titaniaParser::WhileDoContext *context) = 0;

    virtual antlrcpp::Any visitIfThen(titaniaParser::IfThenContext *context) = 0;

    virtual antlrcpp::Any visitExprStmnt(titaniaParser::ExprStmntContext *context) = 0;

    virtual antlrcpp::Any visitArrayAccess(titaniaParser::ArrayAccessContext *context) = 0;

    virtual antlrcpp::Any visitArithExp(titaniaParser::ArithExpContext *context) = 0;

    virtual antlrcpp::Any visitArithmaticIf(titaniaParser::ArithmaticIfContext *context) = 0;

    virtual antlrcpp::Any visitDereferencePointer(titaniaParser::DereferencePointerContext *context) = 0;

    virtual antlrcpp::Any visitLogicExp(titaniaParser::LogicExpContext *context) = 0;

    virtual antlrcpp::Any visitFieldAccess(titaniaParser::FieldAccessContext *context) = 0;

    virtual antlrcpp::Any visitStringLit(titaniaParser::StringLitContext *context) = 0;

    virtual antlrcpp::Any visitTakeReference(titaniaParser::TakeReferenceContext *context) = 0;

    virtual antlrcpp::Any visitAddOp(titaniaParser::AddOpContext *context) = 0;

    virtual antlrcpp::Any visitIdentifier(titaniaParser::IdentifierContext *context) = 0;

    virtual antlrcpp::Any visitPrefixNegative(titaniaParser::PrefixNegativeContext *context) = 0;

    virtual antlrcpp::Any visitNumberLit(titaniaParser::NumberLitContext *context) = 0;

    virtual antlrcpp::Any visitMultOp(titaniaParser::MultOpContext *context) = 0;

    virtual antlrcpp::Any visitGrouping(titaniaParser::GroupingContext *context) = 0;

    virtual antlrcpp::Any visitFunctionCall(titaniaParser::FunctionCallContext *context) = 0;

    virtual antlrcpp::Any visitOrOp(titaniaParser::OrOpContext *context) = 0;

    virtual antlrcpp::Any visitCompOp(titaniaParser::CompOpContext *context) = 0;

    virtual antlrcpp::Any visitRecusive(titaniaParser::RecusiveContext *context) = 0;

    virtual antlrcpp::Any visitNotOp(titaniaParser::NotOpContext *context) = 0;

    virtual antlrcpp::Any visitBoolLit(titaniaParser::BoolLitContext *context) = 0;

    virtual antlrcpp::Any visitAndOp(titaniaParser::AndOpContext *context) = 0;

    virtual antlrcpp::Any visitBoolLiteral(titaniaParser::BoolLiteralContext *context) = 0;


};

