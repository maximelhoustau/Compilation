#include "evaluator.hh"
#include "../utils/errors.hh"

namespace {

char const * const get_type_name(ast::Type t) {
  switch (t) {
    case t_int:
      return "int";
    case t_string:
      return "string";
    default:
       utils::error("internal error: attempting to print the type of t_void or t_undef");
  }
}

} // namespace


namespace ast {

int ASTEvaluator::visit(const IntegerLiteral &literal) {
  return(literal.value);
}

int ASTEvaluator::visit(const BinaryOperator &binop) {
  int a = binop.get_left().accept(*this);
  int b = binop.get_right().accept(*this);
  std::string op = operator_name[binop.op];
  if(op == "+")
	  return(a+b);
  if(op == "-")
	  return(a-b);
  if(op == "*")
	  return(a*b);
  if(op == "/")
	  return(a/b);
  if(op == "=")
	  return(a==b);
  if(op == "<>")
	  return(a!=b);
  if(op == "<")
	  return(a<b);
  if(op == "<=")
	  return(a<=b);
  if(op == ">")
	  return(a>b);
  if(op == ">=")
	  return(a>=b);
}

int ASTEvaluator::visit(const Sequence &seqExpr) {
  *ostream << "(";
  inc();
  const auto exprs = seqExpr.get_exprs();
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    if (expr != exprs.cbegin())
      *ostream << ';';
    nl();
    (*expr)->accept(*this);
  }
  dnl();
  *ostream << ")";
}

int ASTEvaluator::visit(const IfThenElse &ite) {
  *ostream << "if ";
  inl();
  ite.get_condition().accept(*this);
  dnl();
  *ostream << " then ";
  inl();
  ite.get_then_part().accept(*this);
  dnl();
  *ostream << " else ";
  inl();
  ite.get_else_part().accept(*this);
  dec();
}


} // namespace ast
