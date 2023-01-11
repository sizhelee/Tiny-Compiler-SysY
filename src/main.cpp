#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <map>
#include <variant>

#include "AST.h"
#include "koopa.h"
#include "util.h"

#define DEBUGMODE 1

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin, *yyout;
extern int yyparse(unique_ptr<BaseAST> &ast);
int debugmode = 1;
int expNumCnt = 0, symTabCnt = 0, ifNumCnt = 0, allsymTabCnt = 0, whileNumCnt = 0;
int brctNumCnt = 0;
string now_while_end = "", now_while_entry = "";

map<string, pair<int, int>> symbol_table;
map<string, pair<int, int>> *current_table;
map<map<string, pair<int, int>>*,map<string, pair<int, int>>*> total_table; // 记录每个符号表的父亲，虽然这个结构很丑

map<string, string> func_table;
map<string, pair<int, int>> glob_table;

string str1; // riscv str
map<koopa_raw_value_t, int> stackForInsts;
map<koopa_raw_function_t, int> mapFuncToSp, mapFuncToRa;
koopa_raw_function_t curFunc;

// 函数声明略
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary);
void Visit(const koopa_raw_store_t &rawStore);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_func_arg_ref_t &funcArgRef);
void Visit(const koopa_raw_global_alloc_t &myGlobalAlloc);

int retValue(const koopa_raw_value_t &rawValue);
int retValue(const koopa_raw_integer_t &rawInterger);

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  # if DEBUGMODE
    cout << "******** visit program ********" << endl;
  # endif
  // myPrint("  .text ");
  // cout << "  .text " << endl;
  // str1 += "  .text\n";

  for (size_t i = 0; i < program.values.len; ++i) {
      // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
      // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
      assert(program.values.kind == KOOPA_RSIK_VALUE);
      // 获取当前函数
      // koopa_raw_function_t value = (koopa_raw_function_t) program.values.buffer[i];
      // 进一步处理当前函数
      // myPrint(func->name);

      // cout << value->name << endl;
      // str1 += value->name;
      // str1 += "\n";
  }

  // myPrint("  .global ", false);
  // cout << "  .global ";
  // str1 += "  .global ";

  for (size_t i = 0; i < program.funcs.len; ++i) {
      // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
      // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
      assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
      // 获取当前函数
      // koopa_raw_function_t func = (koopa_raw_function_t) program.funcs.buffer[i];

      // 进一步处理当前函数
      // cout << "*******" << func->name << "*******" << endl;
      // for(int i = 1; i < strlen(func->name); i++)
      // {
      //   char c = func->name[i]; 
      //   cout << c;
      //   // fprintf(yyout, "%c", c);
      //   str1 += c;
      // }
      // fprintf(yyout, "%s",func->name);
      // fprintf(yyout, "\n");
      // cout << endl;
      // str1 += "\n";
  }

  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);

  fprintf(yyout, "%s", str1.c_str());
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  # if DEBUGMODE
    cout << "******** visit slice, kind: " << slice.kind << " ********" << endl;
  # endif
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION: // 2
        // 访问函数
        if(reinterpret_cast<koopa_raw_function_t>(ptr)->bbs.len != 0)
          Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK: // 3
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE: // 4
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  # if DEBUGMODE
    cout << "******** visit function ********" << endl;
  # endif
  // 执行一些其他的必要操作
  // ...
  str1 += "  .text \n";
  str1 += "  .global ";
  string funcName = func->name;
  str1 += funcName.erase(0, 1)+"\n";
  str1 += funcName;
  str1 += ":\n";

  Visit(func->params);

  int spForEachInst = 0;
  int spForRa = 0, spForFuncParams = 0;
  int maxLenForArgs = 0;
  for (int i = 0; i < func->bbs.len; i++)
  {
    auto ptr = func->bbs.buffer[i];
    koopa_raw_basic_block_t funcbb = reinterpret_cast<koopa_raw_basic_block_t>(ptr);
    for (int j = 0; j < funcbb->insts.len; j++)
    {
      auto ptr = funcbb->insts.buffer[j];
      koopa_raw_value_t bbInst = reinterpret_cast<koopa_raw_value_t>(ptr);
      if(bbInst->ty->tag != KOOPA_RTT_UNIT)
      {
          // cout<<spForEachInst<<endl;
          // cout<<bbInst<<endl;
          // stackForInsts[bbInst] = spForEachInst;
          spForEachInst += 4;
      }
      if(bbInst->kind.tag == KOOPA_RVT_CALL)
      {
          if(spForRa == 0){
            spForRa = 4;
          }
          koopa_raw_slice_t funcArgs = bbInst->kind.data.call.args;
          int lenForFuncArgs = funcArgs.len;
          maxLenForArgs = max(lenForFuncArgs, maxLenForArgs);
      }
    }
  }

  spForFuncParams = max(0, maxLenForArgs - 8)*4;
  int spForAll = spForEachInst + spForFuncParams + spForRa;
  spForAll = (spForAll + 15)/16*16;
  mapFuncToSp[func] = spForAll;
  mapFuncToRa[func] = spForRa;
  int specialSpForEachInst = 0;
  for (int i = 0; i < func->bbs.len; i++)
  {
      auto ptr = func->bbs.buffer[i];
      koopa_raw_basic_block_t funcbb = reinterpret_cast<koopa_raw_basic_block_t>(ptr);

      for (int j = 0; j < funcbb->insts.len; j++)
      {
        auto ptr = funcbb->insts.buffer[j];
        koopa_raw_value_t bbInst = reinterpret_cast<koopa_raw_value_t>(ptr);
        if (bbInst->ty->tag != KOOPA_RTT_UNIT)
        {
          stackForInsts[bbInst] = spForFuncParams + specialSpForEachInst;
          specialSpForEachInst += 4;
        }
      }
  }

  mapFuncToSp[func] = spForEachInst;
  koopa_raw_function_t prevFunc = curFunc;
  curFunc = func;
  myPrologue(curFunc);

  // 访问所有基本块
  Visit(func->bbs);
  curFunc = prevFunc;
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  # if DEBUGMODE
    cout << "******** visit block ********" << endl;
  # endif
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  string blockname = bb->name;
  blockname.erase(0,1);

  if (blockname != "entry")
    str1 += blockname + ":\n";
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  # if DEBUGMODE
    cout << "******** visit value ********" << endl;
  # endif
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      cout<<"KOOPA_RVT_BINARY"<<endl;
      Visit(kind.data.binary);
      writeTo(value, "t0");
      break;
    case KOOPA_RVT_STORE:
      // 访问 integer 指令
      cout<<"KOOPA_RVT_STORE"<<endl;
      Visit(kind.data.store);
      break;
    case KOOPA_RVT_LOAD:
      Visit(kind.data.load);
      writeTo(value, "t0");
      break;
    case KOOPA_RVT_ALLOC:
      cout<<"KOOPA_RVT_ALLOC"<<endl;
      break;
    case KOOPA_RVT_BRANCH:
      cout<<"KOOPA_RVT_BRANCH"<<endl;
      Visit(kind.data.branch);
      break;
    case KOOPA_RVT_JUMP:
      cout<<"KOOPA_RVT_JUMP"<<endl;
      Visit(kind.data.jump);
      break;
    case KOOPA_RVT_CALL:
      Visit(kind.data.call);
      writeTo(value, "a0");
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      Visit(kind.data.func_arg_ref);
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      myPrintGlobalVar(value);
      Visit(kind.data.global_alloc);
      break;
    case KOOPA_RVT_ZERO_INIT:
      str1 += " .zero 4\n";
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void Visit(const koopa_raw_return_t &ret) {
  # if DEBUGMODE
    cout << "******** visit return ********" << endl;
  # endif

  if (ret.value == NULL)  {}
  else if(stackForInsts.find(ret.value)!= stackForInsts.end() ){
    str1 += " lw a0, ";
    str1 += to_string(stackForInsts[ret.value]);
    str1 += "(sp)\n";
  }else{
    str1 += " li a0, ";
    str1 += to_string(ret.value->kind.data.integer.value);
    str1 += "\n";
  }
  
  myEpilogue(curFunc);
  str1 += " ret\n";
}

