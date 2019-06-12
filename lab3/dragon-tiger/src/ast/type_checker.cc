#include "type_checker.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"
#include <string>

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
	Expr* last_expr = exprs[(int)exprs.size()-1];
	last_expr->accept(*this);
	Type type = last_expr->get_type();
	if(type != t_undef)
		seq.set_type(type);
	else
		seq.set_type(t_void);
}

void TypeChecker::visit(Let &let){
	std::vector<Decl *> decls = let.get_decls();
      	Decl* last_decl = decls[(int)decls.size()-1];
	last_decl->accept(*this);
	Type type = last_decl->get_type();
	if(type != t_undef)
		let.set_type(type);
	else
		let.set_type(t_void);

}

void TypeChecker::visit(Identifier &){

}

void TypeChecker::visit(IfThenElse &ite){
	ite.get_condition().accept(*this);
	Type type_if = ite.get_condition().get_type();
	ite.get_then_part().accept(*this);
	Type type_then = ite.get_then_part().get_type();
	ite.get_else_part().accept(*this);
	Type type_else = ite.get_else_part().get_type();
	if(type_if == t_int && type_then == type_else)
		ite.set_type(type_then);
	else
		error("Condition isn't int or then and else part do not have the same type");
		
}

Type TypeChecker::symbol_to_type(Symbol type_s){
	std::string type = type_s.get();
	if (type == "int")
		return(t_int);
	if(type == "string")
		return(t_string);
	if(type == "void")
	       error("Variable cannot be void");
	else
		error("Type undefined");	
}

void TypeChecker::visit(VarDecl &decl){
	decl.accept(*this);
	optional<Expr &> expr = decl.get_expr();
	optional<Symbol> type = decl.type_name;
	if(type){
		if(symbol_to_type(*type) == expr->get_type())
			decl.set_type(expr->get_type());
	}
	else{
		Type type_expr = expr->get_type();
		if(type_expr == t_int || type_expr  == t_string)
			decl.set_type(type_expr);
		else
			error("Variable type must be integer or string, not void");
		
	}
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


