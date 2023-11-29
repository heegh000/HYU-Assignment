/* LC-2K Instruction-level simulator */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define OPCODE (7 << 22)
#define REG1 (7 << 19)
#define REG2 (7 << 16)
#define DEST 7
#define OFFSET 0xffff

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

int main(int argc, char *argv[]) {    
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    
    int i, count = 0;
    int inst;
    int opcode;
    int reg1, reg2, dest, offset;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
        
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
                printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    for(i = 0; i <NUMREGS; i++)
        state.reg[i] = 0;
    
    while(1) {
        
        printState(&state);
        
        inst = state.mem[state.pc];

        state.pc ++;
        count ++;

        if(count > 200) {
            printf("error infinite loop\n");
            exit(1);
        }

        opcode = inst & OPCODE;
        opcode = opcode >> 22;

        reg1 = inst & REG1;
        reg1 = reg1 >> 19;

        reg2 = inst & REG2;
        reg2 = reg2 >> 16;

        //R-type
        if(opcode == 0 || opcode == 1) {
            dest = inst & DEST;
            
            if(opcode == 0)
                state.reg[dest] = state.reg[reg1] + state.reg[reg2];
            else
                state.reg[dest] = ~(state.reg[reg1] | state.reg[reg2]);
        }

        //I-type
        else if(opcode == 2 || opcode == 3 || opcode == 4) {
            offset = inst & OFFSET;
                
            if(offset > 0x7fff)
                offset = offset | (OFFSET << 16);
            
            if(opcode == 2) {
                state.reg[reg2] = state.mem[state.reg[reg1] + offset];
            }
            else if(opcode == 3) {
                state.mem[state.reg[reg1] + offset] = state.reg[reg2];
            
            }
            else {
                if(state.reg[reg1] == state.reg[reg2])
                    state.pc = state.pc + offset;
                    if(state.pc < 0) {
                        printf("error branching to negative address\n");
                        exit(1);
                    }
            }
        }
        //J-type
        else if(opcode == 5) {
            state.reg[reg2] = state.pc;
            state.pc = state.reg[reg1];
			
            if(state.pc < 0) {
                printf("error branching to negative address\n");
                exit(1);
            }

        }
        //halt
        else if(opcode == 6) {
            printf("\nmachine halted\ntotal of %d instrcutions executed\nfinal state of machine:\n", count);
            printState(&state);
            break;
        }
        //noop
        else if(opcode == 7)
            continue;
        

    }
    return(0);
}


void printState(stateType *statePtr) {    
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}