void Visit(const koopa_raw_call_t &call) 
{
  for (int i = 0; i < call.args.len; i++)
  {
    string destReg = "";
    if (i < 8)
    {
      destReg = "a"+to_string(i);
      readFrom(reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]), destReg);
    }
    else
    {
      string destMem = to_string((i-8)*4)+"(sp)";
      readFrom(reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]), "t0");
      str1 += "sw t0, "+destMem+"\n";
    }
  }
  string callFuncName = call.callee->name;
  callFuncName.erase(0,1);
  str1 += " call " +callFuncName+"\n";
}

void Visit(const koopa_raw_func_arg_ref_t &funcArgRef) 
{
}

void Visit(const koopa_raw_global_alloc_t &myGlobalAlloc) 
{
  if (myGlobalAlloc.init->kind.tag == KOOPA_RVT_INTEGER)
    str1 += " .word "+to_string(myGlobalAlloc.init->kind.data.integer.value)+"\n";
  else if (myGlobalAlloc.init->kind.tag == KOOPA_RVT_ZERO_INIT)
    Visit(myGlobalAlloc.init);
  else
    assert(false);

}

void Visit(const koopa_raw_integer_t &integer) {
  # if DEBUGMODE
    cout << "******** visit integer ********" << endl;
  # endif
  cout<<integer.value<<endl;
}

