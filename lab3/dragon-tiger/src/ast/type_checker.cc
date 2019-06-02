#include "type_checker.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"
  
using utils::error;
using utils::non_fatal_error;


namespace ast{
namespace type_checker{

void TypeChecker::visit(IntegerLiteral &id){
	id.accept(*this);
	id.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &str){
	str.accept(*this);
	str.set_type(t_string);
}

void TypeChecker::visit(BinaryOperator &op){
	
}

void TypeChecker::visit(Sequence &seq){
	std::vector<Expr *> exprs = seq.get_exprs();
	Type type = exprs[(int)exprs.size() - 1]->get_type();
	if(type != t_undef)
		seq.set_type(type);
	else
		seq.set_type(t_void);
}

void TypeChecker::visit(Let &let){
	std::vector<Decl *> decls = let.get_decls();
      	Type type = decls[(int)decls.size() - 1]->get_type();
	if(type != t_undef)
		let.set_type(type);
	else
		let.set_type(t_void);

}

void TypeChecker::visit(Identifier &){

}

void TypeChecker::visit(IfThenElse &ite){
	//Type type = ite.get_condition()->get_type();
}

void TypeChecker::visit(VarDecl &decl){
	optional<Expr &> expr = decl.get_expr();
	optional<Symbol> type = decl.type_name;
      	/*
	if(type != t_void)
		decl.set_type(type);
	else
		error("Variable declarartion must be int or string");
	*/
}

void TypeChecker::visit(FunDecl &){

}

void TypeChecker::visit(FunCall &){

}

void TypeChecker::visit(WhileLoop &){

}

void TypeChecker::visit(ForLoop &){

}

void TypeChecker::visit(Break &){

}

void TypeChecker::visit(Assign &){

}


}
}


