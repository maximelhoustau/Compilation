#include "type_checker.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"
#include <string>

using utils::error;
using utils::non_fatal_error;


namespace ast{
namespace type_checker{

void TypeChecker::visit(IntegerLiteral &id){
	//Int
	id.accept(*this);
	id.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &str){
	//String
	str.accept(*this);
	str.set_type(t_string);
}

void TypeChecker::visit(BinaryOperator &op){
	op.get_left().accept(*this);
	Type type_op_left = op.get_left().get_type();
        op.get_right().accept(*this);	
	Type type_op_right = op.get_right().get_type();
	if(type_op_left == type_op_right){
		Operator ope = op.op;
		//Toutes operations sont permises sur les int
		if(type_op_left == t_int)
			op.set_type(t_int);
		//Seulement = <> <= >= de permis avec les strings
		if(type_op_left == t_string && (ope == o_eq || ope == o_neq || ope == o_ge || ope == o_le))
			op.set_type(t_string);
		else
			error("Operation not permitted with srings");
	}
	else
		error("Operation of two different type objects");
}

void TypeChecker::visit(Sequence &seq){
	//Prend le type de la derniere expr
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
	//Prend le type de la derniere expr
	std::vector<Decl *> decls = let.get_decls();
      	Decl* last_decl = decls[(int)decls.size()-1];
	last_decl->accept(*this);
	Type type = last_decl->get_type();
	if(type != t_undef)
		let.set_type(type);
	else
		let.set_type(t_void);

}

void TypeChecker::visit(Identifier &id){
	optional<VarDecl &> decl = id.get_decl();
	//Prend le type de sa declaration
	if(decl)
		id.set_type(decl->get_type());
	else
		error("No Declarartion for this id");
}

void TypeChecker::visit(IfThenElse &ite){
	ite.get_condition().accept(*this);
	Type type_if = ite.get_condition().get_type();
	ite.get_then_part().accept(*this);
	Type type_then = ite.get_then_part().get_type();
	ite.get_else_part().accept(*this);
	Type type_else = ite.get_else_part().get_type();
	//Type du if doit etre int, ceux du then et du else doivent etre les meme
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
	//Type défini, on vérifie que le type déclaré est le meme que le type de la variable
	if(type){
		if(symbol_to_type(*type) == expr->get_type())
			decl.set_type(expr->get_type());
	}
	//Type non défini, on attribue le type de la variable
	else{
		Type type_expr = expr->get_type();
		if(type_expr == t_int || type_expr  == t_string)
			decl.set_type(type_expr);
		else
			error("Variable type must be integer or string, not void");		
	}
}

void TypeChecker::visit(FunDecl &fundecl){
	fundecl.accept(*this);
	optional<Expr &> expr = fundecl.get_expr();
	optional<Symbol> type = fundecl.type_name;
	//Si type est mentionné dans la declaration
	if(type){
		//Prend le type de son expr
		if(symbol_to_type(*type) == expr->get_type())
			fundecl.set_type(expr->get_type());
		else
			error("Function declaration can't have 2 different types");
	}
	//Sinon doit etre void
	else{
		Type type_expr = expr->get_type();
		if(type_expr == t_void)
			fundecl.set_type(t_void);
		else
			error("Expression is not void in the function declarartion");
	}
}

void TypeChecker::visit(FunCall &funcall){
	std::vector<Expr *> &args = funcall.get_args();
	optional<FunDecl &> decl = funcall.get_decl();
        if(decl){
		//Si noeud pas encore analysé recursion
		if(decl->get_type() == t_undef)
		//	Binder::visit(decl);
		std::vector<VarDecl *> params = decl->get_params();
		//Check de la taille et de la validité des affectations des parametres de la fonction
		if((int) args.size() == (int) params.size()){
			for(int i = 0; i < (int) args.size(); i++){
                		args[i]->accept(*this);
				if(!(args[i]->get_type() == params[i]->get_type()))
					error("Parameter and argument do not have the same type");
			}
			funcall.set_type(decl->get_type());
       		}
	}
	else
		error("No declaration for this function");
}

void TypeChecker::visit(WhileLoop &loop){
	loop.get_condition().accept(*this);
	Type type_cond = loop.get_condition().get_type();
	loop.get_body().accept(*this);
	Type type_body = loop.get_body().get_type();
	//Body void, condition int
	if(type_body == t_void && type_cond == t_int)
		loop.set_type(t_void);
	else
		error("While loops must have a voided body and an integer condition");
}

void TypeChecker::visit(ForLoop &loop){
	loop.get_variable().accept(*this);
	Type type_var = loop.get_variable().get_type();
	loop.get_high().accept(*this);
	Type type_high = loop.get_high().get_type();
	loop.get_body().accept(*this);
	Type type_body = loop.get_body().get_type();
	//indices et variable d'arret entiers, body void
	if(type_var == t_int && type_high == t_int && type_body == t_void)
		loop.set_type(t_void);
	else
		error("Index and bounds must be integers, and body is voided");
}

void TypeChecker::visit(Break &br){
	//Breaks sont void
	br.accept(*this);
	br.set_type(t_void);
}

void TypeChecker::visit(Assign &assign){
	Identifier * identifier = dynamic_cast<Identifier *>(&assign.get_lhs());
        if(identifier == nullptr)
                error(identifier->loc, "This is not an identifier");
        identifier->accept(*this);
        Type type_l = identifier->get_decl()->get_type();
        assign.get_rhs().accept(*this);
	Type type_r = assign.get_rhs().get_type();
	//Verification des types de l'assignement
	if(type_l == type_r)
		assign.set_type(t_void);
	else
		error("Declaration and assignement do not have the same type");
}


}
}


