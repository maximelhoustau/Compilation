#ifndef TYPE_CHECKER
#define TYPE_CHECKER

#include "nodes.hh"

namespace ast {
namespace type_checker {

class TypeChecker : public ASTVisitor {
	std::vector<FunDecl *> fundecl_stack;	
public:
	TypeChecker() {};
	Type symbol_to_type(Symbol type_s);
  	virtual void visit(IntegerLiteral &);
  	virtual void visit(StringLiteral &);
  	virtual void visit(BinaryOperator &);
  	virtual void visit(Sequence &);
  	virtual void visit(Let &);
  	virtual void visit(Identifier &);
  	virtual void visit(IfThenElse &);
  	virtual void visit(VarDecl &);
  	virtual void visit(FunDecl &);
  	virtual void visit(FunCall &);
	virtual void visit(WhileLoop &);
      	virtual void visit(ForLoop &);
      	virtual void visit(Break &);
	virtual void visit(Assign &);

};
}
}
#endif
