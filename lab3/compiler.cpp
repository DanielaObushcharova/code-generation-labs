#include "parser.h"
#include <llvm/IR/IRBuilder.h>
#include <map>
#include <set>

std::set<std::string> get_vars(Node *tree) {
  if (tree->rule == TERM && tree->token->type == IDENT) {
    return {tree->token->identAttr};
  }
  std::set<std::string> res;
  for (auto child : tree->children) {
    auto cur = get_vars(child);
    res.insert(cur.begin(), cur.end());
  }
  return res;
}

class IRGenerator {
private:
  llvm::LLVMContext &ctx;
  llvm::IRBuilder<> &builder;
  llvm::Function *func;
  std::map<std::string, llvm::AllocaInst *> &vars;
  llvm::Value *generateRval(Node *tree);

public:
  IRGenerator(llvm::LLVMContext &_ctx, llvm::IRBuilder<> &builder,
              llvm::Function *_func,
              std::map<std::string, llvm::AllocaInst *> &vars);

  llvm::BasicBlock *generate(Node *tree, llvm::BasicBlock *parent);
};

IRGenerator::IRGenerator(llvm::LLVMContext &_ctx, llvm::IRBuilder<> &_builder,
                         llvm::Function *_func,
                         std::map<std::string, llvm::AllocaInst *> &_vars)
    : ctx(_ctx), builder(_builder), func(_func), vars(_vars) {}

llvm::Value *IRGenerator::generateRval(Node *tree) {
  switch (tree->rule) {
  case TERM: {
    switch (tree->token->type) {
    case NUMBER:
      return llvm::ConstantInt::get(builder.getInt32Ty(),
                                    tree->token->numberAttr);
    case IDENT: {
      auto var = vars[tree->token->identAttr];
      return builder.CreateLoad(var->getAllocatedType(), var);
    }
    }
    break;
  }
  case RVAL: {
    llvm::Value *left = generateRval(tree->children[0]);
    llvm::Value *right = generateRval(tree->children[1]->children[1]);
    switch (tree->children[1]->children[0]->token->opAttr) {
    case '+':
      return builder.CreateAdd(left, right);
    case '-':
      return builder.CreateSub(left, right);
    case '*':
      return builder.CreateMul(left, right);
    }
  }
  }
  return nullptr;
}

llvm::BasicBlock *IRGenerator::generate(Node *tree, llvm::BasicBlock *parent) {
  switch (tree->rule) {
  case S: {
    llvm::BasicBlock *prev = parent;
    for (auto child : tree->children) {
      if (!prev) {
        return nullptr;
      }
      prev = generate(child, prev);
    }
    return prev;
  }
  case BB: {
    llvm::BasicBlock *bb = llvm::BasicBlock::Create(ctx, "BB", func);
    builder.SetInsertPoint(parent);
    builder.CreateBr(bb);
    builder.SetInsertPoint(bb);
    for (auto child : tree->children) {
      switch (child->rule) {
      case ASSIGN_RULE: {
        std::string var_name = child->children[0]->token->identAttr;
        builder.CreateStore(generateRval(child->children[1]),
                            this->vars[var_name]);
        break;
      }
      case RETURN_RULE: {
        builder.CreateRet(generateRval(child->children[0]));
        return nullptr;
      }
      }
    }
    return bb;
  }
  case IF_RULE: {
    llvm::BasicBlock *header = llvm::BasicBlock::Create(ctx, "if_header", func);
    builder.SetInsertPoint(parent);
    builder.CreateBr(header);
    llvm::BasicBlock *branch_true = llvm::BasicBlock::Create(ctx, "true", func);
    llvm::BasicBlock *branch_false =
        llvm::BasicBlock::Create(ctx, "false", func);
    llvm::BasicBlock *merge = llvm::BasicBlock::Create(ctx, "if_merge", func);
    builder.SetInsertPoint(header);
    llvm::Value *condVal = generateRval(tree->children[0]);
    llvm::Value *cond = builder.CreateICmpSGT(
        condVal, llvm::ConstantInt::get(builder.getInt32Ty(), 0));
    builder.CreateCondBr(cond, branch_true, branch_false);
    auto bbt = generate(tree->children[1], branch_true);
    if (bbt) {
      builder.SetInsertPoint(bbt);
      builder.CreateBr(merge);
    }
    auto bbf = generate(tree->children[2], branch_false);
    if (bbf) {
      builder.SetInsertPoint(bbf);
      builder.CreateBr(merge);
    }
    return merge;
  }
  case WHILE_RULE: {
    llvm::BasicBlock *header =
        llvm::BasicBlock::Create(ctx, "while_header", func);
    builder.SetInsertPoint(parent);
    builder.CreateBr(header);
    llvm::BasicBlock *loop = llvm::BasicBlock::Create(ctx, "loop", func);
    auto content = generate(tree->children[1], loop);
    if (content) {
        builder.SetInsertPoint(content);
        builder.CreateBr(header);
    }
    llvm::BasicBlock *out = llvm::BasicBlock::Create(ctx, "while_out", func);
    builder.SetInsertPoint(header);
    llvm::Value *condVal = generateRval(tree->children[0]);
    llvm::Value *cond = builder.CreateICmpSGT(
        condVal, llvm::ConstantInt::get(builder.getInt32Ty(), 0));
    builder.CreateCondBr(cond, loop, out);
    return out;
  }
  }
  return nullptr;
}

int main() {
  /* freopen("./lab3/input.txt", "r", stdin); */
  std::vector<Token> tokens;
#ifdef DEBUG
  std::cout << "TOKENS:" << std::endl;
#endif
  while (true) {
    Token token = yylex();
#ifdef DEBUG
    std::cout << token << std::endl;
#endif
    tokens.push_back(token);
    if (token.type == EOF_TOKEN) {
      break;
    }
  }
  Parser parser(tokens);
  Node *tree;
  try {
    tree = parser.parse();
#ifdef DEBUG
    std::cout << "TREE:" << std::endl;
    tree->print();
#endif
  } catch (SyntaxError err) {
    std::cout << err.what() << std::endl;
    return 1;
  }
  auto vars = get_vars(tree);
#ifdef DEBUG
  std::cout << "VARS:" << std::endl;
  for (auto var : vars) {
    std::cout << var << std::endl;
  }
#endif
  llvm::LLVMContext ctx;
  llvm::IRBuilder<> builder(ctx);
  llvm::Module *mod = new llvm::Module("top", ctx);
  llvm::FunctionType *funcType =
      llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *mainFunc = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, "main", mod);
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(ctx, "alloc", mainFunc);
  builder.SetInsertPoint(entry);
  std::map<std::string, llvm::AllocaInst *> varsMap;
  for (auto var : vars) {
    varsMap[var] =
        builder.CreateAlloca(llvm::Type::getInt32Ty(ctx), nullptr, var);
  }
  auto generator = new IRGenerator(ctx, builder, mainFunc, varsMap);
  auto program = generator->generate(tree, entry);
  builder.SetInsertPoint(program);
  if (program) {
      llvm::BasicBlock *ret = llvm::BasicBlock::Create(ctx, "return", mainFunc);
      builder.CreateBr(ret);
      builder.SetInsertPoint(ret);
      llvm::Value *retVal = llvm::ConstantInt::get(builder.getInt32Ty(), 0);
      builder.CreateRet(retVal);
  }
  mod->print(llvm::errs(), nullptr);
}
