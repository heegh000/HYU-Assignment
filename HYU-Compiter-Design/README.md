# 컴파일러 설계

2022년 2학기에 수강한 컴파일러 설계 과제입니다.

C-Minus 컴파일러의 프론트엔드(Scanner, Parser, Semantic Analyzer) 을 구현하는 과제입니다.

Scanner
---
input으로 주어진 소스 코드를 읽고, tokenize한 후 token을 return하는 프로그램입니다.

C code와 flex 2가지 방법으로 구현하였습니다.

구현에 대한 자세한 내용은 [Report#1](https://github.com/heegh000/HYU-Compiler-Design/blob/main/1_Scanner/Project%231_Scanner_Report.pdf)에 작성되어 있습니다.

Parser
---
Scanner 중 flex로 구현한 코드를 이용하여 input으로 주어진 소스 코드를 읽고 tokenize한 후, Parser가 읽어들여 AST(Abstract Syntax Tree)를 return하는 프로그램입니다.

Yacc를 이용하여 C-Mius의 BNF Grammer를 맞게끔 AST를 만듭니다.

구현에 대한 자세한 내용은 [Report#2](https://github.com/heegh000/HYU-Compiler-Design/blob/main/2_Parser/Project%232_Parser_Report.pdf)에 작성되어 있습니다.

Semaintic Analyzer
---
Parser를 통해 생성된 AST를 통해 symbol table과 type checker를 만들어 모든 Semantic Error를 찾아내는 프로그램입니다.

Tiny 컴파일러에는 존재하지 않는 scope 개념을 적용해 symbol table을 구성하고 type checker를 만들었습니다.

구현에 대한 자세한 내용은 [Report#3](https://github.com/heegh000/HYU-Compiler-Design/blob/main/3_Semantic/Project%233_Semantic_Report.pdf)에 작성되어 있습니다.
