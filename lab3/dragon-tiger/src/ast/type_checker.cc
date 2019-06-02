#include "type_checker.hh"

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

void TypeChecker::visit(BinaryOperator &){

}

void TypeChecker::visit(Sequence &){

}

void TypeChecker::visit(Let &){

}

void TypeChecker::visit(Identifier &){

}

void TypeChecker::visit(IfThenElse &){

}

void TypeChecker::visit(VarDecl &){

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


