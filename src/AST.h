#include <iostream>
#include <memory>
#include <string>
#include <vector>
#pragma once

extern FILE *yyout;

enum TYPE{
  _UnaryExp, _PrimaryExp, _UnaryOp, _Number, _Exp, _OP, 
};
extern int tmpcnt;

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
    BaseAST(TYPE t, char o): type(t), op(o)
    {
        std::cout << "char_______op" << t << o << std::endl;
    }

    virtual ~BaseAST() = default;
    virtual void Dump(std::string& str0) const = 0;
    virtual std::string retvaltmp(std::string& str0)
    {
        return "";
    }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
    public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump(std::string& str0) const override {
        // std::cout << "CompUnitAST { ";
        str0 = "";
        func_def->Dump(str0);
        // std::cout << " }";
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump(std::string& str0) const override {
        // // std::cout << "FuncDefAST { ";
        // // func_type->Dump();
        // // std::cout << ", " << ident << ", ";
        // // block->Dump();
        // // std::cout << " }";
        // fprintf(yyout, "fun @");
        // std::cout << "fun @";
        // fprintf(yyout, ident.c_str());
        // fprintf(yyout, "(): ");
        // std::cout << ident << "(): ";
        // func_type->Dump();
        // fprintf(yyout, " { \n");
        // std::cout << " { "<< std::endl;
        // block->Dump();
        // fprintf(yyout, "}");
        // std::cout << "}";
        str0 += "fun @";
        str0 += ident;
        str0 += "(): ";
        std::cout << "fun @";
        std::cout << ident << "(): ";
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
            // std::cout << "FuncTypeAST { ";
            // std::cout << "int";
            // std::cout << " }";

            // fprintf(yyout, "i32");
            // std::cout << "i32";

            str0 += "i32";
            std::cout << "i32";
        }
    }
};

class BlockAST : public  BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump(std::string& str0) const override {
        // std::cout << "BlockAST { ";
        // stmt->Dump();
        // std::cout << " }";
        
        // fprintf(yyout, "%%");
        // fprintf(yyout, "entry");
        // fprintf(yyout, ":\n");
        // std::cout << "%";
        // std::cout << "entry";
        // std::cout << ":"<<std::endl;
        // stmt->Dump(str0);
        // fprintf(yyout, "\n");
        // std::cout << std::endl;

        str0 += "%";
        str0 += "entry";
        str0 += ":\n";
        std::cout << "%";
        std::cout << "entry";
        std::cout << ":"<<std::endl;
        stmt->Dump(str0);
        str0 += "\n";
        std::cout << std::endl;
    }
};

class StmtAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;

    void Dump(std::string& str0) const override {
        // std::cout << "StmtAST { ";
        // std::cout << number;
        // std::cout << " }";
        
        // fprintf(yyout, "  ret ");
        // fprintf(yyout, std::to_string(number).c_str());
        // std::cout <<"  "<< "ret ";
        // std::cout << number;

        // str0 += " ret ";
        // str0 += std::to_string(number).c_str();
        // std::cout <<"  "<< "ret ";
        // std::cout << number;

        std::string tmp = exp->retvaltmp(str0);
        std::cout<<"asdfghjkasdfghjkasdfghj"<<std::endl;
        str0 += " ret ";
        str0 += tmp.c_str();
        std::cout<<str0<<std::endl;
    }
};

class ExpAST : public BaseAST {
 public:
  ExpAST()
  {
    type = _Exp;
  }
  std::unique_ptr<BaseAST> unaryexp;
  char op;
  int val;
  
  void Dump(std::string& str0) const override {
  }

  std::string retvaltmp(std::string& str0) override
  {
    std::cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<std::endl;
    std::string tmp = unaryexp->retvaltmp(str0);
    return tmp;
  }
};

class PrimaryExp : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> number;
  int val;
  char op = '+';
  PrimaryExp()
  {
    type = _PrimaryExp;
  }
  void Dump(std::string& str0) const override {
  }
};

class Number : public BaseAST {
 public:

  Number()
  {
    type = _Number;
  }


  void Dump(std::string& str0) const override {
  }
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
  
  UnaryExp()
  {
    type = _UnaryExp;
  }
  
  void Dump(std::string& str0) const override {
  }


  std::string retvaltmp(std::string& str0) override {
    std::cout<<"11111111"<<std::endl;
    std::string tmp1;
    std::string tmp2;
    if(son[0]->type == _PrimaryExp)
    {
      std::cout<<"a11111111"<<std::endl;
      BaseAST* ptr = son[0]; 
      if(ptr->son[0]->type == _Number)
      {
        std::cout<<"aa11111111"<<std::endl;
        tmp1 = std::to_string(son[0]->son[0]->val);
      }else if(ptr->son[0]->type == _Exp)
      {
        std::cout<<"ab11111111"<<std::endl;
        
        tmp1 = ptr->son[0]->retvaltmp(str0);
      }
    }else if (son[0]->type == _UnaryOp)
    {
      std::cout<<"b11111111"<<std::endl;
      std::cout<<son[0]->op<<std::endl;
      std::cout<<son[0]->son[0]->op<<std::endl;
      if(son[0]->son[0]->op == '-')
      {
        std::cout<<"c11111111"<<std::endl;
        tmp2 = son[1]->retvaltmp(str0);
        tmp1 = "%" + std::to_string(tmpcnt);
        tmpcnt++;
        
        str0 += " ";
        str0 += tmp1.c_str();
        str0 += " = ";
        str0 += "sub ";
        str0 += "0, ";
        str0 += tmp2.c_str();
        str0 += "\n";
        std::cout<<str0<<std::endl;
      }else if (son[0]->son[0]->op == '!')
      {
        std::cout<<"d11111111"<<std::endl;
        tmp2 = son[1]->retvaltmp(str0);
        tmp1 = "%" + std::to_string(tmpcnt);
        tmpcnt++;
        
        str0 += " ";
        str0 += tmp1.c_str();
        str0 += " = ";
        str0 += "eq ";
        str0 += "0, ";
        str0 += tmp2.c_str();
        str0 += "\n";
        std::cout<<str0<<std::endl;
      }else if (son[0]->son[0]->op == '+')
      {
        std::cout<<"e11111111"<<std::endl;
        tmp1 = son[1]->retvaltmp(str0);
      }else
      {
        std::cout<<"wrong_op_type"<<std::endl;
      }
    }
    return tmp1;
  }
};

class UnaryOp : public BaseAST {
  public:
  // char op;
  UnaryOp()
  {
    type = _UnaryOp;
  }
  void Dump(std::string& str0) const override {
    
  }
};


class Op : public BaseAST {
  public:
  Op(TYPE t, char o):BaseAST(t,o){}
  // char op;
  void Dump(std::string& str0) const override {
   
  }
};