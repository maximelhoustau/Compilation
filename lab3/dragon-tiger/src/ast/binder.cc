#include <sstream>

#include "binder.hh"
#include "../utils/errors.hh"
#include "../utils/nolocation.hh"

using utils::error;
using utils::non_fatal_error;

namespace ast {
namespace binder {

/* Returns the current scope */
scope_t &Binder::current_scope() { return scopes.back(); }

/* Pushes a new scope on the stack */
void Binder::push_scope() { scopes.push_back(scope_t()); }

/* Pops the current scope from the stack */
void Binder::pop_scope() { scopes.pop_back(); }

/* Enter a declaration in the current scope. Raises an error if the declared name
 * is already defined */
void Binder::enter(Decl &decl) {
  scope_t &scope = current_scope();
  auto previous = scope.find(decl.name);
  if (previous != scope.end()) {
    non_fatal_error(decl.loc,
                    decl.name.get() + " is already defined in this scope");
    error(previous->second->loc, "previous declaration was here");
  }
  scope[decl.name] = &decl;
}

/* Finds the declaration for a given name. The scope stack is traversed
 * front to back starting from the current scope. The first matching
 * declaration is returned. Raises an error, if no declaration matches. */
Decl &Binder::find(const location loc, const Symbol &name) {
  for (auto scope = scopes.crbegin(); scope != scopes.crend(); scope++) {
    auto decl_entry = scope->find(name);
    if (decl_entry != scope->cend()) {
      return *decl_entry->second;
    }
  }
  error(loc, name.get() + " cannot be found in this scope");
}

Binder::Binder() : scopes() {
  /* Create the top-level scope */
  push_scope();

  /* Populate the top-level scope with all the primitive declarations */
  enter_primitive("print_err", "void", {"string"});
  enter_primitive("print", "void", {"string"});
  enter_primitive("print_int", "void", {"int"});
  enter_primitive("flush", "void", {});
  enter_primitive("getchar", "string", {});
  enter_primitive("ord", "int", {"string"});
  enter_primitive("chr", "string", {"int"});
  enter_primitive("size", "int", {"string"});
  enter_primitive("substring", "string", {"string", "int", "int"});
  enter_primitive("concat", "string", {"string", "string"});
  enter_primitive("strcmp", "int", {"string", "string"});
  enter_primitive("streq", "int", {"string", "string"});
  enter_primitive("not", "int", {"int"});
  enter_primitive("exit", "void", {"int"});
}

/* Declares a new primitive into the current scope*/
void Binder::enter_primitive(
    const std::string &name, const std::string &type_name,
    const std::vector<std::string> &argument_typenames) {
  std::vector<VarDecl *> args;
  int counter = 0;
  for (const std::string &tn : argument_typenames) {
    std::ostringstream argname;
    argname << "a_" << counter++;
    args.push_back(
        new VarDecl(utils::nl, Symbol(argname.str()), nullptr, Symbol(tn)));
  }

  FunDecl *fd = new FunDecl(utils::nl, Symbol(name), std::move(args), nullptr,
                            Symbol(type_name), true);
  fd->set_external_name(Symbol("__" + name));
  enter(*fd);
}

/* Sets the parent of a function declaration and computes and sets
 * its unique external name */
void Binder::set_parent_and_external_name(FunDecl &decl) {
  auto parent = functions.empty() ? nullptr : functions.back();
  Symbol external_name;
  if (parent) {
    decl.set_parent(parent);
    external_name = parent->get_external_name().get() + '.' + decl.name.get();
  } else
    external_name = decl.name;
  while (external_names.find(external_name) != external_names.end())
    external_name = Symbol(external_name.get() + '_');
  external_names.insert(external_name);
  decl.set_external_name(external_name);
}

/* Binds a whole program. This method wraps the program inside a top-level main
 * function.  Then, it visits the programs with the Binder visitor; binding
 * each identifier to its declaration and computing depths.*/
FunDecl *Binder::analyze_program(Expr &root) {
  std::vector<VarDecl *> main_params;
  Sequence *const main_body = new Sequence(
      utils::nl,
      std::vector<Expr *>({&root, new IntegerLiteral(utils::nl, 0)}));
  FunDecl *const main = new FunDecl(utils::nl, Symbol("main"), main_params,
                                    main_body, Symbol("int"), true);
  main->accept(*this);
  return main;
}

void Binder::visit(IntegerLiteral &literal) {
}

void Binder::visit(StringLiteral &literal) {
}

void Binder::visit(BinaryOperator &op) {
	op.get_left().accept(*this);
	op.get_right().accept(*this);
}

void Binder::visit(Sequence &seq) {
	std::vector<Expr *> exprs = seq.get_exprs();
	for(int i = 0; i < (int) exprs.size(); i++){
		exprs[i]->accept(*this);
	}
}

void Binder::visit(Let &let) {
	push_scope();
	//Breaks not allowed
	parentloops.push_back(nullptr);

	std::vector<Decl*> decls = let.get_decls();
	for(int i = 0; i < (int) decls.size(); i++){
		FunDecl* fundecl = dynamic_cast<FunDecl *>(decls[i]);
		if(fundecl == nullptr)
			decls[i]->accept(*this);
		else{
			int j = i;
			while(fundecl != nullptr){
				enter(*fundecl);
				j++;
				if( j >= (int) decls.size())
					break;
				fundecl = dynamic_cast<FunDecl *>(decls[j]);
			}
			for(int m = i; m < j; m++){
				decls[m]->accept(*this);
			}
			i = j-1;
		}
	}
	let.get_sequence().accept(*this);
	//Breaks back
	parentloops.pop_back();
	pop_scope();
}

void Binder::visit(Identifier &id) {
	Decl& declaration = find(id.loc, id.name);
	VarDecl* decl = dynamic_cast<VarDecl*>( &declaration );
	if (decl == nullptr) { utils::error(id.loc,"No Var declaration for this id");}
	id.set_depth(depth);
	id.set_decl(decl);
	if(decl->get_depth() != depth)
		decl->set_escapes();

}

void Binder::visit(IfThenElse &ite) {
	ite.get_condition().accept(*this);
	ite.get_then_part().accept(*this);
	ite.get_else_part().accept(*this);
}

void Binder::visit(VarDecl &decl) {
  optional<Expr&> expr = decl.get_expr();
  enter(decl);
  if(expr)
	  expr->accept(*this);

}

void Binder::visit(FunDecl &decl) {
  set_parent_and_external_name(decl);
  functions.push_back(&decl);
  /* ... put your code here ... */
	push_scope();
	depth++;
	std::vector<VarDecl *> &params = decl.get_params();
	for(int i = 0; i < (int) params.size(); i++){
		params[i]->accept(*this);
	}
	optional<Expr&> expr = decl.get_expr();
	if(expr)
		expr->accept(*this);
	depth--;
	functions.pop_back();
	pop_scope();
}

void Binder::visit(FunCall &call) {
	std::vector<Expr *> &args = call.get_args();
	for(int i = 0; i < (int) args.size(); i++){
		args[i]->accept(*this);
	}
	Decl & decl = find(call.loc, call.func_name);
	FunDecl *fundecl = dynamic_cast<FunDecl *>(&decl);
	if(fundecl == nullptr)
		error(call.loc, "Dunction Declaration not found");	
}


void Binder::visit(WhileLoop &loop) {
	loop.get_condition().accept(*this);
	loop.get_body().accept(*this);
	parentloops.push_back(&loop);
	parentloops.pop_back();
}

void Binder::visit(ForLoop &loop) {
	push_scope();
	// Put the loop index into indices vector
	loop.get_variable().accept(*this);
	indices.push_back(&loop.get_variable());
	loop.get_high().accept(*this);
	// Add new loop in parentloops vector
	parentloops.push_back(&loop);
	loop.get_body().accept(*this);
	parentloops.pop_back();
	pop_scope();
}

void Binder::visit(Break &b) {
	b.set_loop(parentloops.back());
	if(parentloops.empty() || parentloops.back())
		error("Break is at a wrong place");
}

void Binder::visit(Assign &assign) {
	Identifier * identifier = dynamic_cast<Identifier *>(&assign.get_lhs());
	//Check identifiers
	if(identifier == nullptr)
		error(identifier->loc, "This is not an identifier");
	identifier->accept(*this);
	optional<VarDecl &> declaration = identifier->get_decl();
	//Checking declaration with indices
	for(int i = 0; i < (int) indices.size(); i++){
		if(&*declaration == indices[i])
			error(identifier->loc, "Impossible to assign loop variable");
		}
	assign.get_rhs().accept(*this);
}


} // namespace binder
} // namespace ast
