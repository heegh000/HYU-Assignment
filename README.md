# 컴퓨터구조론

2022년 1학기에 수강한 컴퓨터구조론 과제입니다. 

LC-2K 구조에 맞게 assembler와 simulator를 작성하는 과제입니다.

Assembler
---
LC-2K assembly language를 읽고 machine language로 번역하여 파일로 출력하는 프로그램입니다. 

input 파일은 .as, 출력파일은 .mc입니다


Simulator
---
machine language를 읽고, CPU, memory가 존재한다고 가정하여 machine laguage를 수행하는 프로그램입니다.

LC-2K
---

- 8개의 레지스터 (reg0는 항상 0)
- 32-bit 구조
- 65536 word-memory (word addresses)
- 4개의 Instruction format
    1. R type (add, nor)
        - bits 24-22: opcode
        - bits 21-19: reg A
        - bits 18-16: reg B
        - bits 15-3: unused
        - bits 2-0: destReg
    2. I type(lw, sw, beq)
        - bits 24-22: opcode
        - bits 21-19: reg A
        - bits 18-16: reg B
        - bits 15-0: offset
    3. J type (jalr)
        - bits 24-22: opcode
        - bits 21-19: reg A
        - bits 18-16: reg B
        - bits 15-0: unused
    4. O type(halt, noop)
        - bits 24-22: opcode
        - bits 21-0: unused
