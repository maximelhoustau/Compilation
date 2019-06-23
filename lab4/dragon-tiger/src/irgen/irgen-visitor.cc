#include <cstdlib>  // For exit
#include <iostream> // For std::cerr
#include "irgen.hh"

#include "llvm/Support/raw_ostream.h"

namespace {

// This function can be removed once the lab has been fully implemented.
[[noreturn]] void UNIMPLEMENTED() {
  std::cerr << "Error: unimplemented feature\n";
  exit(1);
}

} // namespace

namespace irgen {

llvm::Value *IRGenerator::visit(const IntegerLiteral &literal) {
  return Builder.getInt32(literal.value);
}

llvm::Value *IRGenerator::visit(const StringLiteral &literal) {
  return Builder.CreateGlobalStringPtr(literal.value.get());
}

llvm::Value *IRGenerator::visit(const Break &b) {
	Builder.CreateBr(loop_exit_bbs[&*b.get_loop()]);
	return(nullptr);;
}

llvm::Value *IRGenerator::visit(const BinaryOperator &op) {
  llvm::Value *l = op.get_left().accept(*this);
  llvm::Value *r = op.get_right().accept(*this);

  if (op.get_left().get_type() == t_string) {
    auto const strcmp = Mod->getOrInsertFunction(
        "__strcmp", Builder.getInt32Ty(), Builder.getInt8PtrTy(),
        Builder.getInt8PtrTy(), nullptr);
    l = Builder.CreateCall(strcmp, {l, r});
    r = Builder.getInt32(0);
  }

  switch(op.op) {
    case o_plus: return Builder.CreateBinOp(llvm::Instruction::Add, l, r);
    case o_minus: return Builder.CreateBinOp(llvm::Instruction::Sub, l, r);
    case o_times: return Builder.CreateBinOp(llvm::Instruction::Mul, l, r);
    case o_divide: return Builder.CreateBinOp(llvm::Instruction::SDiv, l, r);
    default: break;
  }

  // Comparisons return an i1 result which needs to be
  // casted to i32, as Tiger might use that as an integer.
  llvm::Value *cmp;

  switch(op.op) {
    case o_eq: cmp = Builder.CreateICmpEQ(l, r); break;
    case o_neq: cmp = Builder.CreateICmpNE(l, r); break;
    case o_gt: cmp = Builder.CreateICmpSGT(l, r); break;
    case o_lt: cmp = Builder.CreateICmpSLT(l, r); break;
    case o_ge: cmp = Builder.CreateICmpSGE(l, r); break;
    case o_le: cmp = Builder.CreateICmpSLE(l, r); break;
    default: assert(false); __builtin_unreachable();
  }

  return Builder.CreateIntCast(cmp, Builder.getInt32Ty(), true);
}

llvm::Value *IRGenerator::visit(const Sequence &seq) {
  llvm::Value *result = nullptr;
  for (auto expr : seq.get_exprs())
    result = expr->accept(*this);
  // An empty sequence should return () but the result
  // will never be used anyway, so nullptr is fine.
  return result;
}

llvm::Value *IRGenerator::visit(const Let &let) {
  for (auto decl : let.get_decls())
    decl->accept(*this);

  return let.get_sequence().accept(*this);
}

llvm::Value *IRGenerator::visit(const Identifier &id) {
	llvm::Value * ptr = address_of(id);
	return Builder.CreateLoad(ptr);

}

llvm::Value *IRGenerator::visit(const IfThenElse &ite) {
	llvm::Value * ptr;
	Type if_type = ite.get_type();
	//Alloue la mémoire pour la pile
	if( if_type != t_void)
		ptr = alloca_in_entry(llvm_type(if_type), "Result");
	//On construit les 3 blocs d'entrée
	llvm::BasicBlock * block_then = llvm::BasicBlock::Create(Context, "Then", current_function);
	llvm::BasicBlock * block_else = llvm::BasicBlock::Create(Context, "Else", current_function);
	llvm::BasicBlock * block_end = llvm::BasicBlock::Create(Context, "End", current_function);
	Builder.CreateCondBr(Builder.CreateIsNotNull(ite.get_condition().accept(*this)), block_then, block_else);
	Builder.SetInsertPoint(block_then);
	
	llvm::Value * then_part = ite.get_then_part().accept(*this);
	
	if(if_type != t_void)
		Builder.CreateStore(then_part, ptr);
	
	Builder.CreateBr(block_end);
	Builder.SetInsertPoint(block_else);
	llvm::Value * else_part = ite.get_else_part().accept(*this);
	
	if(if_type != t_void)
		Builder.CreateStore(else_part, ptr);
	
	Builder.CreateBr(block_end);
	Builder.SetInsertPoint(block_end);
	
	if(if_type != t_void)
		return(Builder.CreateLoad(ptr));
	else
		return(nullptr);

}

llvm::Value *IRGenerator::visit(const VarDecl &decl) {
	llvm::Type * type = llvm_type(decl.get_type());
	llvm::Value * ptr = alloca_in_entry(type, decl.name);
	const optional<const Expr&> expr = decl.get_expr();
	llvm::Value * value = nullptr;	
	if(expr){
		value = expr->accept(*this);
		Builder.CreateStore(value, ptr);
	}
	std::pair<const VarDecl*, llvm::Value*> allocation (&decl, ptr);
	allocations.insert(allocation);
	return(ptr);

}

llvm::Value *IRGenerator::visit(const FunDecl &decl) {
  std::vector<llvm::Type *> param_types;

  for (auto param_decl : decl.get_params()) {
    param_types.push_back(llvm_type(param_decl->get_type()));
  }

  llvm::Type *return_type = llvm_type(decl.get_type());

  llvm::FunctionType *ft =
      llvm::FunctionType::get(return_type, param_types, false);

  llvm::Function::Create(ft,
                         decl.is_external ? llvm::Function::ExternalLinkage
                                          : llvm::Function::InternalLinkage,
                         decl.get_external_name().get(), Mod.get());

  if (decl.get_expr())
    pending_func_bodies.push_front(&decl);

  return nullptr;
}

llvm::Value *IRGenerator::visit(const FunCall &call) {
  // Look up the name in the global module table.
  const FunDecl &decl = call.get_decl().get();
  llvm::Function *callee =
      Mod->getFunction(decl.get_external_name().get());

  if (!callee) {
    // This should only happen for primitives whose Decl is out of the AST
    // and has not yet been handled
    assert(!decl.get_expr());
    decl.accept(*this);
    callee = Mod->getFunction(decl.get_external_name().get());
  }

  std::vector<llvm::Value *> args_values;
  for (auto expr : call.get_args()) {
    args_values.push_back(expr->accept(*this));
  }

  if (decl.get_type() == t_void) {
    Builder.CreateCall(callee, args_values);
    return nullptr;
  }
  return Builder.CreateCall(callee, args_values, "call");
}

llvm::Value *IRGenerator::visit(const WhileLoop &loop) {
	llvm::BasicBlock * block_cond = llvm::BasicBlock::Create(Context, "Loop condition", current_function);
	llvm::BasicBlock * block_body = llvm::BasicBlock::Create(Context, "Loop body", current_function);
	llvm::BasicBlock * block_end = llvm::BasicBlock::Create(Context, "Loop end", current_function);
	
	loop_exit_bbs[&loop] = block_end;
	
	Builder.CreateBr(block_cond);
	Builder.SetInsertPoint(block_cond);
	
	Builder.CreateCondBr(Builder.CreateIsNotNull(loop.get_condition().accept(*this)), block_body, block_end);
	Builder.SetInsertPoint(block_body);
	
	loop.get_body().accept(*this);
	
	Builder.CreateBr(block_cond);
	Builder.SetInsertPoint(block_end);
	return(nullptr);
}

llvm::Value *IRGenerator::visit(const ForLoop &loop) {
  llvm::BasicBlock *const test_block =
      llvm::BasicBlock::Create(Context, "loop_test", current_function);
  llvm::BasicBlock *const body_block =
      llvm::BasicBlock::Create(Context, "loop_body", current_function);
  llvm::BasicBlock *const end_block =
      llvm::BasicBlock::Create(Context, "loop_end", current_function);
  loop_exit_bbs[&loop] = end_block;
  llvm::Value *const index = loop.get_variable().accept(*this);
  llvm::Value *const high = loop.get_high().accept(*this);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(test_block);
  Builder.CreateCondBr(Builder.CreateICmpSLE(Builder.CreateLoad(index), high),
                       body_block, end_block);

  Builder.SetInsertPoint(body_block);
  loop.get_body().accept(*this);
  Builder.CreateStore(
      Builder.CreateAdd(Builder.CreateLoad(index), Builder.getInt32(1)), index);
  Builder.CreateBr(test_block);

  Builder.SetInsertPoint(end_block);
  return nullptr;
}

llvm::Value *IRGenerator::visit(const Assign &assign) {
	llvm::Value * ptr = allocations[&*assign.get_lhs().get_decl()];
	llvm::Value * value = assign.get_rhs().accept(*this);
	return(Builder.CreateStore(value, ptr));
}

} // namespace irgen
