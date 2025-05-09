%require "3.2"
%language "c++"

%define api.parser.class {Parser}
%define api.value.type variant
%define api.value.automove true
%define api.token.raw
%define api.token.constructor

%define parse.assert
%define parse.trace
%define parse.error detailed
%define parse.lac full

%locations
%define api.location.file "location.hh"


%param {yy::Lexer &lexer}



%code requires {

#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include "include/ast.hh"
#include "include/util_logger.hh"
#include "error_handler.hh"


namespace yy {
  class Lexer;
}
}



%code {
  #include "lexer.hh"

  yy::Parser::symbol_type yylex(yy::Lexer &lexer) {
      return lexer.lex();
  }
}


%token END 0 "end of file"
%token NEW_LINE "new line"

%token ASM
%token <std::string> STRING_LITERAL

%token LBRACE
%token RBRACE
%token RPAREN
%token LPAREN

%token PLUS
%token MINUS
%token TIMES
%token DIVIDE
%token MODULO

%token LT
%token GT
%token EQ
%token LEQ
%token GEQ
%token NEQ

%token UNARY_MINUS
%token TILDE

%token LOGICAL_AND
%token LOGICAL_OR

%token BITWISE_AND
%token BITWISE_OR

%token SEMICOLON
%token ASSIGN
%token COMMA

%token BACK_TICK

%token <std::string> IDENT
%token <std::string> TYPE_IDENT

%token <int> INTEGER
%token <float> FLOAT



%token RETURN
%token VAR
%token MODULE
%token FN
%token PRINT


%parse-param  { std::shared_ptr<ast::mod::Module>& module }

%type <std::shared_ptr<ast::mod::Module>> module_definition;

%type <std::vector<std::shared_ptr<ast::func::Function>>> function_definition_list;
%type <std::shared_ptr<ast::func::Function>> function_definition;
%type <std::shared_ptr<ast::func::Body>> function_body;
%type <std::shared_ptr<ast::ident::Identifier>> function_identifier;

%type <std::shared_ptr<ast::func::ReturnTypeInfo>> function_return_type;
%type <std::vector<std::shared_ptr<ast::ident::TypeIdentifier>>> multiple_return_types;

%type <std::vector<std::shared_ptr<ast::func::Parameter>>> parameter_list;
%type <std::vector<std::shared_ptr<ast::func::Parameter>>> non_empty_parameter_list;
%type <std::shared_ptr<ast::func::Parameter>> parameter;

%type <std::vector<std::shared_ptr<ast::stmt::Statement>>> statement_list;
%type <std::shared_ptr<ast::stmt::Statement>> statement;
%type <std::shared_ptr<ast::stmt::Statement>> execution_statement;
%type <std::shared_ptr<ast::stmt::Block>> block_statement;
%type <std::shared_ptr<ast::stmt::Statement>> control_statement;
%type <std::shared_ptr<ast::stmt::Statement>> declaration_statement;

%type <std::shared_ptr<ast::stmt::Print>> print_statement;

%type <std::shared_ptr<ast::stmt::Assignment>> assignment_statement;

%type <std::shared_ptr<ast::stmt::Return>> return_statement;
%type <std::shared_ptr<ast::stmt::VariableDeclaration>> variable_declaration;

%type <std::shared_ptr<ast::expr::Expression>> expression;
%type <std::shared_ptr<ast::expr::Unary>> unary_expression;
%type <std::shared_ptr<ast::expr::Expression>> logical_expression;
%type <std::shared_ptr<ast::expr::Expression>> arithmetic_expression;
%type <std::shared_ptr<ast::expr::Expression>> bitwise_expression;
%type <std::shared_ptr<ast::expr::Expression>> relational_expression;

%type <std::shared_ptr<ast::expr::Expression>> term;
%type <std::shared_ptr<ast::expr::Expression>> factor;
%type <std::shared_ptr<ast::expr::Expression>> primary;
%type <std::shared_ptr<ast::expr::Literal>> literal;
%type <std::shared_ptr<ast::expr::Literal>> boolean_literal;

%type <std::shared_ptr<ast::expr::Variable>> variable;
%type <std::shared_ptr<ast::ident::TypeIdentifier>> type_identifier;
%type <std::shared_ptr<ast::ident::Identifier>> identifier;

