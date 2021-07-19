
// Generated from titania.g4 by ANTLR 4.7

#pragma once


#include "antlr4-runtime.h"
#include "titaniaVisitor.h"


/**
 * This class provides an empty implementation of titaniaVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  titaniaBaseVisitor : public titaniaVisitor {
public:

  virtual antlrcpp::Any visitFile(titaniaParser::FileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFromSection(titaniaParser::FromSectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInstantiate(titaniaParser::InstantiateContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitImport(titaniaParser::ImportContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeSection(titaniaParser::TypeSectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeRecord(titaniaParser::TypeRecordContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeAlias(titaniaParser::TypeAliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypePointer(titaniaParser::TypePointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeFunctionPointer(titaniaParser::TypeFunctionPointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeArray(titaniaParser::TypeArrayContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdDecl(titaniaParser::IdDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstSection(titaniaParser::ConstSectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitConstElem(titaniaParser::ConstElemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionSection(titaniaParser::FunctionSectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForwardFunction(titaniaParser::ForwardFunctionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionDefinition(titaniaParser::FunctionDefinitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBody(titaniaParser::BodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarSection(titaniaParser::VarSectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarElem(titaniaParser::VarElemContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitModuleBodySection(titaniaParser::ModuleBodySectionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatementList(titaniaParser::StatementListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignment(titaniaParser::AssignmentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhileDo(titaniaParser::WhileDoContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfThen(titaniaParser::IfThenContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExprStmnt(titaniaParser::ExprStmntContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArrayAccess(titaniaParser::ArrayAccessContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArithExp(titaniaParser::ArithExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArithmaticIf(titaniaParser::ArithmaticIfContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDereferencePointer(titaniaParser::DereferencePointerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLogicExp(titaniaParser::LogicExpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFieldAccess(titaniaParser::FieldAccessContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStringLit(titaniaParser::StringLitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTakeReference(titaniaParser::TakeReferenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAddOp(titaniaParser::AddOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIdentifier(titaniaParser::IdentifierContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPrefixNegative(titaniaParser::PrefixNegativeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNumberLit(titaniaParser::NumberLitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMultOp(titaniaParser::MultOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitGrouping(titaniaParser::GroupingContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionCall(titaniaParser::FunctionCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitOrOp(titaniaParser::OrOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCompOp(titaniaParser::CompOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitRecusive(titaniaParser::RecusiveContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNotOp(titaniaParser::NotOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolLit(titaniaParser::BoolLitContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAndOp(titaniaParser::AndOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolLiteral(titaniaParser::BoolLiteralContext *ctx) override {
    return visitChildren(ctx);
  }


};

