/* 一些细节                                                                     *
 * 1) 关于decl语句,dump和dump2str本身区别不大,有无返回值主要用于区分是否为全局     *
 *   变量,实际上均不需要返回值,只是借用dump函数向str0输出global                   *
 * 2) 很多时候dump和dump2str都被赋予其他作用
 * 3) 由于处理时细节问题考虑并不清晰，处理stmt时需先判断return，再判断if           *
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <variant>
#pragma once

extern FILE *yyout;

enum TYPE{
    _CompUnit, _FuncDef, _Stmt, _UnaryExp, _PrimaryExp, _UnaryOp, _Number, 
    _Exp, _OP, _AddExp, _MulExp, _RelExp, _EqExp, _LAndExp, _LOrExp, _LT, _GT,
    _LE, _GE, _AND, _OR, _EQ, _NE, _Decl, _ConstDecl, _BType, _ConstDef, 
    _myConstDef, _ConstInitVal, _Block, _BlockItem, _myBlockItem, _LVal, 
    _ConstExp, _VarDecl, _myVarDef, _VarDef, _InitVal, _FuncFParamsAST, 
    _FuncFParamAST, _FuncRParamsAST, _myInitVal, _myConstInitVal, 

};
extern int expNumCnt, symTabCnt, ifNumCnt, whileNumCnt, allsymTabCnt, brctNumCnt, expIfCnt;
extern std::map<std::string, std::pair<int, int>> symbol_table;
extern std::map<std::string, std::pair<int, int>> *current_table;
extern std::map<std::map<std::string, std::pair<int, int>>*, std::map<std::string, std::pair<int, int>>*> total_table;
extern std::string now_while_end, now_while_entry;

extern std::map<std::string, std::string> func_table;
extern std::map<std::string, std::pair<int, int>> glob_table;

// 所有 AST 的基类
class BaseAST {
    public:

    std::vector<BaseAST*> son; // 记录下层的结构
    TYPE type;
    int val;
    char op;
    bool isident = false;
    bool isarray = false;
    bool isint = false, ret = false;
    bool isif = false, iswhile = false, isbreak = false, iscontinue = false;
    std::string ident;

    BaseAST() = default;
    BaseAST(TYPE t): type(t){}
    BaseAST(TYPE t, char o): type(t), op(o) {}

    virtual ~BaseAST() = default;
    virtual void Dump(std::string& str0) const = 0; // 向字符串str0输出本层代码
    
    virtual std::string dump2str(std::string& str0) // 向str输出本层代码并返回结果的寄存器
    {
        return "";
    }
};


// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_def;

    CompUnitAST()   { type = _CompUnit; }

    void Dump(std::string& str0) const override 
    {
        current_table = &symbol_table;
        str0 = "";

        str0 += "decl @getint(): i32\n";
        str0 += "decl @getch(): i32\n";
        str0 += "decl @getarray(*i32): i32\n";
        str0 += "decl @putint(i32)\n";
        str0 += "decl @putch(i32)\n";
        str0 += "decl @putarray(i32, *i32)\n";
        str0 += "decl @starttime()\n";
        str0 += "decl @stoptime()\n";

        func_table["getint"] = "int";
        func_table["getch"] = "int";
        func_table["getarray"] = "int";
        func_table["putint"] = "void";
        func_table["putch"] = "void";
        func_table["putarray"] = "void";
        func_table["starttime"] = "void";
        func_table["stoptime"] = "void";

        symTabCnt += 1;
        allsymTabCnt += 1;
        for (auto iter = son.begin(); iter != son.end(); iter++)
        {    
            (*iter)->Dump(str0);
            std::cout << str0 << std::endl;
        }
        symTabCnt -= 1;
    }
};


// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    FuncDefAST()    { type = _FuncDef;  }

    void Dump(std::string& str0) const override 
    {
        str0 += "fun @";
        str0 += ident;
        func_table[ident] = func_type->dump2str(str0);
        str0 += "(";

        if (son.size() == 3)
            son[1]->Dump(str0);
        str0 += ")";
        if(func_type->dump2str(str0) == "int")
            str0 += ": i32";
        else if (func_type->dump2str(str0) == "void")
            str0 += "";

        str0 += " { \n";
        str0 += "\%entry:\n";

        // 为函数新建局部符号表
        std::map<std::string, std::pair<int, int>> new_table;
        std::map<std::string, std::pair<int, int>>* record_table = current_table;
        current_table = &new_table;
        total_table[current_table] = record_table;
        symTabCnt += 1;
        allsymTabCnt += 1;

        if (son.size() == 3)
        {
            for (auto iter = son[1]->son.begin(); iter != son[1]->son.end(); iter++)
            {
                std::string tmp = (*iter)->dump2str(str0);
                str0 += " @";
                str0 += tmp;
                str0 += "_";
                str0 += std::to_string(allsymTabCnt);

                str0 += " = alloc i32\n";
                str0 += " store @";
                str0 += tmp;
                str0 += ", @";
                str0 += tmp;
                str0 += "_";
                str0 += std::to_string(allsymTabCnt);
                str0 += "\n";

                std::string new_tab_name = tmp+"_"+std::to_string(allsymTabCnt);
                (*current_table)[new_tab_name] = std::make_pair((*iter)->val, 0);
            }
        }

        // std::cout << "###### DEBUG SYM TABLE ######\n";
        // std::cout << "all tab num: " << allsymTabCnt << " ";
        // std::cout << "exist tab num: " << symTabCnt << std::endl;
        // for (auto iter = current_table->begin(); iter != current_table->end(); iter++)
        // {
        //     std::cout << (*iter).first << " ";
        //     std::cout << (*iter).second.first << std::endl;
        // }

        block->Dump(str0);

        if (func_type->dump2str(str0) == "void")
            str0 += " ret \n";

        str0 += "}\n";
        current_table = record_table; // 注意函数退出后返回上一层符号表
        symTabCnt -= 1;
    }
};


class FuncTypeAST : public BaseAST {
    public:
    std::string func_type;

    void Dump(std::string& str0) const override {
        if(func_type == "int")
        {
            str0 += ": i32";
            std::cout << ": i32";
        }
        else if (func_type == "void")
            str0 += " ";
    }

    std::string dump2str(std::string& str0) override {
        return func_type;
    }
};


class FuncFParamsAST : public BaseAST {
    public:
    FuncFParamsAST()    { type = _FuncFParamsAST;   }

    void Dump(std::string& str0) const override {
        int son_size = son.size();
        for (int i = 0; i < son_size; i++)
        {
            son[i]->Dump(str0);
            if (i != son_size - 1)
                str0 += ", ";
        }
    }
};


class FuncFParamAST : public BaseAST {
    public:
    std::string ident;

    void Dump(std::string &str0) const override {
        str0 += "@" + ident;
        str0 += ": ";
        son[0]->Dump(str0);
    }

    std::string dump2str(std::string &str0) override {
        return ident;
    }
};


class FuncRParamsAST : public BaseAST {
    public:
    void Dump(std::string& str0) const override 
    {
        for (auto iter = son.begin(); iter != son.end(); iter++)
        {
            std::string tmp = (*iter)->dump2str(str0);
            if (iter != son.begin())
                str0 += ", ";
            str0 += tmp;
        }
    }
};


class BlockAST : public  BaseAST {
    public:
    std::unique_ptr<BaseAST> stmt;
    BlockAST()  { type = _Block;    }

    void Dump(std::string& str0) const override 
    {
        // 每一块需要自己的符号表
        std::map<std::string, std::pair<int, int>> new_table;
        std::map<std::string, std::pair<int, int>>* record_table = current_table;
        current_table = &new_table;
        total_table[current_table] = record_table;
        symTabCnt += 1;
        allsymTabCnt += 1;

        // std::cout << "******* block item num: " << son.size() << " *********\n";
        
        for(auto iter = son.begin(); iter != son.end(); iter++)
        {
            (*iter)->dump2str(str0);
            if ((*iter)->ret)
                break;
        }

        str0 += "\n";

        current_table = record_table;
        symTabCnt -= 1;
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
        {
            (*iter)->dump2str(str0);
            if ((*iter)->ret)
                break;
        }
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
    StmtAST()   { type = _Stmt; }

    void Dump(std::string& str0) const override 
    {
        if (ret)
        {
            if (son.size() > 0)
            {
                if (son[0]->type == _Block)
                    son[0]->Dump(str0);
                else
                {
                    std::string tmp = exp->dump2str(str0);
                    str0 += " ret ";
                    str0 += tmp;
                    str0 += "\n";
                }     
            }
            else str0 += " ret 0\n";
        }

        else if (isif)
        {
            std::cout << "@@@@@@@ Stmt IF statement @@@@@@@\n";
            std::string tmp = son[0]->dump2str(str0);

            // 记录当前的if信息，处理嵌套if
            int ifNumCnt_tmp = ifNumCnt;
            ifNumCnt++;

            str0 += " br ";
            str0 += tmp;
            str0 += ", \%then_";
            str0 += std::to_string(ifNumCnt_tmp);
            str0 += ", ";

            if (son.size() == 3) // if-else
                str0 += "\%else_";
            else if (son.size() == 2)
                str0 += "\%end_";

            str0 += std::to_string(ifNumCnt_tmp);
            str0 += "\n";

            // deal with the 'then' statement
            str0 += "\%then_";
            str0 += std::to_string(ifNumCnt_tmp);
            str0 += ":\n";
            son[1]->Dump(str0);
            if (!son[1]->ret)
            {
                str0 += " jump \%end_";
                str0 += std::to_string(ifNumCnt_tmp);
                str0 += "\n";
            }

            // deal with the 'else' statement
            if (son.size() == 3)
            {
                str0 += "\%else_";
                str0 += std::to_string(ifNumCnt_tmp);
                str0 += ":\n";
                son[2]->Dump(str0);
                if (!son[2]->ret)
                {
                    str0 += " jump \%end_";
                    str0 += std::to_string(ifNumCnt_tmp);
                    str0 += "\n";
                }
            }

            str0 += "\%end_";
            str0 += std::to_string(ifNumCnt_tmp);
            str0 += ":\n";
        }

        else if (iswhile)
        {
            std::cout << "@@@@@@@ Stmt WHILE statement @@@@@@@\n";
            // 记录当前的while信息，处理嵌套while
            int whileNumCnt_tmp = whileNumCnt;
            whileNumCnt += 1;
            std::string now_while_end_tmp = now_while_end;
            std::string now_while_entry_tmp = now_while_entry;

            now_while_end = "\%while_end_" + std::to_string(whileNumCnt_tmp);
            now_while_entry = "\%while_entry_" + std::to_string(whileNumCnt_tmp);

            str0 += " jump \%while_entry_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += "\n";

            str0 += "\%while_entry_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += ":\n";

            std::string tmp1 = son[0]->dump2str(str0);
            str0 += " br ";
            str0 += tmp1;
            str0 += ", \%while_body_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += ", \%while_end_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += "\n";

            str0 += "\%while_body_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += ":\n";
            son[1]->Dump(str0);

            if (!son[1]->ret)
            {
                str0 += " jump \%while_entry_";
                str0 += std::to_string(whileNumCnt_tmp);
                str0 += "\n";
            }

            str0 += "\%while_end_";
            str0 += std::to_string(whileNumCnt_tmp);
            str0 += ":\n";

            now_while_end = now_while_end_tmp;
            now_while_entry = now_while_entry_tmp;
        }

        else if (isbreak)
        {
            str0 += " jump ";
            str0 += now_while_end;
            str0 += "\n";

            str0 += "\%while_body1_";
            str0 += std::to_string(brctNumCnt);
            str0 += ":\n";
            brctNumCnt += 1;
        }

        else if (iscontinue)
        {
            str0 += " jump ";
            str0 += now_while_entry;
            str0 += "\n";

            str0 += "\%while_body1_";
            str0 += std::to_string(brctNumCnt);
            str0 += ":\n";
            brctNumCnt += 1;
        }

        else if (son.size() == 0)
            return;
        else if (son[0]->type == _LVal)
        {
            std::string exptmp = son[2]->dump2str(str0);
            std::string identtmp = son[0]->dump2str(str0);
            std::string identtmp_num;

            // 在符号表中逐层查找被赋值元素
            int searchdep = symTabCnt;
            std::map<std::string, std::pair<int, int>> *search_table = current_table;
            while (searchdep > 0)
            {   
                int flag = 0;
                for (int ident_idx = allsymTabCnt; ident_idx >= searchdep; ident_idx--)
                {
                    identtmp_num = identtmp + '_' + std::to_string(ident_idx);
                    if ((*search_table).find(identtmp_num) != (*search_table).end())
                        flag = 1;
                    if (flag)
                        break;
                }
                if (flag)
                    break;
                
                search_table = total_table[search_table];
                searchdep -= 1;
            }

            str0 += " store ";
            str0 += exptmp;
            str0 += ", @";
            str0 += identtmp_num;
            str0 += "\n";
            // std::cout << str0 << std::endl;
        }

        else if (son[0]->type == _Exp)
        {
            std::string tmp = son[0]->dump2str(str0);
        }

        else if (son[0]->type == _Block)
        {
            son[0]->Dump(str0);
        }
    }
};


class ExpAST : public BaseAST {
    public:
    ExpAST()    { type = _Exp;  }
    // 表达式都需要返回结果寄存器
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override {
        return son[0]->dump2str(str0);
    }
};


class PrimaryExp : public BaseAST {
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
    PrimaryExp()    { type = _PrimaryExp;   }
    void Dump(std::string& str0) const override {}
};


class Number : public BaseAST {
    public:

    Number()    { type = _Number;   }

    void Dump(std::string& str0) const override {}
};


class UnaryExp : public BaseAST {
    public:
    std::unique_ptr<BaseAST> primaryexp;
    std::unique_ptr<BaseAST> unaryop;
    std::unique_ptr<BaseAST> unaryexp;
  
    UnaryExp()  { type = _UnaryExp; }
  
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override 
    {
        std::string tmp1, tmp2;
        if (isident)
        {
            std::string funcretreg;
            if (func_table[ident] == "int")
            {
                funcretreg = "\%" + std::to_string(expNumCnt++);
                if (son.size() > 0)
                {
                    std::vector<std::string> allFuncParams;
                    for (auto iter = son[0]->son.begin(); iter != son[0]->son.end(); iter++)
                    {
                        std::string funcParam = (*iter)->dump2str(str0);
                        allFuncParams.push_back(funcParam);
                    }

                    str0 += " ";
                    str0 += funcretreg;
                    str0 += " = call @";
                    str0 += ident;
                    str0 += "(";

                    for (int i = 0; i < allFuncParams.size(); i++)
                    {
                        if (i)
                            str0 += ", ";
                        str0 += allFuncParams[i];
                    }
                    str0 += ")\n";
                }
                else 
                {
                    str0 += " ";
                    str0 += funcretreg;
                    str0 += " = call @";
                    str0 += ident;
                    str0 += "()\n";
                }
            }

            else if (func_table[ident] == "void")
            {
                funcretreg = "";

                if (son.size() > 0)
                {
                    std::vector<std::string> allFuncParams;
                    for (auto iter = son[0]->son.begin(); iter != son[0]->son.end(); iter++)
                    {
                        std::string funcParam = (*iter)->dump2str(str0);
                        allFuncParams.push_back(funcParam);
                    }

                    str0 += " call @";
                    str0 += ident;
                    str0 += "(";
                    for (int i = 0; i < allFuncParams.size(); i++)
                    {
                        if (i)
                            str0 += ", ";
                        str0 += allFuncParams[i];
                    }
                    str0 += ")\n";
                }
                else
                {
                    str0 += " call @";
                    str0 += ident;
                    str0 += "()\n";
                }
            }

            return funcretreg;
        }

        else if(son[0]->type == _PrimaryExp)
        {
            BaseAST* ptr = son[0]; 
            if(ptr->son[0]->type == _Number)
                tmp1 = std::to_string(son[0]->son[0]->val);
            else if(ptr->son[0]->type == _Exp)
                tmp1 = ptr->son[0]->dump2str(str0);
            else if (ptr->son[0]->type == _LVal)
            {
                tmp1 = ptr->son[0]->dump2str(str0);

                if (ptr->son[0]->isarray)
                {
                    std::string strtmp = ptr->son[0]->son[0]->dump2str(str0);

                    std::string identtmp = tmp1;
                    std::string identtmp_num;
                    int searchdep = symTabCnt;
                    std::map<std::string, std::pair<int, int>> *search_table = current_table;    

                    while (searchdep > 0)
                    {
                        int flag = 0;
                        for (int ident_idx = allsymTabCnt; ident_idx >= searchdep; ident_idx--)
                        {
                            identtmp_num = identtmp + '_' + std::to_string(ident_idx);
                            if ((*search_table).find(identtmp_num) != (*search_table).end())
                                flag = 1;
                            if (flag)
                                break;
                        }
                        if (flag)
                            break;
                        if (searchdep > 1)
                            search_table = total_table[search_table];
                        searchdep -= 1;
                    }

                    str0 += " \%";
                    str0 += std::to_string(expNumCnt++);
                    str0 += " = getelemptr @";
                    str0 += identtmp_num;
                    str0 += ", ";
                    str0 += strtmp;
                    str0 += "\n \%";
                    str0 += std::to_string(expNumCnt++);
                    str0 += " = load \%";
                    str0 += std::to_string(expNumCnt-2);
                    str0 += "\n";
                    
                    return "\%" + std::to_string(expNumCnt-1);
                }

                else
                {
                    std::string identtmp = tmp1;
                    std::string identtmp_num;
                    int searchdep = symTabCnt;
                    std::map<std::string, std::pair<int, int>> *search_table = current_table;
                    
                    // std::cout << "###### DEBUG SYM TABLE ######\n";
                    // std::cout << "all tab num: " << allsymTabCnt << " ";
                    // std::cout << "exist tab num: " << symTabCnt << std::endl;
                    // for (auto iter = current_table->begin(); iter != current_table->end(); iter++)
                    // {
                    //     std::cout << (*iter).first << " ";
                    //     std::cout << (*iter).second.first << std::endl;
                    // }

                    while (searchdep > 0)
                    {
                        int flag = 0;
                        for (int ident_idx = allsymTabCnt; ident_idx >= searchdep; ident_idx--)
                        {
                            identtmp_num = identtmp + '_' + std::to_string(ident_idx);
                            if ((*search_table).find(identtmp_num) != (*search_table).end())
                                flag = 1;
                            if (flag)
                                break;
                        }
                        if (flag)
                            break;
                        if (searchdep > 1)
                            search_table = total_table[search_table];
                        searchdep -= 1;
                    }

                    // std::cout << "###### DEBUG SYM TABLE ######\n";
                    // std::cout << "all tab num: " << allsymTabCnt << " ";
                    // std::cout << "exist tab num: " << symTabCnt << std::endl;
                    // std::cout << "search depth: " << searchdep << " ";
                    // std::cout << "search for: " << identtmp_num << std::endl;
                    // for (auto iter = search_table->begin(); iter != search_table->end(); iter++)
                    // {
                    //     std::cout << (*iter).first << " ";
                    //     std::cout << (*iter).second.first << " " << (*iter).second.second << std::endl;
                    // }

                    if (searchdep == 0) // 在全局函数表中
                    {
                        auto sym_label_val = symbol_table[tmp1];
                        return std::to_string(sym_label_val.first);
                    }
                    
                    auto sym_label_val = (*search_table)[identtmp_num];
                    if (sym_label_val.second == 0) // 非const
                    {
                        tmp2 = "%" + std::to_string(expNumCnt++);
                        str0 += " ";
                        str0 += tmp2;
                        str0 += " = load @";
                        str0 += identtmp_num;
                        str0 += "\n";
                        std::cout << str0;
                    }
                    else tmp2 = std::to_string(sym_label_val.first);
                    return tmp2;
                }
            }
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
                // std::cout << str0 << std::endl;
            }
            else if (son[0]->son[0]->op == '+')
                tmp1 = son[1]->dump2str(str0);
        }
        return tmp1;
    }
};


class UnaryOp : public BaseAST {
    public:
    UnaryOp()   { type = _UnaryOp;  }
    void Dump(std::string& str0) const override {}
};


class Op : public BaseAST {
    public:
    Op(TYPE t, char o):BaseAST(t,o){}
    void Dump(std::string& str0) const override {}
};


class LT : public BaseAST {
    public:
    LT()    { type = _LT;   }
    void Dump(std::string& str0) const override {}
};


class LE : public BaseAST {
    public:
    LE()    { type = _LE;   }
    void Dump(std::string& str0) const override {}
};


class GT : public BaseAST {
    public:
    GT()    { type = _GT;   }
    void Dump(std::string& str0) const override {}
};


class GE : public BaseAST {
    public:
    GE()    { type = _GE;   }
    void Dump(std::string& str0) const override {}
};


class EQ : public BaseAST {
    public:
    EQ()    { type = _EQ;   }
    void Dump(std::string& str0) const override {}
};


class NE : public BaseAST {
    public:
    NE()    { type = _NE;   }
    void Dump(std::string& str0) const override {}
};


class AND : public BaseAST {
    public:
    AND()   { type = _AND;  }
    void Dump(std::string& str0) const override {}
};


class OR : public BaseAST {
    public:
    OR()    { type = _OR;   }
    void Dump(std::string& str0) const override {}
};


class AddExp : public BaseAST {
    public:
    AddExp()    { type = _AddExp;   }

    void Dump(std::string& str0) const override {}
    std::string dump2str(std::string& str0) override 
    {
        if (son.size() == 1)
            return son[0]->dump2str(str0);
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                lasttmp = tmp1;
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = lasttmp;
                lasttmp = tmp1;
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
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                lasttmp = tmp1;
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = lasttmp;
                lasttmp = tmp1;
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
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                lasttmp = tmp1;
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = lasttmp;
                lasttmp = tmp1;
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
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {   
            if (i == 1)
            {
                tmp2 = son[0]->dump2str(str0);
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                lasttmp = tmp1;
            }
            else
            {
                tmp3 = son[i + 1]->dump2str(str0);
                tmp1 = "%" + std::to_string(expNumCnt++);
                tmp2 = lasttmp;
                lasttmp = tmp1;
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
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {   
            int expIfCnt_ori = expIfCnt;
            expIfCnt++;
            std::string tmpreg = "@tttmp_" + std::to_string(expIfCnt_ori);
            str0 += " ";
            str0 += tmpreg;
            str0 += " = alloc i32\n";
            str0 += " store 0, " + tmpreg + "\n";

            if (i == 1)
                tmp2 = son[0]->dump2str(str0);
            else
                tmp2 = lasttmp;

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp2;
            str0 += "\n";

            str0 += " br %";
            str0 += std::to_string(expNumCnt-1);
            str0 += ", \%expthen_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ", \%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += "\n";

            str0 += "\%expthen_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ":\n";
            tmp3 = son[i + 1]->dump2str(str0);

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp3;
            str0 += "\n";

            str0 += " store %";
            str0 += std::to_string(expNumCnt-1);
            str0 += ", " + tmpreg + "\n";
            str0 += " jump \%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += "\n";

            str0 += "\%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ":\n";

            tmp1 = "%" + std::to_string(expNumCnt++);
            lasttmp = tmp1;

            str0 += " ";
            str0 += tmp1;
            str0 += " = load " + tmpreg + "\n";
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
        std::string tmp1, tmp2, tmp3, lasttmp;

        for(int i = 1; i < son.size(); i += 2)
        {   
            int expIfCnt_ori = expIfCnt;
            expIfCnt++;
            std::string tmpreg = "@tttmp_" + std::to_string(expIfCnt_ori);
            str0 += " ";
            str0 += tmpreg;
            str0 += " = alloc i32\n";
            str0 += " store 1, " + tmpreg + "\n";

            if (i == 1)
                tmp2 = son[0]->dump2str(str0);
            else
                tmp2 = lasttmp;

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = eq 0, ";
            str0 += tmp2;
            str0 += "\n";

            str0 += " br %";
            str0 += std::to_string(expNumCnt-1);
            str0 += ", \%expthen_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ", \%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += "\n";

            str0 += "\%expthen_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ":\n";
            tmp3 = son[i + 1]->dump2str(str0);

            str0 += " %";
            str0 += std::to_string(expNumCnt++);
            str0 += " = ne 0, ";
            str0 += tmp3;
            str0 += "\n";

            str0 += " store %";
            str0 += std::to_string(expNumCnt-1);
            str0 += ", " + tmpreg + "\n";
            str0 += " jump \%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += "\n";

            str0 += "\%expend_";
            str0 += std::to_string(expIfCnt_ori);
            str0 += ":\n";

            tmp1 = "%" + std::to_string(expNumCnt++);
            lasttmp = tmp1;

            str0 += " ";
            str0 += tmp1;
            str0 += " = load " + tmpreg + "\n";
        }
        return tmp1;
    }
};


class Decl : public BaseAST {
    public:
    Decl()  { type = _Decl;   }
    void Dump(std::string& str0) const override 
    {
        son[0]->Dump(str0);
    }

    std::string dump2str(std::string &str0) override 
    {
        son[0]->dump2str(str0);
        return "";
    }
};


class ConstDecl: public BaseAST {
    public:
    ConstDecl() { type = _ConstDecl;    }
    void Dump(std::string& str0) const override 
    {
        son[1]->Dump(str0);
    }

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
    void Dump(std::string& str0) const override 
    {
        std::cout << "BBBBBBBBBBBB\n";
        std::cout << son[0]->type << std::endl;
        for (auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->Dump(str0);
    }

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
    void Dump(std::string& str0) const override 
    {
        if(ident == "int")
            str0 += "i32";
    }
    std::string dump2str(std::string &str0) override 
    {
        return ident;
    }
};


class ConstDef: public BaseAST {
    public:
    std::string ident;
    int constval;
    ConstDef()  { type = _ConstDef; }
    void Dump(std::string& str0) const override 
    {
        if (isarray)
        {
            std::string tmp = ident + "_" + std::to_string(allsymTabCnt);
            (*current_table)[tmp] = std::make_pair(val, 1);
            glob_table[tmp] = std::make_pair(val, 1);

            str0 += "global @";
            str0 += tmp;
            str0 += " = alloc [i32, ";
            str0 += std::to_string(val);
            str0 += "], {";

            if (son.size() == 1)
            {
                for (int i = 0; i < val; i++)
                {
                    if (i)
                        str0 += ", ";
                    str0 += "0";
                }
            }
            else
            {
                int i = 0;
                if (son[1]->son.size())
                {
                    for(; i < son[1]->son[0]->son.size(); i++)
                    {
                        if (i)
                            str0 += ", ";
                        str0 += std::to_string(son[1]->son[0]->son[i]->val);
                    }
                }
                for(; i < val; i++)
                {
                    if (i)
                        str0 += ", ";
                    str0 += "0";
                }
            }
            str0 += "}\n";
        }
    }

    std::string dump2str(std::string &str0) override
    {
        std::string identtmp_num = ident + '_' + std::to_string(allsymTabCnt);
        
        if (isarray)
        {
            (*current_table)[identtmp_num] = std::make_pair(val, 1);

            str0 += " @";
            str0 += identtmp_num;
            str0 += " = alloc [i32, ";
            str0 += std::to_string(val);
            str0 += "]\n";

            std::vector<int> arrayinitnum;
            int i = 0;
            if (son.size() > 1 && son[1]->son.size())
                for(; i < son[1]->son[0]->son.size(); i++)
                    arrayinitnum.push_back(son[1]->son[0]->son[i]->val);
            for(; i < val; i++)
                arrayinitnum.push_back(0);

            for (i = 0; i < val; i++)
            {
                str0 += " \%";
                str0 += std::to_string(expNumCnt);
                str0 += " = getelemptr @";
                str0 += identtmp_num;
                str0 += ", ";
                str0 += std::to_string(i);

                str0 += "\n store ";
                str0 += std::to_string(arrayinitnum[i]);
                str0 += ", \%";
                str0 += std::to_string(expNumCnt++);
                str0 += "\n";
            }
        }

        else
        {
            std::string identtmp = ident + '_' + std::to_string(allsymTabCnt);
            std::cout << "ConstDef: " << ident << std::endl;
            (*current_table)[identtmp] = std::make_pair(constval, 1);
        }
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


class myConstInitVal: public BaseAST {
    public:
    myConstInitVal()    { type = _myConstInitVal;   }
    void Dump(std::string& str0) const override {}
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
        return ident;
    }
};


class VarDecl: public BaseAST {
    public:
    VarDecl()   { type = _VarDecl;  }
    void Dump(std::string &str0) const override {
        if (son[1]->type == _myVarDef)
            son[1]->Dump(str0);
    }

    std::string dump2str(std::string &str0) override 
    {
        son[1]->dump2str(str0);
        return "";
    }
};


class myVarDef: public BaseAST {
    public:
    myVarDef()  { type = _myVarDef; }
    void Dump(std::string& str0) const override 
    {
        for (auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->Dump(str0);
    }

    std::string dump2str(std::string &str0) override
    {
        for(auto iter = son.begin(); iter != son.end(); iter++)
            (*iter)->dump2str(str0);
        return "";
    }
};


class VarDef: public BaseAST {
    public:
    std::string ident;
    int varval;

    VarDef()    { type = _VarDef;   }
    void Dump(std::string& str0) const override 
    {
        if (isarray)
        {
            std::string tmp = ident + "_" + std::to_string(allsymTabCnt);
            (*current_table)[tmp] = std::make_pair(val, 1);
            glob_table[tmp] = std::make_pair(val, 1);

            str0 += "global @";
            str0 += tmp;
            str0 += " = alloc [i32, ";
            str0 += std::to_string(val);
            str0 += "], {";

            if (son.size() == 1)
            {
                for (int i = 0; i < val; i++)
                {
                    if (i)
                        str0 += ", ";
                    str0 += "0";
                }
            }
            else
            {
                int i = 0;
                if (son[1]->son.size())
                {
                    for(; i < son[1]->son[0]->son.size(); i++)
                    {
                        if (i)
                            str0 += ", ";
                        str0 += std::to_string(son[1]->son[0]->son[i]->val);
                    }
                }
                for(; i < val; i++)
                {
                    if (i)
                        str0 += ", ";
                    str0 += "0";
                }
            }
            str0 += "}\n";
        }

        else
        {
            std::string tmp = ident + "_" + std::to_string(allsymTabCnt);
            str0 += "global @";
            str0 += tmp;
            str0 += " = alloc i32, ";

            if (son.size() > 0)
            {
                std::string globalVarVal = son[0]->dump2str(str0);
                str0 += globalVarVal;
                str0 += "\n";
                (*current_table)[tmp] = std::make_pair(son[0]->val, 0);
                glob_table[tmp] = std::make_pair(son[0]->val, 0);
            }
            else
            {
                str0 += "zeroinit\n";
                (*current_table)[tmp] = std::make_pair(0, 0);
                glob_table[tmp] = std::make_pair(0, 0);
            }
        }
    }

    std::string dump2str(std::string& str0) override
    {
        std::string identtmp_num = ident + '_' + std::to_string(allsymTabCnt);
        
        if (isarray)
        {
            (*current_table)[identtmp_num] = std::make_pair(val, 1);

            str0 += " @";
            str0 += identtmp_num;
            str0 += " = alloc [i32, ";
            str0 += std::to_string(val);
            str0 += "]\n";

            std::vector<int> arrayinitnum;
            int i = 0;
            if (son.size() > 1 && son[1]->son.size())
                for(; i < son[1]->son[0]->son.size(); i++)
                    arrayinitnum.push_back(son[1]->son[0]->son[i]->val);
            for(; i < val; i++)
                arrayinitnum.push_back(0);

            for (i = 0; i < val; i++)
            {
                str0 += " \%";
                str0 += std::to_string(expNumCnt);
                str0 += " = getelemptr @";
                str0 += identtmp_num;
                str0 += ", ";
                str0 += std::to_string(i);

                str0 += "\n store ";
                str0 += std::to_string(arrayinitnum[i]);
                str0 += ", \%";
                str0 += std::to_string(expNumCnt++);
                str0 += "\n";
            }
        }

        else
        {
            if (glob_table.find("@"+identtmp_num) == glob_table.end())
            {
                str0 += " @";
                str0 += identtmp_num;
                str0 += " = alloc i32\n";
            }

            if (son.size() > 0)
            {
                std::string tmp = son[0]->dump2str(str0);
                str0 += " store ";
                str0 += tmp;
                str0 += ", @";
                str0 += identtmp_num;
                str0 += "\n";

                (*current_table)[identtmp_num] = std::make_pair(std::atoi(tmp.c_str()), 0);
            }
            else
            {
                str0 += " store ";
                str0 += std::to_string(varval);
                str0 += ", @";
                str0 += identtmp_num;
                str0 += "\n";

                (*current_table)[identtmp_num] = std::make_pair(varval, 0);
            }
        }

        return "";
    }
};


class InitVal: public BaseAST {
    public:
    InitVal()   { type = _InitVal;  }
    void Dump(std::string& str0) const override {}

    std::string dump2str(std::string& str0) override
    {
        return son[0]->dump2str(str0);
    }
};


class myInitVal: public BaseAST {
    public: 
    myInitVal() { type = _myInitVal;    }

    void Dump(std::string &str0) const override {}
};