%type <std::shared_ptr<sym_table::Type>> type_specifier;
%type <std::optional<std::shared_ptr<ast::expr::Expression>>> optional_initialiser;


// Operator precedence - from lowest to highest
%precedence ASSIGN
%left LOGICAL_OR
%left LOGICAL_AND
%left EQ NEQ
%left GT LT GEQ LEQ
%left PLUS MINUS
%left TIMES DIVIDE MODULO
%right NOT
%precedence UNARY_MINUS


%start module_definition



%%


module_definition
  : MODULE identifier SEMICOLON statement_list function_definition_list {
    module = std::make_shared<ast::mod::Module>($2, $4, $5);
    module->set_location(@$);
    $$ = module;
  }
  | MODULE identifier SEMICOLON function_definition_list {
    module = std::make_shared<ast::mod::Module>($2, $4);
    module->set_location(@$);
    $$ = module;
  }
  | MODULE identifier error {
    error::DiagnosticHandler::instance().error(
      "Missing semicolon after module declaration",
      @$,
      std::nullopt,
      "Add a semicolon after the module name"
    );
    // Recovery: assume empty module
    module = std::make_shared<ast::mod::Module>($2, std::vector<std::shared_ptr<ast::stmt::Statement>>{});
    module->set_location(@$);
    $$ = module;
  }
  | MODULE error {
    error::DiagnosticHandler::instance().error(
      "Invalid module declaration",
      @$,
      std::nullopt,
      "Module must be followed by an identifier"
    );
    // Recovery: create empty module with dummy identifier
    module = std::make_shared<ast::mod::Module>(
      std::make_shared<ast::ident::Identifier>("<invalid_module>"),
      std::vector<std::shared_ptr<ast::stmt::Statement>>{}
    );
    module->set_location(@$);
    $$ = module;
  }
;


function_definition_list
  : function_definition {
    $$ = std::vector<std::shared_ptr<ast::func::Function>> { $1 };
  }
  | function_definition_list function_definition {
    $$ = $1;
    $$.emplace_back($2);
  }
;


function_definition
  : FN function_identifier LPAREN RPAREN function_return_type function_body {
    $$ = std::make_shared<ast::func::Function>($2, std::vector<std::shared_ptr<ast::func::Parameter>>{}, $5, $6);
  }
  | FN function_identifier LPAREN parameter_list RPAREN function_return_type function_body {
    $$ = std::make_shared<ast::func::Function>($2, $4, $6, $7);
  }
  | FN function_identifier function_body {
    // No params, no return type
    $$ = std::make_shared<ast::func::Function>(
      $2,
      std::vector<std::shared_ptr<ast::func::Parameter>>{},
      nullptr, // No return type
      $3
    );
  }
  | FN function_identifier error function_body {
    error::DiagnosticHandler::instance().error(
        "Invalid function parameter declaration",
        @$
    );
    // Recovery by assuming no parameters
    $$ = std::make_shared<ast::func::Function>($2, std::vector<std::shared_ptr<ast::func::Parameter>>{}, nullptr, $4);
  }
;


function_identifier
  : IDENT {
    $$ = std::make_shared<ast::ident::Identifier>($1);
  }
;


function_return_type
  : type_identifier {
    $$ = std::make_shared<ast::func::SingleReturnType>($1);
  }
  | LPAREN multiple_return_types RPAREN {
    $$ = std::make_shared<ast::func::MultipleReturnType>($2);
  }
  | %empty {
    $$ = nullptr; // No return type
  }
;


multiple_return_types
  : type_identifier COMMA type_identifier {
    $$ = std::vector<std::shared_ptr<ast::ident::TypeIdentifier>> { $1, $3 };
  }
  | multiple_return_types COMMA type_identifier {
    $$ = $1;
    $$.push_back($3);
  }
;


parameter_list
  : %empty {
    $$ = std::vector<std::shared_ptr<ast::func::Parameter>>{};
  }
  | non_empty_parameter_list {
    $$ = $1;
  }
;


non_empty_parameter_list
  : parameter {
    $$ = std::vector<std::shared_ptr<ast::func::Parameter>> { $1 };
  }
  | non_empty_parameter_list COMMA parameter {
    $$ = $1;
    $$.push_back($3);
  }
