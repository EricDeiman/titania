
// Generated from titania.g4 by ANTLR 4.7

#pragma once


#include "antlr4-runtime.h"




class  titaniaParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, T__16 = 17, T__17 = 18, T__18 = 19, T__19 = 20, 
    T__20 = 21, T__21 = 22, T__22 = 23, T__23 = 24, T__24 = 25, T__25 = 26, 
    T__26 = 27, T__27 = 28, T__28 = 29, T__29 = 30, T__30 = 31, T__31 = 32, 
    T__32 = 33, T__33 = 34, T__34 = 35, T__35 = 36, T__36 = 37, T__37 = 38, 
    T__38 = 39, T__39 = 40, T__40 = 41, T__41 = 42, T__42 = 43, T__43 = 44, 
    T__44 = 45, STRING = 46, COMMENT = 47, NUMBER = 48, DIGIT = 49, ID = 50, 
    WS = 51
  };

  enum {
    RuleFile = 0, RuleFromSection = 1, RuleFromElem = 2, RuleTypeSection = 3, 
    RuleTypeElem = 4, RuleIdDecl = 5, RuleConstSection = 6, RuleConstElem = 7, 
    RuleFunctionSection = 8, RuleFunctionElement = 9, RuleBody = 10, RuleVarSection = 11, 
    RuleVarElem = 12, RuleModuleBodySection = 13, RuleStatementList = 14, 
    RuleStatement = 15, RuleExpression = 16, RuleArithExpression = 17, RuleLogicExpression = 18, 
    RuleBoolLiteral = 19
  };

  titaniaParser(antlr4::TokenStream *input);
  ~titaniaParser();

  virtual std::string getGrammarFileName() const override;
  virtual const antlr4::atn::ATN& getATN() const override { return _atn; };
  virtual const std::vector<std::string>& getTokenNames() const override { return _tokenNames; }; // deprecated: use vocabulary instead.
  virtual const std::vector<std::string>& getRuleNames() const override;
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;


  class FileContext;
  class FromSectionContext;
  class FromElemContext;
  class TypeSectionContext;
  class TypeElemContext;
  class IdDeclContext;
  class ConstSectionContext;
  class ConstElemContext;
  class FunctionSectionContext;
  class FunctionElementContext;
  class BodyContext;
  class VarSectionContext;
  class VarElemContext;
  class ModuleBodySectionContext;
  class StatementListContext;
  class StatementContext;
  class ExpressionContext;
  class ArithExpressionContext;
  class LogicExpressionContext;
  class BoolLiteralContext; 

  class  FileContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *idToken = nullptr;;
    std::vector<antlr4::Token *> name;;
    std::vector<antlr4::Token *> params;;
    FileContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    FromSectionContext *fromSection();
    TypeSectionContext *typeSection();
    ConstSectionContext *constSection();
    FunctionSectionContext *functionSection();
    VarSectionContext *varSection();
    ModuleBodySectionContext *moduleBodySection();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FileContext* file();

  class  FromSectionContext : public antlr4::ParserRuleContext {
  public:
    FromSectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FromElemContext *> fromElem();
    FromElemContext* fromElem(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FromSectionContext* fromSection();

  class  FromElemContext : public antlr4::ParserRuleContext {
  public:
    FromElemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FromElemContext() : antlr4::ParserRuleContext() { }
    void copyFrom(FromElemContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ImportContext : public FromElemContext {
  public:
    ImportContext(FromElemContext *ctx);

    antlr4::Token *name = nullptr;
    antlr4::Token *idToken = nullptr;
    std::vector<antlr4::Token *> uses;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  InstantiateContext : public FromElemContext {
  public:
    InstantiateContext(FromElemContext *ctx);

    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FromElemContext* fromElem();

  class  TypeSectionContext : public antlr4::ParserRuleContext {
  public:
    TypeSectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<TypeElemContext *> typeElem();
    TypeElemContext* typeElem(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  TypeSectionContext* typeSection();

  class  TypeElemContext : public antlr4::ParserRuleContext {
  public:
    TypeElemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    TypeElemContext() : antlr4::ParserRuleContext() { }
    void copyFrom(TypeElemContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  TypeFunctionPointerContext : public TypeElemContext {
  public:
    TypeFunctionPointerContext(TypeElemContext *ctx);

    antlr4::Token *typeName = nullptr;
    antlr4::Token *idToken = nullptr;
    std::vector<antlr4::Token *> params;
    antlr4::Token *base = nullptr;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TypeArrayContext : public TypeElemContext {
  public:
    TypeArrayContext(TypeElemContext *ctx);

    antlr4::Token *typeName = nullptr;
    antlr4::Token *base = nullptr;
    antlr4::Token *length = nullptr;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    antlr4::tree::TerminalNode *NUMBER();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TypePointerContext : public TypeElemContext {
  public:
    TypePointerContext(TypeElemContext *ctx);

    antlr4::Token *typeName = nullptr;
    antlr4::Token *base = nullptr;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TypeAliasContext : public TypeElemContext {
  public:
    TypeAliasContext(TypeElemContext *ctx);

    antlr4::Token *typeName = nullptr;
    antlr4::Token *base = nullptr;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TypeRecordContext : public TypeElemContext {
  public:
    TypeRecordContext(TypeElemContext *ctx);

    antlr4::Token *typeName = nullptr;
    std::vector<IdDeclContext *> idDecl();
    IdDeclContext* idDecl(size_t i);
    antlr4::tree::TerminalNode *ID();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  TypeElemContext* typeElem();

  class  IdDeclContext : public antlr4::ParserRuleContext {
  public:
    antlr4::Token *name = nullptr;;
    antlr4::Token *type = nullptr;;
    IdDeclContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  IdDeclContext* idDecl();

  class  ConstSectionContext : public antlr4::ParserRuleContext {
  public:
    ConstSectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ConstElemContext *> constElem();
    ConstElemContext* constElem(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstSectionContext* constSection();

  class  ConstElemContext : public antlr4::ParserRuleContext {
  public:
    ConstElemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdDeclContext *idDecl();
    ExpressionContext *expression();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ConstElemContext* constElem();

  class  FunctionSectionContext : public antlr4::ParserRuleContext {
  public:
    FunctionSectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<FunctionElementContext *> functionElement();
    FunctionElementContext* functionElement(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  FunctionSectionContext* functionSection();

  class  FunctionElementContext : public antlr4::ParserRuleContext {
  public:
    FunctionElementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    FunctionElementContext() : antlr4::ParserRuleContext() { }
    void copyFrom(FunctionElementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ForwardFunctionContext : public FunctionElementContext {
  public:
    ForwardFunctionContext(FunctionElementContext *ctx);

    antlr4::Token *fnName = nullptr;
    antlr4::Token *idToken = nullptr;
    std::vector<antlr4::Token *> params;
    antlr4::Token *fnType = nullptr;
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FunctionDefinitionContext : public FunctionElementContext {
  public:
    FunctionDefinitionContext(FunctionElementContext *ctx);

    antlr4::Token *fnName = nullptr;
    antlr4::Token *fnType = nullptr;
    BodyContext *body();
    std::vector<antlr4::tree::TerminalNode *> ID();
    antlr4::tree::TerminalNode* ID(size_t i);
    std::vector<IdDeclContext *> idDecl();
    IdDeclContext* idDecl(size_t i);
    FromSectionContext *fromSection();
    TypeSectionContext *typeSection();
    ConstSectionContext *constSection();
    VarSectionContext *varSection();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  FunctionElementContext* functionElement();

  class  BodyContext : public antlr4::ParserRuleContext {
  public:
    BodyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StatementListContext *statementList();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BodyContext* body();

  class  VarSectionContext : public antlr4::ParserRuleContext {
  public:
    VarSectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<VarElemContext *> varElem();
    VarElemContext* varElem(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarSectionContext* varSection();

  class  VarElemContext : public antlr4::ParserRuleContext {
  public:
    VarElemContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    IdDeclContext *idDecl();
    ExpressionContext *expression();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  VarElemContext* varElem();

  class  ModuleBodySectionContext : public antlr4::ParserRuleContext {
  public:
    ModuleBodySectionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StatementListContext *statementList();

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  ModuleBodySectionContext* moduleBodySection();

  class  StatementListContext : public antlr4::ParserRuleContext {
  public:
    StatementListContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<StatementContext *> statement();
    StatementContext* statement(size_t i);

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  StatementListContext* statementList();

  class  StatementContext : public antlr4::ParserRuleContext {
  public:
    StatementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    StatementContext() : antlr4::ParserRuleContext() { }
    void copyFrom(StatementContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AssignmentContext : public StatementContext {
  public:
    AssignmentContext(StatementContext *ctx);

    titaniaParser::ExpressionContext *lval = nullptr;
    titaniaParser::ExpressionContext *rval = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ExprStmntContext : public StatementContext {
  public:
    ExprStmntContext(StatementContext *ctx);

    ExpressionContext *expression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  WhileDoContext : public StatementContext {
  public:
    WhileDoContext(StatementContext *ctx);

    titaniaParser::ExpressionContext *test = nullptr;
    titaniaParser::StatementListContext *whileBody = nullptr;
    ExpressionContext *expression();
    StatementListContext *statementList();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IfThenContext : public StatementContext {
  public:
    IfThenContext(StatementContext *ctx);

    titaniaParser::ExpressionContext *test = nullptr;
    titaniaParser::StatementListContext *thenBody = nullptr;
    titaniaParser::StatementListContext *elseBody = nullptr;
    ExpressionContext *expression();
    std::vector<StatementListContext *> statementList();
    StatementListContext* statementList(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  StatementContext* statement();

  class  ExpressionContext : public antlr4::ParserRuleContext {
  public:
    ExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ExpressionContext() : antlr4::ParserRuleContext() { }
    void copyFrom(ExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  ArrayAccessContext : public ExpressionContext {
  public:
    ArrayAccessContext(ExpressionContext *ctx);

    titaniaParser::ExpressionContext *base = nullptr;
    titaniaParser::ExpressionContext *index = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ArithExpContext : public ExpressionContext {
  public:
    ArithExpContext(ExpressionContext *ctx);

    ArithExpressionContext *arithExpression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  ArithmaticIfContext : public ExpressionContext {
  public:
    ArithmaticIfContext(ExpressionContext *ctx);

    titaniaParser::ExpressionContext *test = nullptr;
    titaniaParser::ExpressionContext *consq = nullptr;
    titaniaParser::ExpressionContext *altrn = nullptr;
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  DereferencePointerContext : public ExpressionContext {
  public:
    DereferencePointerContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  LogicExpContext : public ExpressionContext {
  public:
    LogicExpContext(ExpressionContext *ctx);

    LogicExpressionContext *logicExpression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FieldAccessContext : public ExpressionContext {
  public:
    FieldAccessContext(ExpressionContext *ctx);

    titaniaParser::ExpressionContext *base = nullptr;
    antlr4::Token *field = nullptr;
    ExpressionContext *expression();
    antlr4::tree::TerminalNode *ID();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  StringLitContext : public ExpressionContext {
  public:
    StringLitContext(ExpressionContext *ctx);

    antlr4::tree::TerminalNode *STRING();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  TakeReferenceContext : public ExpressionContext {
  public:
    TakeReferenceContext(ExpressionContext *ctx);

    ExpressionContext *expression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ExpressionContext* expression();
  ExpressionContext* expression(int precedence);
  class  ArithExpressionContext : public antlr4::ParserRuleContext {
  public:
    ArithExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    ArithExpressionContext() : antlr4::ParserRuleContext() { }
    void copyFrom(ArithExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  AddOpContext : public ArithExpressionContext {
  public:
    AddOpContext(ArithExpressionContext *ctx);

    titaniaParser::ArithExpressionContext *left = nullptr;
    antlr4::Token *op = nullptr;
    titaniaParser::ArithExpressionContext *right = nullptr;
    std::vector<ArithExpressionContext *> arithExpression();
    ArithExpressionContext* arithExpression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  IdentifierContext : public ArithExpressionContext {
  public:
    IdentifierContext(ArithExpressionContext *ctx);

    antlr4::tree::TerminalNode *ID();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  PrefixNegativeContext : public ArithExpressionContext {
  public:
    PrefixNegativeContext(ArithExpressionContext *ctx);

    ExpressionContext *expression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NumberLitContext : public ArithExpressionContext {
  public:
    NumberLitContext(ArithExpressionContext *ctx);

    antlr4::tree::TerminalNode *NUMBER();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  MultOpContext : public ArithExpressionContext {
  public:
    MultOpContext(ArithExpressionContext *ctx);

    titaniaParser::ArithExpressionContext *left = nullptr;
    antlr4::Token *op = nullptr;
    titaniaParser::ArithExpressionContext *right = nullptr;
    std::vector<ArithExpressionContext *> arithExpression();
    ArithExpressionContext* arithExpression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  GroupingContext : public ArithExpressionContext {
  public:
    GroupingContext(ArithExpressionContext *ctx);

    ExpressionContext *expression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  FunctionCallContext : public ArithExpressionContext {
  public:
    FunctionCallContext(ArithExpressionContext *ctx);

    antlr4::Token *name = nullptr;
    titaniaParser::ExpressionContext *expressionContext = nullptr;
    std::vector<ExpressionContext *> args;
    antlr4::tree::TerminalNode *ID();
    std::vector<ExpressionContext *> expression();
    ExpressionContext* expression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  ArithExpressionContext* arithExpression();
  ArithExpressionContext* arithExpression(int precedence);
  class  LogicExpressionContext : public antlr4::ParserRuleContext {
  public:
    LogicExpressionContext(antlr4::ParserRuleContext *parent, size_t invokingState);
   
    LogicExpressionContext() : antlr4::ParserRuleContext() { }
    void copyFrom(LogicExpressionContext *context);
    using antlr4::ParserRuleContext::copyFrom;

    virtual size_t getRuleIndex() const override;

   
  };

  class  OrOpContext : public LogicExpressionContext {
  public:
    OrOpContext(LogicExpressionContext *ctx);

    titaniaParser::LogicExpressionContext *left = nullptr;
    titaniaParser::LogicExpressionContext *right = nullptr;
    std::vector<LogicExpressionContext *> logicExpression();
    LogicExpressionContext* logicExpression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  CompOpContext : public LogicExpressionContext {
  public:
    CompOpContext(LogicExpressionContext *ctx);

    titaniaParser::LogicExpressionContext *left = nullptr;
    antlr4::Token *op = nullptr;
    titaniaParser::LogicExpressionContext *right = nullptr;
    std::vector<LogicExpressionContext *> logicExpression();
    LogicExpressionContext* logicExpression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  RecusiveContext : public LogicExpressionContext {
  public:
    RecusiveContext(LogicExpressionContext *ctx);

    ArithExpressionContext *arithExpression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  NotOpContext : public LogicExpressionContext {
  public:
    NotOpContext(LogicExpressionContext *ctx);

    LogicExpressionContext *logicExpression();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  BoolLitContext : public LogicExpressionContext {
  public:
    BoolLitContext(LogicExpressionContext *ctx);

    BoolLiteralContext *boolLiteral();
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  class  AndOpContext : public LogicExpressionContext {
  public:
    AndOpContext(LogicExpressionContext *ctx);

    titaniaParser::LogicExpressionContext *left = nullptr;
    titaniaParser::LogicExpressionContext *right = nullptr;
    std::vector<LogicExpressionContext *> logicExpression();
    LogicExpressionContext* logicExpression(size_t i);
    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
  };

  LogicExpressionContext* logicExpression();
  LogicExpressionContext* logicExpression(int precedence);
  class  BoolLiteralContext : public antlr4::ParserRuleContext {
  public:
    BoolLiteralContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;

    virtual antlrcpp::Any accept(antlr4::tree::ParseTreeVisitor *visitor) override;
   
  };

  BoolLiteralContext* boolLiteral();


  virtual bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;
  bool expressionSempred(ExpressionContext *_localctx, size_t predicateIndex);
  bool arithExpressionSempred(ArithExpressionContext *_localctx, size_t predicateIndex);
  bool logicExpressionSempred(LogicExpressionContext *_localctx, size_t predicateIndex);

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

