#include <iostream>
#include <memory>
#include <string>
#include <vector>
#pragma once

extern FILE *yyout;

enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, _Exp, _OP, _AddExp, _MulExp,
};
extern int expNumCnt;

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

        stmt->Dump(str0);

        str0 += "\n";
        std::cout << std::endl;
    }
};


class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;

    void Dump(std::string& str0) const override {
        std::string tmp = exp->dump2str(str0);
        str0 += " ret ";
        str0 += tmp;
        std::cout << str0 << std::endl;
    }
};


class ExpAST : public BaseAST {
    public:
    ExpAST() {
        type = _Exp;
    }
    // std::unique_ptr<BaseAST> unaryexp;
    char op;
    int val;
  
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override {
        return son[0]->dump2str(str0);
    }
};


class PrimaryExp : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
    int val;
    char op = '+';
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
    int val;
    char op;
    int cnt1;
    int cnt2;
  
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
    // char op;
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
            tmp3 = son[i + 1]->dump2str(str0);

            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = tmp1;
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
            tmp3 = son[i + 1]->dump2str(str0);
            
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
            }
            else
            {
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = tmp1;
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