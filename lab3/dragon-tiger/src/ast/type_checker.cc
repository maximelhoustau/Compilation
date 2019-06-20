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
	//std::cerr << "Visit Integer" << "\n";
	id.set_type(t_int);
}

void TypeChecker::visit(StringLiteral &str){
	//std::cerr << "Visit String" << "\n";
	//String
	str.set_type(t_string);
}

void TypeChecker::visit(BinaryOperator &op){
	//std::cerr << "Visit BinOp" << "\n";
	op.get_left().accept(*this);
	Type type_op_left = op.get_left().get_type();
	op.get_right().accept(*this);	
	Type type_op_right = op.get_right().get_type();
       	if(type_op_left == type_op_right){
		Operator ope = op.op;
		//Seulement comparaison de permis avec les strings
		if(type_op_left == t_string && (ope == o_plus || ope == o_minus || ope == o_divide || ope == o_times))
			error("Operation not permitted with strings");
		if(type_op_left == t_void || type_op_left == t_undef)
			error("Operation not permitted between void objects");
		op.set_type(t_int);
	}
	else
		error("Operation of two different type objects");
}

void TypeChecker::visit(Sequence &seq){
	//std::cerr << "Visit Sequence" << "\n";
	//Prend le type de la derniere expr
	std::vector<Expr *> exprs = seq.get_exprs();
	if(exprs.empty())
		seq.set_type(t_void);
	else {
		for(int i = 0; i < (int) exprs.size(); i++){
                	exprs[i]->accept(*this);
        	}
		Expr* last_expr = exprs[(int)exprs.size()-1];
		Type type = last_expr->get_type();
		seq.set_type(type);
	}
}

void TypeChecker::visit(Let &let){
	//std::cerr << "Visit Let" << "\n";
	//Prend le type de la derniere expr
	std::vector<Decl *> decls = let.get_decls();
	Sequence &seq = let.get_sequence();
	for(int i = 0; i < (int) decls.size(); i++){
              decls[i]->accept(*this); 
        }
        seq.accept(*this);
	Type type = seq.get_type();
	if(type != t_undef)
		let.set_type(type);
	else
		let.set_type(t_void);

}

void TypeChecker::visit(Identifier &id){
	//std::cerr << "Visit Identifier" << "\n";
	optional<VarDecl &> decl = id.get_decl();
	//Prend le type de sa declaration
	if(decl){
		std::cerr << decl->get_type() << "\n";
		id.set_type(decl->get_type());
	}
	else
		error("No Declaration for this id");
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
		return(t_void);
	else
		error("Type undefined");	
}

void TypeChecker::visit(VarDecl &decl){
	//std::cerr << "Visit VarDecl" << "\n";
	optional<Expr &> expr = decl.get_expr();
	optional<Symbol> type = decl.type_name;
	if(expr)	
		expr->accept(*this);
	else
		error("Expression invalid");
	
	//Type défini, on vérifie que le type déclaré est le meme que le type de la variable
	if(type){
		if(symbol_to_type(*type) == t_void)
			error("Variable cannot be void");
		else if(symbol_to_type(*type) == expr->get_type())
			decl.set_type(expr->get_type());
		else
			error("Declaration and expression must have the same type");
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
	//std::cerr << "Visit FunDecl" << "\n";
	optional<Expr &> expr = fundecl.get_expr();
	optional<Symbol> type = fundecl.type_name;
	if(expr){
		expr->accept(*this);
		//Si type est mentionné dans la declaration
          	if(type){
                  //Prend le type de son expr
                	if(symbol_to_type(*type) == t_void)
                        	error("Variable cannot be void");
                	 if(symbol_to_type(*type) == expr->get_type())
                        	fundecl.set_type(expr->get_type());
                  	else
                          	error("Function declaration can't have 2 different types");
          	}
          	//Sinon doit etre void
          	else{
                	Type type_expr = expr->get_type();
                  	if(type_expr == t_void)
				error("Expression is not void in the function declaration");
			fundecl.set_type(type_expr);
          	}
	}

	else{
		if(symbol_to_type(*type) == t_int)
			fundecl.set_type(t_int);
		if(symbol_to_type(*type) == t_string)
			fundecl.set_type(t_string);
		//Fonction primitive
		if(fundecl.is_external){
			if(type)
				fundecl.set_type(symbol_to_type(*type));
		}
		else
			error("Expression incorrect");
	}
}

void TypeChecker::visit(FunCall &funcall){
	FunDecl *decl = &*funcall.get_decl();
    	int stack_size = fundecl_stack.size();
    	for (int i = 0 ; i < stack_size ; i++) {
        	FunDecl *fundecl_tmp = fundecl_stack[i];
        	if (decl == fundecl_tmp) {
            		funcall.set_type(decl->get_type());
            	return;
        	}
    	}
	//Si noeud pas encore analysé recursion
    	if (decl->get_type() == t_undef){
       	 	decl->accept(*this);
    	}

	//std::cerr << "Visit FunCall" << "\n";
	std::vector<Expr *> &args = funcall.get_args();
	std::vector<VarDecl *> &params = decl->get_params();
	//Check de la taille et de la validité des affectations des parametres de la fonction
	if((int) args.size() != (int) params.size())
		error("Wrong number of arguments");

	for(int i = 0; i < (int) args.size(); i++){
		args[i]->accept(*this);
		if(args[i]->get_type() != params[i]->get_type())
			error("Parameter and argument do not have the same type");
	}
	funcall.set_type(decl->get_type());
	
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
	br.set_type(t_void);
}

void TypeChecker::visit(Assign &assign){
	std::cerr << "Visit Assign" << "\n";
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