void Visit(const koopa_raw_binary_t &binary) {
  # if DEBUGMODE
    cout << "******** visit binary ********" << endl;
  # endif
  
  readFrom(binary.lhs, "t0");
  readFrom(binary.rhs, "t1");

  switch (binary.op) {
    case KOOPA_RBO_EQ:
      str1 += " xor t0, t0, t1\n";
      str1 += " seqz t0, t0\n";
      break;
    case KOOPA_RBO_NOT_EQ:
      str1 += " xor t0, t0, t1\n";
      str1 += " snez t0, t0\n";
      break;
    case KOOPA_RBO_GT:
      str1 += " sgt t0, t0, t1\n";
      break;
    case KOOPA_RBO_LT:
      str1 += " slt t0, t0, t1\n";
      break;
    case KOOPA_RBO_GE:
      str1 += " slt t0, t0, t1\n";
      str1 += " seqz t0, t0\n";
      break;
    case KOOPA_RBO_LE:
      str1 += " sgt t0, t0, t1\n";
      str1 += " seqz t0, t0\n";
      break;
    case KOOPA_RBO_SUB:
      str1 += " sub t0, t0, t1\n";
      break;
    case KOOPA_RBO_ADD:
      str1 += " add t0, t0, t1\n";
      break;
    case KOOPA_RBO_MUL:
      str1 += " mul t0, t0, t1\n";
      break;
    case KOOPA_RBO_DIV:
      str1 += " div t0, t0, t1\n";
      break;
    case KOOPA_RBO_MOD:
      str1 += " rem t0, t0, t1\n";
      break;
    case KOOPA_RBO_AND:
      str1 += " and t0, t0, t1\n";
      break;
    case KOOPA_RBO_OR:
      str1 += " or t0, t0, t1\n";
      break;
    case KOOPA_RBO_XOR:
      str1 += " xor t0, t0, t1\n";
      break;
    case KOOPA_RBO_SHL:
      str1 += " sll t0, t0, t1\n";
      break;
    case KOOPA_RBO_SHR:
      str1 += " srl t0, t0, t1\n";
      break;
    case KOOPA_RBO_SAR:
      str1 += " sra t0, t0, t1\n";
      break;
    default:
      assert(false);
  }
}


void Visit(const koopa_raw_store_t &rawStore) {
  if(rawStore.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
  {
    int myIndex = rawStore.value->kind.data.func_arg_ref.index;
    if(myIndex < 8)
      writeTo(rawStore.dest, "a"+to_string(rawStore.value->kind.data.func_arg_ref.index));
    else
    {
      int spNumForCalleeArgs = mapFuncToSp[curFunc] + (myIndex-8)*4;
      str1 += " lw t0, "+to_string(spNumForCalleeArgs)+"(sp)\n";
      writeTo(rawStore.dest, "t0");
    }
     
  }
  else
  {
    readFrom(rawStore.value, "t0");
    writeTo(rawStore.dest, "t0");
  }
}

void Visit(const koopa_raw_load_t &load) {
  readFrom(load.src, "t0");
}


void Visit(const koopa_raw_branch_t &branch) {
  readFrom(branch.cond, "t0");
  string trueLabel = branch.true_bb->name;
  trueLabel.erase(0,1);
  str1 += " bnez t0, "+trueLabel+"\n";
  string falseLabel = branch.false_bb->name; 
  falseLabel.erase(0,1);
  str1 += " j "+falseLabel+"\n";
 
}

void Visit(const koopa_raw_jump_t &jump) {
 string targetLabel = jump.target->name;
 targetLabel.erase(0,1);
 str1 += " j "+targetLabel+"\n";
}


int retValue(const koopa_raw_value_t &rawValue){
  const auto &kind = rawValue->kind;
  switch (kind.tag) {
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      return retValue(kind.data.integer);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}


int retValue(const koopa_raw_integer_t &rawInterger){
  return rawInterger.value;
}


int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  cout << "mode" << mode << endl;
  // 打开输入输出文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  yyout = fopen(output, "w");
  assert(yyin);

  // parse input file
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  std::cout << "successfully parsing the input file" << std::endl;

  // dump AST
  string str0 = "";
  ast->Dump(str0);
  cout << endl;

  //得到str，即KoopaIR
  cout << str0 <<endl;
  const char* str = str0.c_str();

  cout << "mode" << mode << endl;

  if(strcmp(mode,"-koopa") == 0)
  {
    cout << "yyout" << endl;
    for(int i = 0; i < strlen(str); i++)
    {
      char c = str[i];
      cout << c;
      if (c == '%') 
        fprintf(yyout,"%%");
      else
        fprintf(yyout,"%c", c);
    }
  }

  // 解析字符串 str, 得到 Koopa IR 程序
  koopa_program_t program;
  koopa_error_code_t ret0 = koopa_parse_from_string(str, &program);
  assert(ret0 == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  if (strcmp(mode, "-riscv") == 0)
  {
    cout << "riscv-out" << endl;
    Visit(raw);
  }

  koopa_delete_raw_program_builder(builder);

  // 输出解析得到的 AST, 其实就是个字符串
  // cout << *ast << endl;
  return 0;
}