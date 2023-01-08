#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#pragma once

extern FILE *yyout;

enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, _Exp, _OP, _AddExp, _MulExp, _RelExp, 
  _EqExp, _LAndExp, _LOrExp, _LT, _GT, _LE, _GE, _AND, _OR, _EQ, _NE,
  _Decl, _ConstDecl, _BType, _ConstDef, _myConstDef, _ConstInitVal, _Block, 
  _BlockItem, _myBlockItem, _LVal, _ConstExp,

};
extern int expNumCnt;
extern std::map<std::string, int> symbol_table;

// 所有 AST 的基类
class BaseAST {
    public:

    std::vector<BaseAST *> son;
    TYPE type;
    int val;
    char op;
    bool isint = false;

    BaseAST() = default;
    BaseAST(TYPE t): type(t){}
    BaseAST(TYPE t, char o): type(t), op(o) {}

    virtual ~BaseAST() = default;
    virtual void Dump(std::string& str0) const = 0;
    
    virtual std::string dump2str(std::string& str0) {
        return "";
    }
};


// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_def;

    void Dump(std::string& str0) const override {
        str0 = "";
        func_def->Dump(str0);
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump(std::string& str0) const override {
        str0 += "fun @";
        str0 += ident;
        str0 += "(): ";
        std::cout << "fun @" << ident << "(): ";
        func_type->Dump(str0);

        str0 += " { \n";
        std::cout << " { "<<std::endl;
        block->Dump(str0);
        str0 += "}";
        std::cout << "}";
    }
};


class FuncTypeAST : public BaseAST {
    public:
    std::string func_type;

    void Dump(std::string& str0) const override {
        if(func_type == "int")
        {
            str0 += "i32";
            std::cout << "i32";
        }
    }
};


class BlockAST : public  BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump(std::string& str0) const override {
        str0 += "\%entry:\n";
        std::cout << "\%entry" << ":" << std::endl;

        // for(auto iter = son.begin(); iter != son.end(); iter++)
        //     (*iter)->dump2str(str0);
        for(int i = 0; i < son.size(); i++)
            son[i]->dump2str(str0);

        str0 += "\n";
        std::cout << std::endl;
    }
    std::string dump2str(std::string &str0) override 
    {
        return "";
    }
};


class myBlockItem : public BaseAST {
    public:
    myBlockItem()   { type = _myBlockItem;  }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override
    {
        for(auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->dump2str(str0);
        return "";
    }
};


class BlockItem: public BaseAST {
    public:
    BlockItem() { type = _BlockItem;    }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override
    {
        if(son[0]->type == _Decl)
            son[0]->dump2str(str0);
        else son[0]->Dump(str0);
        return "";
    }
};


class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;

    void Dump(std::string& str0) const override {
        std::string tmp = exp->dump2str(str0);
        str0 += " ret ";
        // str0 += tmp;
        str0 += std::to_string(val);
        std::cout << str0 << std::endl;
    }
};


class ExpAST : public BaseAST {
    public:
    ExpAST() {
        type = _Exp;
    }
  
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override {
        return son[0]->dump2str(str0);
    }
};


class PrimaryExp : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
    PrimaryExp() {
        type = _PrimaryExp;
    }
    void Dump(std::string& str0) const override {}
};


class Number : public BaseAST {
    public:

    Number() {
        type = _Number;
    }

    void Dump(std::string& str0) const override {}
};


class UnaryExp : public BaseAST {
    public:
    std::unique_ptr<BaseAST> primaryexp;
    std::unique_ptr<BaseAST> unaryop;
    std::unique_ptr<BaseAST> unaryexp;
  
    UnaryExp() {
        type = _UnaryExp;
    }
  
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override {
        std::string tmp1, tmp2;
        if(son[0]->type == _PrimaryExp)
        {
            BaseAST* ptr = son[0]; 
            if(ptr->son[0]->type == _Number)
                tmp1 = std::to_string(son[0]->son[0]->val);
            else if(ptr->son[0]->type == _Exp)
                tmp1 = ptr->son[0]->dump2str(str0);
            else if (ptr->son[0]->type == _LVal)
                tmp1 = std::to_string(son[0]->val);
        }

        else if (son[0]->type == _UnaryOp)
        {
            std::cout << son[0]->op << std::endl;
            std::cout << son[0]->son[0]->op << std::endl;
            if(son[0]->son[0]->op == '-' || son[0]->son[0]->op == '!')
            {
                tmp2 = son[1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                str0 += " ";
                str0 += tmp1.c_str();
                str0 += " = ";

                if (son[0]->son[0]->op == '-')
                    str0 += "sub ";
                else str0 += "eq ";

                str0 += "0, ";
                str0 += tmp2.c_str();
                str0 += "\n";
                std::cout << str0 << std::endl;
            }
            else if (son[0]->son[0]->op == '+')
                tmp1 = son[1]->dump2str(str0);

            else {
                std::cout << "wrong_op_type" << std::endl;
            }
        }
        return tmp1;
    }
};


class UnaryOp : public BaseAST {
    public:
    UnaryOp()
    {
        type = _UnaryOp;
    }
    void Dump(std::string& str0) const override {}
};


class Op : public BaseAST {
    public:
    Op(TYPE t, char o):BaseAST(t,o){}
    void Dump(std::string& str0) const override {}
};


class LT : public BaseAST {
    public:
    LT() {
        type = _LT;
    }
    void Dump(std::string& str0) const override {}
};


class LE : public BaseAST {
    public:
    LE() {
        type = _LE;
    }
    void Dump(std::string& str0) const override {}
};


class GT : public BaseAST {
    public:
    GT() {
        type = _GT;
    }
    void Dump(std::string& str0) const override {}
};


class GE : public BaseAST {
    public:
    GE() {
        type = _GE;
    }
    void Dump(std::string& str0) const override {}
};


class EQ : public BaseAST {
    public:
    EQ() {
        type = _EQ;
    }
    void Dump(std::string& str0) const override {}
};


class NE : public BaseAST {
    public:
    NE() {
        type = _NE;
    }
    void Dump(std::string& str0) const override {}
};


class AND : public BaseAST {
    public:
    AND() {
        type = _AND;
    }
    void Dump(std::string& str0) const override {}
};


class OR : public BaseAST {
    public:
    OR() {
        type = _OR;
    }
    void Dump(std::string& str0) const override {}
};


class AddExp : public BaseAST {
    public:
    AddExp(){
        type = _AddExp;
    }

