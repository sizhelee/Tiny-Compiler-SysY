#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <map>
#include "koopa.h"
#pragma once
extern FILE *yyout;
extern std::string str1;
extern std::map<koopa_raw_value_t, int> stackForInsts;
extern std::map<koopa_raw_function_t, int> mapFuncToSp;

void myPrint(std::string s, bool enter = true)
{
    std::cout << s;
    fprintf(yyout, s.c_str());
    if (enter)
    {
        std::cout << std::endl;
        fprintf(yyout, "\n");
    }
}

void myPrologue(const koopa_raw_function_t &func){
  int sp = mapFuncToSp[func];
  if(sp != 0){
      str1 += " addi sp, sp, " + std::to_string(-sp) + "\n";
  }
}


void writeTo(const koopa_raw_value_t &value){
  if(stackForInsts.find(value) == stackForInsts.end()){
    assert(false);
  }
  str1 += " sw t0, "+std::to_string(stackForInsts[value])+"(sp)\n";
  str1 += "\n";
}


void myEpilogue(const koopa_raw_function_t &func){
  int sp = mapFuncToSp[func];
  if(sp == 0){

  }else{
     str1 += " addi sp, sp, " + std::to_string(mapFuncToSp[func]) + "\n";
  }
}


void readFrom(const koopa_raw_value_t &value, std::string destReg){
  // auto instType = value->ty->tag;
  if(stackForInsts.find(value) != stackForInsts.end()){
    str1 += " lw "+destReg+", "+ std::to_string(stackForInsts[value]) + "(sp)" + "\n";
  }else{
    str1 += " li "+destReg+", "+ std::to_string(value->kind.data.integer.value) + "\n";
  }
}