;


parameter
  : function_identifier type_identifier {
    auto st = sym_table::SymbolTable::get_instance();
    auto t = st->lookup_type($2->name());
    if (t) {
      $$ = std::make_shared<ast::func::Parameter>($1, t);
    } else {
      $$ = std::make_shared<ast::func::Parameter>($1, nullptr);
    }
  }
;


function_body
  : statement_list {
    $$ = std::make_shared<ast::func::Body>($1);
  }
;


statement_list
  : statement {
    $$ = std::vector<std::shared_ptr<ast::stmt::Statement>>{$1};
  }
  | statement_list statement {
    $$ = $1;
    $$.push_back($2);
  }
;


statement
  : declaration_statement {
    $$ = $1;
  }
  | execution_statement {
    $$ = $1;
  }
  | control_statement {
    $$ = $1;
  }
  | assignment_statement {
    $$ = $1;
  }
  | error {
    error::DiagnosticHandler::instance().error(
        "Invalid statement syntax",
        @$,
        std::nullopt,
        "Expected a valid statement (declaration, execution, control, or assignment)"
    );
    $$ = std::make_shared<ast::stmt::Empty>();
    $$->set_location(@$);
  }
;


execution_statement
  : block_statement {
    $$ = $1;
  }
  | print_statement {
    $$ = $1;
  }
;


block_statement
  : LBRACE statement_list RBRACE {
    $$ = std::make_shared<ast::stmt::Block>($2);
    $$->set_location(@$);
  }
  | LBRACE RBRACE {
    $$ = std::make_shared<ast::stmt::Block>(
      std::vector<std::shared_ptr<ast::stmt::Statement>>{}
    );
    $$->set_location(@$);
  }
;


print_statement
  : PRINT expression SEMICOLON {
    $$ = std::make_shared<ast::stmt::Print>($2);
    $$->set_location(@$);
  }
  | PRINT expression error {
    error::DiagnosticHandler::instance().error(
        "Missing semicolon after print statement",
        @$,
        std::nullopt,
        "Add a semicolon after the print expression"
    );
    $$ = std::make_shared<ast::stmt::Print>($2);
    $$->set_location(@$);
  }
;


declaration_statement
  : variable_declaration SEMICOLON {
    $$ = $1;
  }
  | variable_declaration error {
    error::DiagnosticHandler::instance().error(
        "Missing semicolon after variable declaration",
        @$,
        std::nullopt,
        "Add a semicolon after the variable declaration"
    );
    $$ = $1;
  }
;


control_statement
  : return_statement SEMICOLON {
    $$ = $1;
  }
  | return_statement error {
    error::DiagnosticHandler::instance().error(
        "Missing semicolon after return statement",
        @$,
        std::nullopt,
        "Add a semicolon after the return statement"
    );
    $$ = $1;
  }
;


return_statement
  : RETURN arithmetic_expression {
    $$ = std::make_shared<ast::stmt::Return>($2);
    $$->set_location(@$);
  }
  | RETURN logical_expression {
    $$ = std::make_shared<ast::stmt::Return>($2);
    $$->set_location(@$);
  }
  | RETURN relational_expression {
    $$ = std::make_shared<ast::stmt::Return>($2);
    $$->set_location(@$);
  }
  | RETURN bitwise_expression {
    $$ = std::make_shared<ast::stmt::Return>($2);
    $$->set_location(@$);
  }
  | RETURN {
    $$ = std::make_shared<ast::stmt::Return>(std::nullopt);
    $$->set_location(@$);
  }
;


variable_declaration
  : VAR identifier type_specifier optional_initialiser {
    $$ = std::make_shared<ast::stmt::VariableDeclaration>($2, $3, $4);
    $$->set_location(@$);
  }
;


assignment_statement
  : identifier ASSIGN expression SEMICOLON {
    $$ = std::make_shared<ast::stmt::Assignment>($1, $3);
    $$->set_location(@$);
  }
  | identifier ASSIGN expression error {
    error::DiagnosticHandler::instance().error(
        "Missing semicolon after assignment",
        @$,
        std::nullopt,
        "Add a semicolon after the assignment expression"
    );
    $$ = std::make_shared<ast::stmt::Assignment>($1, $3);
    $$->set_location(@$);
  }
