#include<iostream>
#include <memory>
#include <string>
#pragma once

extern FILE *yyout;

// 所有 AST 的基类
class BaseAST {
    public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
    public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
        // std::cout << "CompUnitAST { ";
        func_def->Dump();
        // std::cout << " }";
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() const override {
        // std::cout << "FuncDefAST { ";
        // func_type->Dump();
        // std::cout << ", " << ident << ", ";
        // block->Dump();
        // std::cout << " }";
        fprintf(yyout, "fun @");
        std::cout << "fun @";
        fprintf(yyout, ident.c_str());
        fprintf(yyout, "(): ");
        std::cout << ident << "(): ";
        func_type->Dump();
        fprintf(yyout, " { \n");
        std::cout << " { "<< std::endl;
        block->Dump();
        fprintf(yyout, "}");
        std::cout << "}";
    }
};

class FuncTypeAST : public BaseAST {
    public:
    std::string func_type;

    void Dump() const override {
        if(func_type == "int")
        {
            // std::cout << "FuncTypeAST { ";
            // std::cout << "int";
            // std::cout << " }";
            fprintf(yyout, "i32");
            std::cout << "i32";
        }
    }
};

class BlockAST : public  BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        // std::cout << "BlockAST { ";
        // stmt->Dump();
        // std::cout << " }";
        fprintf(yyout, "%%");
        fprintf(yyout, "entry");
        fprintf(yyout, ":\n");
        std::cout << "%";
        std::cout << "entry";
        std::cout << ":"<<std::endl;
        stmt->Dump();
        fprintf(yyout, "\n");
        std::cout << std::endl;
    }
};

class StmtAST : public BaseAST {
    public:
    int number;

    void Dump() const override {
        // std::cout << "StmtAST { ";
        // std::cout << number;
        // std::cout << " }";
        fprintf(yyout, "  ret ");
        fprintf(yyout, std::to_string(number).c_str());
        std::cout <<"  "<< "ret ";
        std::cout << number;
    }
};