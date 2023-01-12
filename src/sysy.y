%code requires {
  #include <memory>
  #include <string>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "AST.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
    std::string *str_val;
    int int_val;
    class BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT CONST
%token <int_val> INT_CONST
%token <ast_val> '=' '>' '<' '+' '-' '*' '/' '%' '!'
%token <ast_val> LE GE EQ NE LT GT AND OR

// 非终结符的类型定义
%type <ast_val> FuncDef Block Stmt Number CompUnit
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp AddExp MulExp
%type <ast_val> RelExp EqExp LAndExp LOrExp
%type <ast_val> Decl ConstDecl BType ConstDef ConstInitVal BlockItem
%type <ast_val> LVal ConstExp VarDecl VarDef InitVal FuncFParams
%type <ast_val> FuncFParam FuncRParams
%type <ast_val> myConstDef myBlockItem myVarDef myConstInitVal myInitVal
/* %type <int_val> Number */

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : CompUnit FuncDef {
    std::cout << "CompUnit -> CompUnit FuncDef" << std::endl;
    $1->son.push_back($2);
    $$ = $1;
  }
  | CompUnit Decl {
    std::cout << "CompUnit -> CompUnit Decl" << std::endl;
    $1->son.push_back($2);
    $$ = $1;
  }
  | FuncDef {
    // auto comp_unit = make_unique<CompUnitAST>();
    // comp_unit->func_def = unique_ptr<BaseAST>($1);
    // ast = move(comp_unit);
    std::cout << "CompUnit -> FuncDef" << std::endl;
    auto tmpast = new CompUnitAST();
    tmpast->func_def = unique_ptr<BaseAST>($1);
    tmpast->son.push_back($1);
    $$ = tmpast;

    auto newtmp = unique_ptr<BaseAST>(tmpast);
    ast = move(newtmp);
  } 
  | Decl {
    std::cout << "CompUnit -> Decl" << std::endl;
    auto tmpast = new CompUnitAST();
    tmpast->func_def = unique_ptr<BaseAST>($1);
    tmpast->son.push_back($1);
    $$ = tmpast;

    auto newtmp = unique_ptr<BaseAST>(tmpast);
    ast = move(newtmp);
  }
  ;


FuncDef
  : BType IDENT '(' ')' Block {
    std::cout << "FuncDef -> FuncType IDENT ( ) Block" << std::endl;
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->son.push_back($1);
    ast->son.push_back($5);
    $$ = ast;
  }
  | BType IDENT '(' FuncFParams ')' Block {
    std::cout << "FuncDef -> FuncType IDENT ( FuncFParams ) Block" << std::endl;
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($6);

    ast->son.push_back($1);
    ast->son.push_back($4);
    ast->son.push_back($6);
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam {
    std::cout << "FuncFParams -> FuncFParam" << std::endl;
    auto ast = new FuncFParamsAST();
    ast->son.push_back($1);
    ast->ident = $1->ident;
    $$ = ast;
  }
  | FuncFParams ',' FuncFParam {
    std::cout << "FuncFParams -> FuncFParams , FuncFParam" << std::endl;
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

FuncFParam
  : BType IDENT {
    std::cout << "FuncFParam -> BType IDENT" << std::endl;
    auto ast = new FuncFParamAST();
    ast->son.push_back($1);
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }
  ;
Block
  : '{' myBlockItem '}' {
    std::cout << "Block -> { myBlockItem }" << std::endl;
    auto ast = new BlockAST();
    ast->son = $2->son;
    ast->ret = $2->ret;
    $$ = ast;
  }
  ;

myBlockItem
  : myBlockItem BlockItem {
    std::cout << "myBlockItem -> myBlockItem BlockItem" << std::endl;
    $1->son.push_back($2);
    $1->ret = $1->ret || $2->ret; 
    $$ = $1;
  }
  | {
    std::cout << "myBlockItem -> " << std::endl;
    auto ast = new myBlockItem();
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    std::cout << "BlockItem -> Decl" << std::endl;
    auto ast = new BlockItem();
    ast->son.push_back($1);
    ast->ret = false;
    $$ = ast;
  }
  | Stmt {
    std::cout << "BlockItem -> Stmt" << std::endl;
    auto ast = new BlockItem();
    ast->son.push_back($1);
    ast->ret = $1->ret;
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    std::cout << "Stmt -> RETURN Exp ;" << std::endl;
    auto ast = new StmtAST();
    ast->val = $2->val;
    ast->exp = unique_ptr<BaseAST>($2);
    ast->son.push_back($2);
    ast->ret = true;
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    std::cout << "Stmt -> LVal = Exp ;" << std::endl;
    auto ast = new StmtAST();
    ast->val = $3->val;
    $1->val = $3->val;
    ast->son.push_back($1);
    ast->son.push_back($2);
    ast->son.push_back($3);
    $$ = ast;
  }
  | Exp ';' {
    std::cout << "Stmt -> Exp ;" << std::endl;
    auto ast = new StmtAST();
    ast->son.push_back($1);
    $$ = ast;
  }
  | ';' {
    std::cout << "Stmt -> ;" << std::endl;
    auto ast = new StmtAST();
    $$ = ast;
  }
  | Block {
    std::cout << "Stmt -> Block ;" << std::endl;
    auto ast = new StmtAST();
    ast->son.push_back($1);
    ast->ret = $1->ret;
    $$ = ast;
  }
  | RETURN ';' {
    std::cout << "Stmt -> RETURN ;" << std::endl;
    auto ast = new StmtAST();
    ast->ret = true;
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt ELSE Stmt {
    std::cout << "Stmt -> IF ( Exp ) Stmt ELSE Stmt" << std::endl;
    auto ast = new StmtAST();
    ast->isif = true;
    ast->son.push_back($3);
    ast->son.push_back($5);
    ast->son.push_back($7);
    $$ = ast;
  }
  | IF '(' Exp ')' Stmt {
    std::cout << "Stmt -> IF ( Exp ) Stmt" << std::endl;
    auto ast = new StmtAST();
    ast->isif = true;
    ast->son.push_back($3);
    ast->son.push_back($5);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    std::cout << "Stmt -> WHILE ( Exp ) Stmt" << std::endl;
    auto ast = new StmtAST();
    ast->iswhile = true;
    ast->son.push_back($3);
    ast->son.push_back($5);
    $$ = ast;
  }
  | BREAK {
    std::cout << "Stmt -> BREAK" << std::endl;
    auto ast = new StmtAST();
    ast->isbreak = true;
    $$ = ast;
  }
  | CONTINUE {
    std::cout << "Stmt -> CONTINUE" << std::endl;
    auto ast = new StmtAST();
    ast->iscontinue = true;
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    std::cout << "Exp -> LOrExp" << std::endl;
    auto ast = new ExpAST();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    std::cout << "( Exp )" << std::endl;
    auto ast = new PrimaryExp();
    ast->exp = unique_ptr<BaseAST>($2);
    ast->val = $2->val;
    ast->son.push_back($2);
    $$ = ast;
  }
  | Number {
    std::cout << "PrimaryExp -> Number" << std::endl;
    auto ast = new PrimaryExp();
    ast->val = $1->val;
    cout << "******** PrimaryExp-Number: " << ast->val << " ********" << endl;
    ast->son.push_back($1);
    $$ = ast;
  }
  | LVal {
    std::cout << "PrimaryExp -> LVal" << std::endl;
    auto ast = new PrimaryExp();
    ast->val = $1->val;
    cout << "******** PrimaryExp-Lval: " << ast->val << " ********" << endl;
    ast->son.push_back($1);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    std::cout << "Number -> INT_CONST" << std::endl;
    auto ast = new Number();
    ast->val = $1;
    ast->isint = true;
    $$ = ast;
  }
  ;

UnaryExp 
  : PrimaryExp{
    std::cout << "UnaryExp -> PrimaryExp" << std::endl;
    auto ast = new UnaryExp();
    ast->val = $1->val;
    ast->isint = $1->isint;
    ast->son.push_back($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp{
    std::cout << "UnaryExp -> UnaryOp UnaryExp" << std::endl;
    auto ast = new UnaryExp();
    ast->son.push_back($1);
    ast->son.push_back($2);

    if ($1->son[0]->op == '-')
      ast->val = 0 - ($2->val);
    else if ($1->son[0]->op == '!')
    {
      if ($2->val == 0)
        ast->val = 1;
      else ast->val = 0;
    }
    else ast->val = $2->val;

    $$ = ast;
  }
  | IDENT '(' ')' {
    std::cout << "UnaryExp -> IDENT ( )" << std::endl;
    auto ast = new UnaryExp();
    ast->isident = true;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    std::cout << "UnaryExp -> IDENT ( FuncRParams )" << std::endl;
    auto ast = new UnaryExp();
    ast->isident = true;
    ast->ident = *unique_ptr<string>($1);
    ast->son.push_back($3);
    $$ = ast;
  }
  ;

FuncRParams
  : Exp {
    std::cout << "FuncRParams -> Exp" << std::endl;
    auto ast = new FuncRParamsAST();
    ast->son.push_back($1);
    $$ = ast;
  }
  | FuncRParams ',' Exp {
    std::cout << "FuncRParams -> FuncRParams , Exp" << std::endl;
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

UnaryOp
  : '+'{
    auto ast = new UnaryOp();
    ast->son.push_back($1);
    std::cout<<"ast->type"<<ast->son[0]->op<<std::endl;
    $$ = ast;  
  }
  | '-'{
    auto ast = new UnaryOp();
    ast->son.push_back($1);
    $$ = ast;  
  }
  | '!'{
    auto ast = new UnaryOp();
    ast->son.push_back($1);
    $$ = ast;
  }
  ;

MulExp 
  : UnaryExp {
    auto ast = new MulExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    $1->val = $1->val * $3->val;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | MulExp '/' UnaryExp {
    if ($3->val != 0)
      $1->val = $1->val / $3->val;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | MulExp '%' UnaryExp {
    if ($3->val != 0)
      $1->val = $1->val % $3->val;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | AddExp '+' MulExp {
    $1->val = $1->val + $3->val;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | AddExp '-' MulExp {
    $1->val = $1->val - $3->val;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | RelExp LT AddExp {
    if ($1->val < $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | RelExp GT AddExp {
    if ($1->val > $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | RelExp LE AddExp {
    if ($1->val <= $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | RelExp GE AddExp {
    if ($1->val >= $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | EqExp EQ RelExp {
    if ($1->val == $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  | EqExp NE RelExp {
    if ($1->val != $3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | LAndExp AND EqExp {
    if (!$1->val)
      $1->val = 0;
    else if (!$3->val)
      $1->val = 0;
    else $1->val = 1;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    if ($1->val)
      $1->val = 1;
    else if ($3->val)
      $1->val = 1;
    else $1->val = 0;
    $1->son.push_back($2);
    $1->son.push_back($3);
    $$ = $1;
  }
  ;


Decl
  : ConstDecl {
    std::cout << "Decl -> ConstDecl" << std::endl;
    auto ast = new Decl();
    ast->son.push_back($1);
    $$ = ast;
  }
  | VarDecl {
    std::cout << "Decl -> VarDecl" << std::endl;
    auto ast = new Decl();
    ast->son.push_back($1);
    $$ = ast;
  }
  ;


ConstDecl
  : CONST BType myConstDef ';' {
    std::cout << "ConstDecl -> CONST BType myConstDef ;" << std::endl;
    auto ast = new ConstDecl();
    ast->son.push_back($2);
    ast->son.push_back($3);
    $$ = ast;
  }
  ;


myConstDef
  : myConstDef ',' ConstDef {
    std::cout << "myConstDef -> myConstDef , ConstDef" << std::endl;
    $1->son.push_back($3);
    $$ = $1;
  }
  | ConstDef {
    std::cout << "myConstDef -> ConstDef" << std::endl;
    auto ast = new myConstDef();
    ast->son.push_back($1);
    $$ = ast;
  }
  ;


BType
  : INT {
    std::cout << "BType -> INT" << std::endl;
    auto ast = new BType();
    ast->ident = "int";
    $$ = ast;
  }
  | VOID {
    std::cout << "BType -> VOID" << std::endl;
    auto ast = new BType();
    ast->ident = "void";
    $$ = ast;
  }
  ;


ConstDef
  : IDENT '=' ConstInitVal {
    std::cout << "ConstDef -> IDENT = ConstInitVal" << std::endl;
    symbol_table[*$1] = make_pair($3->val, 1);
    auto ast = new ConstDef();
    ast->ident = *unique_ptr<string>($1);
    ast->son.push_back($2);
    ast->son.push_back($3);
    ast->constval = $3->val;
    $$ = ast;
  }
  | IDENT '[' ConstExp ']' '=' ConstInitVal {
    symbol_table[*$1] = make_pair($3->val, 1);
    auto ast = new ConstDef();
    ast->isarray = true;
    ast->ident = *unique_ptr<string>($1);
    ast->son.push_back($3);
    ast->son.push_back($6);
    symbol_table[ast->ident] = make_pair(ast->val, 1);
    ast->val = $3->val;
    $$ = ast;
  }
  ;


ConstInitVal
  : ConstExp {
    std::cout << "ConstInitVal -> ConstExp" << std::endl;
    auto ast = new ConstInitVal();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new ConstInitVal();
    ast->isarray = true;
    $$ = ast;
  }
  | '{' myConstInitVal '}' {
    auto ast = new ConstInitVal();
    ast->isarray = true;
    ast->son.push_back($2);
    $$ = ast;
  }
  ;


myConstInitVal
  : myConstInitVal ',' ConstExp {
    $1->son.push_back($3);
    $$ = $1;
  }
  | ConstExp {
    auto ast = new myConstInitVal();
    ast->son.push_back($1);
    ast->val = $1->val;
    $$ = ast;
  }
  ;


LVal
  : IDENT {
    std::cout << "LVal -> IDENT" << std::endl;
    auto ast = new LVal();
    ast->ident = *unique_ptr<string>($1);
    ast->val = symbol_table[ast->ident].first;
    cout << "******** LVal(" << ast->ident << "): " << ast->val << " ********" << endl;
    $$ = ast;
  }
  | IDENT '[' Exp ']' {
    auto ast = new LVal();
    ast->ident = *unique_ptr<string>($1);
    ast->isarray = true;
    ast->val = $3->val;
    ast->son.push_back($3);
    $$ = ast;
  }
  ;


ConstExp
  : Exp {
    std::cout << "ConstExp -> Exp" << std::endl;
    auto ast = new ConstExp();
    ast->val = $1->val;
    ast->son.push_back($1);
    $$ = ast;
  }
  ;


VarDecl
  : BType myVarDef ';' {
    std::cout << "VarDecl -> BType myVarDef ;" << std::endl;
    auto ast = new VarDecl();
    ast->son.push_back($1);
    ast->son.push_back($2);
    $$ = ast;
  }
  ;


myVarDef
  : VarDef {
    std::cout << "myVarDef -> VarDef" << std::endl;
    auto ast = new myVarDef();
    ast->son.push_back($1);
    $$ = ast;
  }
  | myVarDef ',' VarDef {
    std::cout << "myVarDef -> myVarDef , VarDef" << std::endl;
    $1->son.push_back($3);
    $$ = $1;
  }
  ;


VarDef
  : IDENT {
    std::cout << "VarDef -> IDENT" << std::endl;
    auto ast = new VarDef();
    ast->ident = *unique_ptr<string>($1);
    symbol_table[ast->ident] = make_pair(0x7fffffff, -1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    std::cout << "VarDef -> IDENT = InitVal" << std::endl;
    auto ast = new VarDef();
    ast->ident = *unique_ptr<string>($1);
    ast->val = $3->val;
    ast->son.push_back($3);
    symbol_table[ast->ident] = make_pair(ast->val, 0);
    $$ = ast;
  }
  | IDENT '[' ConstExp ']' {
    auto ast = new VarDef();
    ast->ident = *unique_ptr<string>($1);
    ast->val = $3->val;
    ast->isarray = true;
    ast->son.push_back($3);
    symbol_table[ast->ident] = make_pair(ast->val, 1);
    $$ = ast;
  }
  | IDENT '[' ConstExp ']' '=' InitVal {
    auto ast = new VarDef();
    ast->ident = *unique_ptr<string>($1);
    ast->val = $3->val;
    ast->isarray = true;
    ast->son.push_back($3);
    ast->son.push_back($6);
    symbol_table[ast->ident] = make_pair(ast->val, 1);
    $$ = ast;
  } 
  ;


InitVal
  : Exp {
    std::cout << "InitVal -> Exp" << std::endl;
    auto ast = new InitVal();
    ast->son.push_back($1);
    ast->val = $1->val;
    $$ = ast;
  }
  | '{' myInitVal '}' {
    auto ast = new InitVal();
    ast->isarray = true;
    ast->son.push_back($2);
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new InitVal();
    ast->isarray = true;
    $$ = ast;
  }
  ;


myInitVal
  : myInitVal ',' Exp {
    $1->son.push_back($3);
    $$ = $1;
  }
  | Exp {
    auto ast = new myInitVal();
    ast->son.push_back($1);
    ast->val = $1->val;
    $$ = ast;
  }
  ;


/* FuncType
  : INT {
    std::cout << "FuncType -> INT" << std::endl;
    auto ast = new FuncTypeAST();
    string str = "int";
    ast->func_type = str;
    $$ = ast;
  }
  | VOID {
    std::cout << "FuncType -> VOID" << std::endl;
    auto ast = new FuncTypeAST();
    string str = "void";
    ast->func_type = str;
    $$ = ast;
  }
  ; */

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
    cerr << "error: " << s << endl;
}
