# 北京大学《编译原理》22秋课程项目

项目实现了一个从 SysY 到 RiscV 的简易编译器，实现分为两个阶段：从 SysY 到 KoopaIR 语言, 以及从 KoopaIR 到RiscV. 编译器能够支持表达式, 函数, 变量和常量, 一维数组, 分支循环, 语句块等基本语法.

## 基于 Makefile 的 SysY 编译器项目模板

感谢来自[pku-minic](https://github.com/pku-minic/sysy-make-template)的编译器项目模板.

该仓库中存放了一个基于 Makefile 的 SysY 编译器项目的模板, 本项目在该模板的基础上进行进一步的开发.

## 运行方式

``` shell
build/compiler -koopa 输入的 SysY 文件 -o 输出的 Koopa 文件
build/compiler -riscv 输入的 SysY 文件 -o 输出的 RiscV 文件
```