#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>

#include "AST.h"
#include "koopa.h"
#include "util.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin, *yyout;
extern int yyparse(unique_ptr<BaseAST> &ast);

// 函数声明略
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  myPrint("  .text ");

  for (size_t i = 0; i < program.values.len; ++i) {
      // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
      // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
      assert(program.values.kind == KOOPA_RSIK_FUNCTION);
      // 获取当前函数
      koopa_raw_function_t func = (koopa_raw_function_t) program.values.buffer[i];
      // 进一步处理当前函数
      myPrint(func->name);
  }

  myPrint("  .global ", false);

  for (size_t i = 0; i < program.funcs.len; ++i) {
      // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
      // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
      assert(program.funcs.kind == KOOPA_RSIK_FUNCTION);
      // 获取当前函数
      koopa_raw_function_t func = (koopa_raw_function_t) program.funcs.buffer[i];

      // 进一步处理当前函数
      cout << func->name << endl;
      for(int i = 1; i < strlen(func->name); i++)
      {
        char c = func->name[i]; 
        cout << c;
        fprintf(yyout, "%c", c);
      }
      // fprintf(yyout, "%s",func->name);
      fprintf(yyout, "\n");
  }

  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
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
  // 执行一些其他的必要操作
  // ...
  for(int i = 1; i < strlen(func->name);i++)
  {
    char c = func->name[i]; 
    cout<<c;
    fprintf(yyout, "%c",c);
  }
  cout<<":"<<endl;
  fprintf(yyout, ":");
  fprintf(yyout, "\n");
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
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
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void Visit(const koopa_raw_return_t &ret) {

  cout<<"li a0, ";
  cout<<ret.value->kind.data.integer.value<<endl;
  cout<<"ret"<<endl;

  fprintf(yyout,"li a0, ");
  fprintf(yyout, "%d",ret.value->kind.data.integer.value);
  fprintf(yyout, "\n");
  fprintf(yyout, "ret");
  fprintf(yyout, "\n");
}

void Visit(const koopa_raw_integer_t &integer) {
  cout<<integer.value<<endl;
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