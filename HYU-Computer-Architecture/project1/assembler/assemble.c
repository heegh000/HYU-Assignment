/* Assembler code fragment for LC-2K */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINELENGTH 1000
#define MAXLINES 51

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int get_label_addr (char*);

typedef struct { 
    char label[MAXLINELENGTH]; 
    char opcode[MAXLINELENGTH]; 
    char arg0[MAXLINELENGTH];
    char arg1[MAXLINELENGTH]; 
    char arg2[MAXLINELENGTH];
} instruction;

instruction inst[MAXLINES];
int count = 0;

int main(int argc, char *argv[]) {    
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;

    int i = 0;
    
    int result = 0;
    
    int opcode, reg0, reg1, dest, offset;
	char check[2];
    check[1] = '\0';

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];
 
    inFilePtr = fopen(inFileString, "r");
    
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    while (readAndParse(inFilePtr, inst[count].label, inst[count].opcode, 
            inst[count].arg0, inst[count].arg1, inst[count].arg2) ) {
        //printf("label: %s op: %s arg0: %s arg1: %s arg2: %s\n", 
        //        inst[count].label, inst[count].opcode, inst[count].arg0, inst[count].arg1, inst[count].arg2);
        if(get_label_addr(inst[count].label) != -1) {
            printf("error duplicate labels %s\n", inst[count].label);
            exit(1);
        }        
		
		check[0] = inst[count].label[0];

        if(strlen(inst[count].label) > 6 ) {
            printf("error undefined labels(out of length) %s\n", inst[count].label);
            exit(1);
        }

        if(isNumber(check)) {
            printf("error undefined labels(start with label) %s\n", inst[count].label);
            exit(1);
        }

        count++;
    }

    for(i = 0; i < count; i++) {
        
        result = 0;
        
        if(!strcmp(inst[i].opcode, "add")) { opcode = 0; }
        else if(!strcmp(inst[i].opcode, "nor")) { opcode = 1; }
        else if(!strcmp(inst[i].opcode, "lw")) { opcode = 2; }    
        else if(!strcmp(inst[i].opcode, "sw")) { opcode = 3; }
        else if(!strcmp(inst[i].opcode, "beq")) { opcode = 4; }    
        else if(!strcmp(inst[i].opcode, "jalr")) { opcode = 5; }    
        else if(!strcmp(inst[i].opcode, "halt")) { opcode = 6; }    
        else if(!strcmp(inst[i].opcode, "noop")) { opcode = 7; }    
        else if(!strcmp(inst[i].opcode, ".fill")) { opcode = 8; }
        else { 
            printf("error unrecongnized opcode %s\n", inst[i].opcode);
            exit(1); 
        }
        
        if(opcode == 6 || opcode == 7) {
            result = result | (opcode << 22);
        }

        else if(opcode == 8) {    
            if(isNumber(inst[i].arg0))
                result = atoi(inst[i].arg0);
            else {
                if((result = get_label_addr(inst[i].arg0)) < 0) {
                    printf("error undefined label %s\n", inst[i].arg0);
                    exit(1);
                }
            }
        }    
        else {
            result = result | (opcode << 22);
            
            if(!isNumber(inst[i].arg0)) {
                printf("error regA is non number %s\n", inst[i].arg0);
                exit(1);
            }
            if(!isNumber(inst[i].arg1)) {
                printf("error regB is non number %s\n", inst[i].arg1);
                exit(1);
            }

            reg0 = atoi(inst[i].arg0);
            reg1 = atoi(inst[i].arg1);
                
            if(reg0 < 0 || reg0 > 7) {
                printf("error regA is out of bound %d\n", reg0);
                exit(1);
            }
            if(reg1 < 0 || reg1 > 7) {
                printf("error regB is out of bound %d\n", reg1);
                exit(1);
            }
                
    
            result = result | (reg0 << 19);
            result = result | (reg1 << 16);

            //R-type
            if(opcode == 0 || opcode == 1) {
                if(!isNumber(inst[i].arg2)) {
                    printf("error dest is non number %s\n", inst[i].arg2);
                    exit(1);
                }    
                dest = atoi(inst[i].arg2);
                
                if(dest < 0 || dest > 7) {
                    printf("error dest is out of bound %d\n", dest);
                    exit(1);
                }
                result = result | dest;    
            }
            //I-type
            else if(opcode == 2 || opcode == 3 || opcode == 4) {
                if(isNumber(inst[i].arg2)) {
                    offset = atoi(inst[i].arg2); 
                }
                else {
                    if((offset = get_label_addr(inst[i].arg2)) < 0) {
                        printf("error undefined label %s\n", inst[i].arg2);
                        exit(1);
                    }
                    
                    if(opcode == 4)
                        offset = offset - (i + 1);                    

                }
                
                if(offset < -32768 || offset > 32767) {
                    printf("error offsetFields that don't fit in 16 bits %d\n", offset);
                    exit(1);
                }

                offset = offset & 0xffff;
                result = result | offset;
            }
        }    
    
    
        fprintf(outFilePtr, "%d\n", result);
    }


    return(0);
}/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2) {    
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

/* return 1 if string is a number */
int isNumber(char *string) {    
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int get_label_addr(char *string) {
    int i;

    for(i = 0; i < count; i++) {
        if(inst[i].label[0] == 0)
            continue;
        if(strcmp(inst[i].label, string) == 0)
            return i;
    }
    
    return -1;

}
