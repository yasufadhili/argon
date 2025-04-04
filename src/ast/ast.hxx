#pragma once

#include <memory>
#include <optional>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "../token/token.hxx"

namespace ast {

  class Visitor;

  struct Node {
    virtual void accept(Visitor&) = 0;
    virtual ~Node() = default;
  };

  struct Identifier final {
    std::string name;
    std::string type;
    std::shared_ptr<Node> value;

    Identifier(std::string name, std::string type) : name(std::move(name)), type(std::move(type)) {}
    ~Identifier() = default;
  };

  namespace expr {

    struct Expression : Node {
      void accept(Visitor &v) override = 0;

      ~Expression() override = default;
    };

    struct Unary final : Expression {
      token::Token op;
      std::shared_ptr<Expression> operand;
      void accept(Visitor &) override;
      Unary(token::Token op, std::shared_ptr<Expression> operand) : op(std::move(op)), operand(std::move(operand)) {}
      ~Unary() override = default;
    };

    struct Binary final : Expression {
      token::Token op;
      std::shared_ptr<Expression> lhs, rhs;
      void accept(Visitor &v) override;
      Binary(token::Token op, std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
      ~Binary() override = default;
    };

    struct Ternary final : Expression {
      std::shared_ptr<Expression> condition, lhs, rhs;
      void accept(Visitor &v) override;
      Ternary(std::shared_ptr<Expression> condition, std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs) : condition(std::move(condition)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
      ~Ternary() override = default;
    };

    struct Grouping final : Expression {
      std::shared_ptr<Expression> expr;
      void accept(Visitor &v) override;
      explicit Grouping(std::shared_ptr<Expression> expr) : expr(std::move(expr)) {}
      ~Grouping() override = default;
    };

    struct Literal final : Expression {
      std::string value;
      void accept(Visitor &v) override;
      explicit Literal(std::string value) : value(std::move(value)) {}
      ~Literal() override = default;
    };

  }

  namespace stmt {

    struct Statement : Node {
      void accept(Visitor &v) override = 0;

      ~Statement() override = default;
    };

    struct Block final: Statement {
      std::vector<std::shared_ptr<Statement>> stmts;
      void accept(Visitor &v) override;

      explicit Block(std::vector<std::shared_ptr<Statement>> stmts) : stmts(std::move(stmts)) {}
      ~Block() override = default;
    };

    struct Return final : Statement {
      std::optional<std::shared_ptr<expr::Expression>> expr;
      void accept(Visitor &v) override;
      explicit Return(const std::optional<std::shared_ptr<expr::Expression>> &exp) : expr(exp) {};
      ~Return() override = default;
    };

    struct Var final : Statement {
      std::string name;
      std::string type;
      std::optional<std::shared_ptr<expr::Expression>> value;
      void accept(Visitor &v) override;
      Var(std::string name, std::string type, const std::optional<std::shared_ptr<expr::Expression>>& value) : name(std::move(name)), type(std::move(type)), value(value) {}
      ~Var() override = default;
    };

  }

  namespace func {

    struct Function final : Node {
      std::string name;
      std::string rt;
      std::shared_ptr<stmt::Block> body;
      void accept(Visitor &v) override;

      Function(std::string name, std::string rt, std::shared_ptr<stmt::Block> body) : name(std::move(name)), rt(std::move(rt)), body(std::move(body)) {}
      ~Function() override = default;
    };

  }

  namespace module {

    struct Module final : Node {
      std::string name;
      std::vector<std::shared_ptr<func::Function>> funcs;
      void accept(Visitor &v) override;
      Module(std::string name, const std::vector<std::shared_ptr<func::Function>>& fns) : name(std::move(name)), funcs(fns) {};
      ~Module() override = default;
    };

  }

  namespace prog {

    struct Program final : Node {
      std::vector<std::shared_ptr<module::Module>> modules;

      void accept(Visitor &v) override;
      explicit Program(std::vector<std::shared_ptr<module::Module>> modules) : modules(std::move(modules)) {}
      ~Program() override = default;
    };

  }


  class Visitor {
    std::stringstream output_ {};

    void emit(const std::string&);

    static std::string generate_label(const std::string&);

  public:

    Visitor();

    virtual ~Visitor() = default;

    auto get_output() const -> std::string;

    virtual void visit(const prog::Program&);
    virtual void visit(const module::Module&);
    virtual void visit(const func::Function&);
    virtual void visit(const stmt::Statement&);
    virtual void visit(const expr::Expression&);
    virtual void visit(const expr::Unary&);
    virtual void visit(const expr::Binary&);
    virtual void visit(const expr::Ternary&);
    virtual void visit(const expr::Grouping&);
    virtual void visit(const stmt::Block&);
    virtual void visit(const stmt::Return&);
    virtual void visit(const expr::Literal&);
    virtual void visit(const stmt::Var&);

  };

  inline void prog::Program::accept(Visitor &v) { v.visit(*this); }
  inline void module::Module::accept(Visitor &v) { v.visit(*this); }
  inline void func::Function::accept(Visitor &v) { v.visit(*this); }
  inline void stmt::Statement::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Expression::accept(Visitor &v) { v.visit(*this); }
  inline void stmt::Block::accept(Visitor &v) { v.visit(*this); }
  inline void stmt::Return::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Literal::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Unary::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Binary::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Ternary::accept(Visitor &v) { v.visit(*this); }
  inline void expr::Grouping::accept(Visitor &v) { v.visit(*this); }
  inline void stmt::Var::accept(Visitor &v) { v.visit(*this); }




}


