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
extern std::map<koopa_raw_function_t, int> mapFuncToSp, mapFuncToRa;

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

void myPrologue(const koopa_raw_function_t &func)
{
    int sp = mapFuncToSp[func], ra = mapFuncToRa[func];
    if (sp != 0)
    {
        str1 += " addi sp, sp, " + std::to_string(-sp) + "\n";
        if (ra != 0)
            str1 += " sw ra, "+std::to_string(sp-4) +"(sp)\n";
    }
}


void writeTo(const koopa_raw_value_t &value, std::string srcReg)
{
    if(stackForInsts.find(value) == stackForInsts.end())
    {
        if(value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
        {
            std::string globalname = value->name;
            str1 += " la t1, ";
            str1 += globalname.erase(0,1);
            str1 += "\n";
            str1 += " sw t0, 0(t1)\n";
        }
    }
    else
    {
        str1 += " sw ";
        str1 += srcReg;
        str1 += ", ";
        str1 += std::to_string(stackForInsts[value]);
        str1 += "(sp)\n\n";
    }
}


void myEpilogue(const koopa_raw_function_t &func){
  int sp = mapFuncToSp[func], ra = mapFuncToRa[func];
  if (sp != 0)
  {
    str1 += " addi sp, sp, " + std::to_string(mapFuncToSp[func]) + "\n";
    if (ra != 0)
        str1 += " lw ra, "+std::to_string(sp-4) +"(sp)\n";
    str1 += " addi sp, sp, " + std::to_string(mapFuncToSp[func]) + "\n";
  }
}


void readFrom(const koopa_raw_value_t &value, std::string destReg){
    if(stackForInsts.find(value) != stackForInsts.end())
    {
        str1 += " lw ";
        str1 += destReg;
        str1 += ", ";
        str1 += std::to_string(stackForInsts[value]);
        str1 += "(sp)\n";
    }
    else if (value->kind.tag == KOOPA_RVT_INTEGER)
    {
        str1 += " li ";
        str1 += destReg;
        str1 += ", ";
        str1 += std::to_string(value->kind.data.integer.value);
        str1 += "\n";
    }
    else if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        std::string globalname = value->name;
        str1 += " la ";
        str1 += destReg;
        str1 += ", ";
        str1 += globalname.erase(0, 1);
        str1 += "\n";

        str1 += " lw ";
        str1 += destReg;
        str1 += ", 0(";
        str1 += destReg;
        str1 += ")\n";
    }
}


void myPrintGlobalVar (const koopa_raw_value_t &value)
{
  str1 += " .data\n";
  std::string myGlobalName = value->name;
  myGlobalName.erase(0, 1);
  str1 += " .global "+myGlobalName+"\n";
  str1 += myGlobalName+":\n";
}