    void Dump(std::string& str0) const override {}
    std::string dump2str(std::string& str0) override 
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->op == '+')
                str0 += "add";
            else if (son[i]->op == '-')
                str0 += "sub";
                
            str0 += " ";
            str0 += tmp2;
            str0 += ", ";
            str0 += tmp3;
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class MulExp : public BaseAST {
    public:
    MulExp() {  type = _MulExp; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override 
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->op == '*')
                str0 += "mul";
            else if (son[i]->op == '/')
                str0 += "div";
            else if (son[i]->op == '%')
                str0 += "mod";
                
            str0 += " ";
            str0 += tmp2;
            str0 += ", ";
            str0 += tmp3;
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class RelExp : public BaseAST {
    public:
    RelExp() {  type = _RelExp; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override 
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->type == _LT)
                str0 += "lt";
            else if (son[i]->type == _LE)
                str0 += "le";
            else if (son[i]->type == _GT)
                str0 += "gt";
            else if (son[i]->type == _GE)
                str0 += "ge";
                
            str0 += " ";
            str0 += tmp2;
            str0 += ", ";
            str0 += tmp3;
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class EqExp : public BaseAST {
    public:
    EqExp() {  type = _EqExp; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override 
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->type == _EQ)
                str0 += "eq";
            else if (son[i]->type == _NE)
                str0 += "ne";
                
            str0 += " ";
            str0 += tmp2;
            str0 += ", ";
            str0 += tmp3;
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class LAndExp : public BaseAST {
    public:
    LAndExp() { type = _LAndExp;    }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp2;
            str0 += "\n";

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp3;
            str0 += "\n";

            tmp1 = "%" + std::to_string(expNumCnt++);

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->type == _AND)
                str0 += "and";
                
            str0 += " %";
            str0 += std::to_string(expNumCnt-3);
            str0 += ", %";
            str0 += std::to_string(expNumCnt-2);
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class LOrExp : public BaseAST {
    public:
    LOrExp()    {   type = _LOrExp; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp2 = son[i - 1]->dump2str(str0);
            }

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp2;
            str0 += "\n";

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp3;
            str0 += "\n";

            tmp1 = "%" + std::to_string(expNumCnt++);

            str0 += " ";
            str0 += tmp1;
            str0 += " = ";

            if (son[i]->type == _OR)
                str0 += "or";
                
            str0 += " %";
            str0 += std::to_string(expNumCnt-3);
            str0 += ", %";
            str0 += std::to_string(expNumCnt-2);
            str0 += "\n";

            std::cout << str0 << std::endl;
        }
        return tmp1;
    }
};


class Decl : public BaseAST {
    public:
    Decl()  { type = _Decl;   }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override 
    {
        son[0]->dump2str(str0);
        return "";
    }
};


class ConstDecl: public BaseAST {
    public:
    ConstDecl() { type = _ConstDecl;    }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override 
    {
        for(auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->dump2str(str0);
        return "";
    }
};


class myConstDef : public BaseAST {
    public:
    myConstDef()    { type = _myConstDef;   }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override 
    {
        for(auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->dump2str(str0);
        return "";
    }
};


class BType: public BaseAST {
    public:
    std::string ident;
    BType() { type = _BType;    }
    void Dump(std::string& str0) const override {}
    std::string dump2str(std::string &str0) override 
    {
        return "";
    }
};


class ConstDef: public BaseAST {
    public:
    std::string ident;
    int constval;
    ConstDef()  { type = _ConstDef; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string &str0) override
    {
        std::cout << "ConstDef: " << ident << std::endl;
        symbol_table[ident] = constval;
        return "";
    }
};


class ConstInitVal: public BaseAST {
    public:
    ConstInitVal()  { type = _ConstInitVal; }
    void Dump(std::string& str0) const override {}
    std::string dump2str(std::string &str0) override 
    {
        return "";
    }
};


class ConstExp: public BaseAST {
    public:
    ConstExp()  { type = _ConstExp; }
    void Dump(std::string& str0) const override {}
    std::string dump2str(std::string &str0) override 
    {
        return "";
    }
};


class LVal: public BaseAST {
    public:
    std::string ident;
    LVal()  { type = _LVal; }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override
    {
        std::cout << "******** LVal *********\n";
        // std::cout << symbol_table[ident] << " ********\n";
        // val = symbol_table[ident];
        // // return std::to_string(val);
        // return ident;
        return "";
    }
};