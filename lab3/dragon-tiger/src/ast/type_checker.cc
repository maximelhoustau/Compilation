#include "type_checker.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"
#include <string>

using utils::error;
using utils::non_fatal_error;


namespace ast{
namespace type_checker{

void TypeChecker::analyze_program(FunDecl &main) {
	main.accept(*this);
}
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
		//std::cerr << decl->get_type() << "\n";
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

void TypeChecker::visit(FunDecl &decl){	
    fundecl_stack.push_back(&decl);
    /* if this function has been visited already by a funcall */
    if (decl.get_type() != t_undef) return;
    /* visit parameters */
    std::vector<VarDecl *> & vars = decl.get_params();
    int n = vars.size();
    for(int i = 0 ; i < n ; i++) {
        vars[i]->accept(*this);
    }
    /* visit expression and check if it match the explicit
     * paramter if any */
    optional<Symbol> type_name = decl.type_name;
    optional<Expr &> expr = decl.get_expr();
    if(expr && type_name) {
        decl.set_type(symbol_to_type(*type_name));
        expr->accept(*this);
        if(symbol_to_type(*type_name) == t_void)
            error(decl.loc, "explicit void type name is disallowed in non-primitive function declaration");
        if (symbol_to_type(*type_name) != expr->get_type())
            error(decl.loc, "mismatch type declaration");
    } else if (expr && !type_name) {
        decl.set_type(t_void);
        expr->accept(*this);
        if (expr->get_type() != t_void)
            error(decl.loc, "function with no explicit type must be void");
    } else {
        if (decl.is_external) {
            /* accept the type of a non primitive declaration without
             * visiting his expression (which is nullptr) */
            decl.set_type(symbol_to_type(*type_name));
        } else if (type_name) {
            if (symbol_to_type(*type_name) != t_void)
                error(decl.loc, "function type mismatch");
            decl.set_type(t_void);
        }
    }
	fundecl_stack.pop_back();

}
	

void TypeChecker::visit(FunCall &funcall){
	//std::cerr <<  "visit FunCall\n";
    FunDecl *funDecl = &*funcall.get_decl();
    int n_recursive = fundecl_stack.size();
    for (int i = 0 ; i < n_recursive ; i++) {
        FunDecl *current_funDecl = fundecl_stack[i];
        if (funDecl == current_funDecl) {
            /* This is a recusive call */
            funcall.set_type(funDecl->get_type());
            return;
        }
    }
    if (funDecl->get_type() == t_undef){
        /* function not declared yet */
        funDecl->accept(*this);
    }
    std::vector<VarDecl *> decl_params = funDecl->get_params();
    std::vector<Expr *> call_exprs = funcall.get_args();
    /* check number of parameters and their types */
    int n_call = call_exprs.size();
    int n_decl = decl_params.size();
    if (n_call != n_decl)
        error(funcall.loc, "The number of parameters mismatch the previous declaration");
    for (int i = 0 ; i < n_call ; i++){
        call_exprs[i]->accept(*this);
        if (call_exprs[i]->get_type() != decl_params[i]->get_type())
            error(funcall.loc, "parameters type mismatch from the declaration");
    }
    funcall.set_type(funDecl->get_type());

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
        assign.get_lhs().accept(*this);
        assign.get_rhs().accept(*this);
	Type type_r = assign.get_rhs().get_type();
	Type type_l = assign.get_lhs().get_type();
	//Verification des types de l'assignement
	if(type_l == type_r)
		assign.set_type(t_void);
	else
		error("Declaration and assignement do not have the same type");
}


}
}


