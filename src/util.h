#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#pragma once
extern FILE *yyout;

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