;


optional_initialiser
  : %empty {
    $$ = std::nullopt;
  }
  | ASSIGN expression {
    $$ = $2;
  }
;


type_specifier
  : TYPE_IDENT {
    $$ = std::make_shared<sym_table::Type>(sym_table::Type::TypeKind::PRIMITIVE, $1);
  }
;


expression
  : logical_expression {
    $$ = $1;
  }
  | bitwise_expression {
    $$ = $1;
  }
;


logical_expression
  : relational_expression {
    $$ = $1;
  }
  | logical_expression LOGICAL_AND relational_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::B_AND, $1, $3
    );
    $$->set_location(@$);
  }
  | logical_expression LOGICAL_OR relational_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::L_OR, $1, $3
    );
    $$->set_location(@$);
  }
;


bitwise_expression
  : arithmetic_expression BITWISE_AND arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::B_AND, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression BITWISE_OR arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::B_OR, $1, $3
    );
    $$->set_location(@$);
  }
;


relational_expression
  : arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::NONE, $1, nullptr
    );
    $$->set_location(@$);
  }
  | arithmetic_expression EQ arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::EQ, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression NEQ arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::NEQ, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression GT arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::GT, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression LT arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::LT, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression LEQ arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::LEQ, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression GEQ arithmetic_expression {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::RelationalOp::GEQ, $1, $3
    );
    $$->set_location(@$);
  }
;


arithmetic_expression
  : term {
    $$ = $1;
  }
  | arithmetic_expression PLUS term {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::ADD, $1, $3
    );
    $$->set_location(@$);
  }
  | arithmetic_expression MINUS term {
    $$ = std::make_shared<ast::expr::Binary>(
      ast::BinaryOp::SUB, $1, $3
    );
    $$->set_location(@$);
  }
;


term
  : factor {
    $$ = $1;
  }
  | term TIMES factor {
    $$ = std::make_shared<ast::expr::Binary>(ast::BinaryOp::MUL, $1, $3);
    $$->set_location(@$);
  }
  | term DIVIDE factor {
    $$ = std::make_shared<ast::expr::Binary>(ast::BinaryOp::DIV, $1, $3);
    $$->set_location(@$);
  }
  | term MODULO factor {
    $$ = std::make_shared<ast::expr::Binary>(ast::BinaryOp::MOD, $1, $3);
    $$->set_location(@$);
  }
;


unary_expression
  : TILDE factor {
    $$ = std::make_shared<ast::expr::Unary>(
      ast::UnaryOp::B_NOT, $2
    );
    $$->set_location(@$);
  }
  | MINUS factor %prec UNARY_MINUS {
    $$ = std::make_shared<ast::expr::Unary>(
      ast::UnaryOp::NEG, $2
    );
    $$->set_location(@$);
  }
;


factor
  : primary {
    $$ = $1;
  }
  | unary_expression {
    $$ = $1;
  }
;


primary
  : literal {
    $$ = $1;
  }
  | variable {
    $$ = $1;
  }
  | LPAREN arithmetic_expression RPAREN {
    $$ = $2;
    $$->set_location(@$);
  }
  | LPAREN bitwise_expression RPAREN {
    $$ = $2;
    $$->set_location(@$);
  }
;


literal
  : INTEGER {
    $$ = std::make_shared<ast::expr::Literal>($1);
    $$->set_location(@$);
  }
  | FLOAT {
    $$ = std::make_shared<ast::expr::Literal>($1);
    $$->set_location(@$);
  }
;


variable
  : identifier {
    $$ = std::make_shared<ast::expr::Variable>($1, nullptr);
    $$->set_location(@$);
  }
;


type_identifier
  : TYPE_IDENT {
    $$ = std::make_shared<ast::ident::TypeIdentifier>($1);
    $$->set_location(@$);
  }
;


identifier
  : IDENT {
    $$ = std::make_shared<ast::ident::Identifier>($1);
    $$->set_location(@$);
  }
;


%%

void yy::Parser::error(const location_type& loc, const std::string& msg)
{
  error::DiagnosticHandler::instance().error(
      msg,
      loc,
      std::nullopt,
      "Check the syntax and try again"
  );
}
