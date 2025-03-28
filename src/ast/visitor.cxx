
#include "ast.hxx"

namespace ast {

  Visitor::Visitor() {
    emit("");
  }


  void Visitor::emit(const std::string &code) {
    output_ << code << " \n";
  }

  auto Visitor::get_output() const -> std::string {
    return output_.str();
  }

  auto Visitor::generate_label(const std::string &) -> std::string {
    return "";
  }

  void Visitor::visit(const prog::Program &p) {
    emit("Program");
    for (const auto &m : p.modules) {
      m->accept(*this);
    }
  }

  void Visitor::visit(const module::Module &m) {
    emit("Module - " + m.name + " " + std::to_string(m.funcs.size()));
    for (const auto &f : m.funcs) {
      f->accept(*this);
    }
  }

  void Visitor::visit(const func::Function &f) {
    emit("Function " + f.name);
    f.body->accept(*this);
  }

  void Visitor::visit(const stmt::Statement &s) {
    emit("Statement");
  }

  void Visitor::visit(const expr::Expression &e) {
    emit("Expression");
  }

  void Visitor::visit(const stmt::Block &b) {
    emit("Block");
    for (const auto &stmt : b.stmts) {
      stmt->accept(*this);
    }
  }

  void Visitor::visit(const expr::Literal &l) {
    emit("Literal " + l.value);
  }

  void Visitor::visit(const stmt::Return &r) {
    emit("Return ");
    if (r.expr.has_value()) {
      r.expr.value()->accept(*this);
    }
  }




}
