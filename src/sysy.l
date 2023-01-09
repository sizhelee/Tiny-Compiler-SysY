%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"
#include "AST.h"

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*$
BlockComment  \/\*(?:[^\*]|\*+[^\/\*])*\*+\/

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*
Operator      "+"|"-"|"*"|"/"|"%"|"!"

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }
{BlockComment}  { /* 忽略, 不做任何操作 */ }

"int"           { return INT; }
"return"        { return RETURN; }
"const"         { return CONST; }
"if"            { return IF; }
"else"          { return ELSE; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Operator}      { auto ast = new Op(_OP, yytext[0]); yylval.ast_val = ast; return yytext[0]; }

"<"             { auto ast = new class LT(); yylval.ast_val = ast; return LT;}
">"             { auto ast = new class GT(); yylval.ast_val = ast; return GT;}
"<="            { auto ast = new class LE(); yylval.ast_val = ast; return LE;}
">="            { auto ast = new class GE(); yylval.ast_val = ast; return GE;}
"&&"            { auto ast = new class AND(); yylval.ast_val = ast; return AND;}
"||"            { auto ast = new class OR(); yylval.ast_val = ast; return OR;}
"=="            { auto ast = new class EQ(); yylval.ast_val = ast; return EQ;}
"!="            { auto ast = new class NE(); yylval.ast_val = ast; return NE;}

.               { return yytext[0]; }

%%
