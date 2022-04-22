#include <llvm/IR/IRBuilder.h>

int main() {
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder(ctx);
    llvm::Module *mod = new llvm::Module("top", ctx);
    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", mod);
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "entrypoint", mainFunc);
    builder.SetInsertPoint(entry);
    llvm::Value *first = llvm::ConstantInt::get(builder.getInt32Ty(), 353);
    llvm::Value *second = llvm::ConstantInt::get(builder.getInt32Ty(), 48);
    llvm::Value *retVal = builder.CreateAdd(first, second);
    builder.CreateRet(retVal);
    mod->print(llvm::errs(), nullptr);
}
