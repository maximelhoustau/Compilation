#include "ast_evaluator.hh"
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

int32_t ASTEvaluator::visit(const IntegerLiteral &literal) {
  return(literal.value);
}

int32_t ASTEvaluator::visit(const BinaryOperator &binop) {
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

int32_t ASTEvaluator::visit(const Sequence &seqExpr) {
  const auto exprs = seqExpr.get_exprs();
  if(exprs.size() == 0)
	  utils::error("Error: the sequence is empty");
  for (auto expr = exprs.cbegin(); expr != exprs.cend(); expr++) {
    if (expr != exprs.cend())
	    return( (*expr)->accept(*this));
    else
	    (*expr)->accept(*this);
  }
  return(-1);
}

int32_t ASTEvaluator::visit(const IfThenElse &ite) {
  int cond = ite.get_condition().accept(*this);
  if(cond == 1)
	  return(ite.get_then_part().accept(*this));
  else
	  return(ite.get_else_part().accept(*this));
}

int32_t ASTEvaluator::visit(const StringLiteral &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const Let &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const Identifier &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const VarDecl &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const FunDecl &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const FunCall &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const WhileLoop &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const ForLoop &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const Break &) {utils::error("Node not implemented yet");}
int32_t ASTEvaluator::visit(const Assign &) {utils::error("Node not implemented yet");}


} // namespace ast
