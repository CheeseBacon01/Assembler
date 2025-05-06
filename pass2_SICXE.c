#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pass1.c"

//SYMTAB linked list
typedef struct sym_node{
    char arr[7];
    int x;
    struct sym_node *next,*prev;
}symNODE;

//modification record
typedef struct{
    int addr;
    int len;
}modified;

symNODE *tail=NULL;
symNODE *symtab=NULL;

//INSERT STMTAB
int flag=1;
symNODE* insert(char *s,int r){
    if(flag==1){
        symtab=(struct sym_node*)malloc(sizeof(struct sym_node));
        tail = (struct sym_node*)malloc(sizeof(struct sym_node));
        symtab->prev=NULL;
        symtab->next=tail;
        tail->prev=symtab;
        tail->next=NULL;
        flag=0;
        if(strlen(s)>0){
            strcpy(symtab->arr,s);
            symtab->x=r;
            return symtab;
        }
    }
    else{
        symNODE *temp=(struct sym_node*)malloc(sizeof(struct sym_node));
        if(strlen(s)>0){
            strcpy(temp->arr,s);
            temp->x=r;
            temp->next=symtab->next;
            temp->prev=symtab;
            symtab->next->prev=temp;
            symtab->next=temp;
            return temp;
        }
    }
    return NULL;
}

//Search SYMTAB
symNODE* search(symNODE *t,char *s){
    if(t){
        if(strcmp(s,t->arr)==0){
            return t;
        }
        else{
            return search(t->next,s);
        }
    }
    else{
        return NULL;
    }
}

// get register for FMT2
int get_register(const char *reg){
    if(strcmp(reg,"A")==0){
        return 0;
    }
    else if(strcmp(reg,"X")==0){
        return 1;
    }
    else if(strcmp(reg,"L")==0){
        return 2;
    }
    else if(strcmp(reg,"B")==0){
        return 3;
    }
    else if(strcmp(reg,"S")==0){
        return 4;
    }
    else if(strcmp(reg,"T")==0){
        return 5;
    }
    else if(strcmp(reg,"F")==0){
        return 6;
    }
    else if(strcmp(reg,"")==0){
        return 0;
    }
    else{
        return -1;
    }
}

int main(int argc, char *argv[]){
    int line_count,c;
    char buf[LEN_SYMBOL];
    LINE line[500];
    modified modified[500];
    int index=0;
    char locctr[7];
    FILE *fptr;
    int start_address=0;
    int error_flag=0;
    int result=0;
    symtab=NULL;
    tail=NULL;

    fptr=fopen("Intermediate File.txt","w");

    if(argc<2){
        printf("Usage: %s fname.asm\n",argv[0]);
    }
    else{
        if(ASM_open(argv[1])==NULL){
            printf("File not found!!\n");
        }
        else{
            for(line_count=1;(c=process_line(&line[line_count]))!=LINE_EOF;line_count++){

                if(strcmp(line[line_count].op,"START")==0&&line_count==1){      //START
                    strcpy(locctr,line[line_count].operand1);
                    start_address=(int)strtol(locctr,NULL,16);          //get start address
                    line[line_count].locctr=start_address;              //store address
                    printf("%06X %-10s %-10s %-10s\n",start_address,line[line_count].symbol,line[line_count].op,line[line_count].operand1);
                    fprintf(fptr,"%06X %-10s %-10s %-10s\n",start_address,line[line_count].symbol,line[line_count].op,line[line_count].operand1);
                }
                else{
                    if(c==-2){          //comment line
                        printf(".\n");
                        fprintf(fptr,".\n");
                        continue;
                    }

                    line[line_count].locctr=start_address;

                    if (strlen(line[line_count].symbol)>0){
                        symNODE *found=search(symtab,line[line_count].symbol);      //search STMTAB
                        if(found!=NULL){
                            printf("Error: Duplicate label '%s'\n",line[line_count].symbol);
                            error_flag = 1;
                        }
                        else{
                            insert(line[line_count].symbol,start_address);
                        }
                    }

                    char mode='\0';
                    if(line[line_count].addressing==ADDR_IMMEDIATE){        //immediate
                        mode='#';
                    }
                    else if(line[line_count].addressing==ADDR_INDIRECT){    //indirect
                        mode='@';
                    }

                    fprintf(fptr,"%06X %-10s",line[line_count].locctr,line[line_count].symbol);
                    printf("%06X %-10s",line[line_count].locctr,line[line_count].symbol);

                    if(line[line_count].fmt==FMT4){             //format 4
                        fprintf(fptr,"+%-10s ",line[line_count].op);
                        printf("+%-10s ",line[line_count].op);
                    }
                    else{
                        fprintf(fptr," %-10s ",line[line_count].op);
                        printf(" %-10s ",line[line_count].op);
                    }
                    if(mode!='\0'){                 //addressing mode
                        fprintf(fptr,"%c%-10s ",mode,line[line_count].operand1);
                        printf("%c%-10s ",mode,line[line_count].operand1);
                    }
                    else{
                        fprintf(fptr,"%-10s",line[line_count].operand1);
                        printf("%-10s",line[line_count].operand1);
                    }
                    if(line[line_count].addressing==9){        //index addressing mode
                        fprintf(fptr,",X ");
                        printf(",X ");
                    }
                    if(line[line_count].operand2[0]!='\0'){
                        fprintf(fptr,",%-10s\n",line[line_count].operand2);
                        printf(",%-10s\n",line[line_count].operand2);
                    }
                    else{
                        fprintf(fptr,"\n");
                        printf("\n");
                    }

                    Instruction *op=is_opcode(line[line_count].op);
                    if(op != NULL){
                        if(line[line_count].fmt==FMT1){
                            start_address+=1;
                        }
                        else if(line[line_count].fmt==FMT2){
                            start_address+=2;
                        } 
                        else if(line[line_count].fmt==FMT3){
                            start_address+=3;
                        } 
                        else if(line[line_count].fmt==FMT4){
                            start_address+=4;
                        }
                        else if(line[line_count].fmt==FMT0){
                            if(strcmp(line[line_count].op,"WORD")==0){
                                start_address += 3;
                            }
                            else if(strcmp(line[line_count].op,"RESW")==0){
                                int words=atoi(line[line_count].operand1);
                                start_address+=3*words;
                            }
                            else if(strcmp(line[line_count].op,"RESB")==0){
                                int bytes=atoi(line[line_count].operand1);
                                start_address+=bytes;
                            }
                            else if(strcmp(line[line_count].op,"BYTE")==0){
                                if(line[line_count].operand1[0]=='C'&&line[line_count].operand1[1]=='\''){
                                    int length=0;
                                    for(int i=2;line[line_count].operand1[i]!='\''&&line[line_count].operand1[i]!='\0';i++){
                                        length++;
                                    }
                                    start_address+=length;
                                }
                                else if(line[line_count].operand1[0]=='X' && line[line_count].operand1[1]=='\''){
                                    int length=0;
                                    for(int i=2;line[line_count].operand1[i]!='\''&&line[line_count].operand1[i]!='\0';i++){
                                        length++;
                                    }
                                    if(length%2==0){
                                        start_address+=length/2;
                                    }
                                    else{
                                        error_flag = 1;
                                    }
                                }
                                else{
                                    error_flag = 1;
                                }
                            }
                        }
                    }
                    else{
                        printf("Error: Unknown operation '%s'\n",line[line_count].op);
                        error_flag=1;
                    }
                }
            }
            result=start_address-line[1].locctr;        //program length
            printf("Program length = %X\n",result);
            fprintf(fptr,"Program length = %X\n",result);
            symNODE *current=symtab;
            while(current!=NULL&&current->next!=NULL){          //print STMTAB
                if(strlen(current->arr)>0){
                    fprintf(fptr,"%10s: %06X\n",current->arr,current->x);
                    printf("%10s: %06X\n",current->arr,current->x);
                }
                current=current->next;
            }
            fclose(fptr);
            printf("\n\n");

            //pass2
            int pos=-1;
            int address=0;
            int j;
            int base_relative=0;
            int base_address=0;
            int start_address=0;

            for (int i=1;i<line_count;i++){
                if(strcmp(line[i].op,"START")==0){              //start
                    printf("H%-06s%06X%06X",line[1].symbol,line[1].locctr,result);      //Head record
                    start_address=line[1].locctr;                             //initial address
                }
                else{
                    if(line[i].fmt==FMT1||line[i].fmt==FMT2||line[i].fmt==FMT3||line[i].fmt==FMT4){
                        if(line[i].fmt==FMT1){           //Format1
                            if(pos==-1){            //handle next line
                                pos=line[i].locctr;
                                start_address=0;
                                for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                printf("\nT%06X%02X",pos,line[j].locctr-pos);
                            }
                            if(start_address+1>30){
                                int j;
                                pos+=start_address;
                                start_address=0;
                                for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                printf("\nT%06X%02X",pos,line[j].locctr-pos);
                            }
                            printf("%02X",line[i].code);
                            start_address+=1;
                        }

                        else if(line[i].fmt==FMT2){             //format 2
                            int reg1=get_register(line[i].operand1);    //register 1
                            int reg2=get_register(line[i].operand2);       //register 2
                            if(reg1==-1||reg2==-1){
                                error_flag=1;
                            }
                            else{
                                if(pos==-1){
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                if(start_address+2>30){
                                    int j;
                                    pos+=start_address;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                printf("%02X%d%d",line[i].code,reg1,reg2);
                                start_address+=2;
                            }
                        }

                        else if(line[i].fmt==FMT3){             //format 3
                            if(strcmp(line[i].op,"RSUB")==0){   //RSUB special case
                                if(pos==-1){
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                if(start_address+3>30){
                                    int j;
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                printf("4F0000");
                                start_address+=3;
                            }
                            else{
                                int operand = 0;
                                int pc_flag = 0;
                                int base_flag = 0;
                                int disp = 0;
                                int SIC = 0;
                        
                                if((line[i].addressing&ADDR_IMMEDIATE)!=0){     //immediate
                                    if(isdigit(line[i].operand1[0])){       //get operand
                                        disp=atoi(line[i].operand1);
                                    }
                                    else{
                                        symNODE *symbol=search(symtab,line[i].operand1);    //search
                                        if(symbol!=NULL){           //if found
                                            disp=symbol->x;
                                            if (disp-line[i+1].locctr>=-2048&&disp-line[i+1].locctr<=2047){ //pc relative
                                                pc_flag=1;
                                                disp=disp-(line[i].locctr+3);
                                            }
                                            else if(base_relative==1&&disp-base_address>=0&&disp-base_address<4096){    //base relative
                                                base_flag=1;
                                                disp=disp-base_address;
                                            }
                                            else if((line[i].addressing&ADDR_SIMPLE)!=0&&disp<32767){       //SIC
                                                SIC=1;
                                            }
                                            else{
                                                error_flag=1;
                                            }
                                        }
                                        else{
                                            error_flag=1;
                                        }
                                    }
                                }
                                else{
                                    symNODE *symbol=search(symtab, line[i].operand1);
                                    if (symbol!=NULL){
                                        disp=symbol->x;
                                        if(disp-line[i+1].locctr>=-2048&&disp-line[i+1].locctr<=2047){
                                            pc_flag = 1;
                                            disp=disp-(line[i].locctr+3);
                                        }
                                        else if(base_relative==1&&disp-base_address>=0&&disp-base_address<4096){
                                            base_flag=1;
                                            disp=disp-base_address;
                                        }
                                        else if((line[i].addressing & ADDR_SIMPLE)!=0&&disp<32767){
                                            SIC=1;
                                        }
                                        else{
                                            error_flag = 1;
                                        }
                                    }
                                    else {
                                        error_flag = 1;
                                    }
                                }
                        
                                int xbpe=0;
                                int op3=0;
                                if((line[i].addressing&ADDR_IMMEDIATE)!=0){ //01
                                    op3=1;
                                }
                                else if((line[i].addressing&ADDR_INDIRECT)!=0){     //10
                                    op3=2;
                                }
                                else if((line[i].addressing&ADDR_SIMPLE)!=0){       //11
                                    op3=3;
                                }
                                else if(SIC==1){                            //00
                                    op3=0;
                                }
                                
                                if(pc_flag==1){     //0010
                                    xbpe=2;
                                }
                                else if(base_flag==1){      //0100
                                    xbpe=4;
                                }
                                else{                   //0000
                                    xbpe=0;
                                }
                                if((line[i].addressing & ADDR_INDEX)!=0){
                                    xbpe+=8;
                                }
                                if (pos==-1){
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                if(start_address+3>30){
                                    int j;
                                    pos+=start_address;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                printf("%02X%X%03X",op3+line[i].code,xbpe,disp&0xFFF);
                                start_address += 3;
                            }
                        }

                        else if(line[i].fmt == FMT4){
                            int disp=0;
                            symNODE *symbol=search(symtab,line[i].operand1);

                            if(symbol!=NULL){   //modified record
                                disp=symbol->x;
                                modified[index].addr=line[i].locctr+1;
                                modified[index].len=5;
                                index++;
                            }
                            else{
                                error_flag=1;
                            }

                            int xbpe=0;
                            int op4=0;

                            if((line[i].addressing&ADDR_IMMEDIATE)!=0){
                                op4=1;      //01
                                xbpe=1;        
                                if (isdigit(line[i].operand1[0])){
                                    disp=atoi(line[i].operand1);
                                }
                            }
                            else if((line[i].addressing&ADDR_INDIRECT)!=0){
                                op4=2;
                                xbpe=1;
                            }
                            else if((line[i].addressing&ADDR_SIMPLE)!=0){
                                op4=3;
                                xbpe=1;
                            }
                            if(line[i].addressing==ADDR_INDEX){
                                xbpe=9;
                            }
                            if(pos==-1){
                                pos=line[i].locctr;
                                start_address=0;
                                for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                printf("\nT%06X%02X",pos,line[j].locctr-pos);
                            }
                            if(start_address+4>30){
                                int j;
                                pos+=start_address;
                                start_address=0;
                                for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                printf("\nT%06X%02X",pos,line[j].locctr-pos);
                            }
                            printf("%03X%05X",(op4+line[i].code)*16+xbpe,disp);
                            start_address+=4;
                        }
                    }
                    else if(line[i].code==OP_RESB||line[i].code==OP_RESW){      //RESB RESW
                        pos=-1;                 //reset,next line
                    }
                    else if(line[i].code==OP_BYTE){                            //BYTE
                        if(line[i].operand1[0]=='C'||line[i].operand1[0]=='c'){
                            for(int k=2;line[i].operand1[k]!='\''&&line[i].operand1[k]!='\0';k++){
                                if(pos==-1){
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                if(start_address+1>30){
                                    int j;
                                    pos+=start_address;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                printf("%02X",line[i].operand1[k]);
                                start_address+=1;
                            }
                        }
                        else if (line[i].operand1[0]=='X'||line[i].operand1[0]=='x'){
                            for(int k=2;line[i].operand1[k]!='\''&&line[i].operand1[k]!='\0';k+=2){
                                if(pos==-1){
                                    pos=line[i].locctr;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                if(start_address+1>30){
                                    int j;
                                    pos+=start_address;
                                    start_address=0;
                                    for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                                    printf("\nT%06X%02X",pos,line[j].locctr-pos);
                                }
                                printf("%c%c",line[i].operand1[k],line[i].operand1[k + 1]);
                                start_address+= 1;
                            }
                        }
                    }
                    else if (line[i].code==OP_WORD){
                        if(pos==-1){
                            pos=line[i].locctr;
                            start_address=0;
                            for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j+1].locctr-pos<=30;j++);
                            printf("\nT%06X%02X",pos,line[j].locctr-pos);
                        }
                        if(start_address+3>30){
                            int j;
                            pos+=line[i].locctr;
                            start_address=0;
                            for(j=i;j<line_count-1&&line[j].code!=OP_RESB&&line[j].code!=OP_RESW&&line[j + 1].locctr-pos<=30;j++);
                            printf("\nT%06X%02X",pos,line[j].locctr-pos);
                        }
                        int word_value=atoi(line[i].operand1);
                        printf("%06X",word_value);
                        start_address+= 3;
                    }
                    else if(line[i].code==OP_BASE){
                        base_relative=1;
                        for(int j=0;j<line_count;j++){
                            symNODE *symbol=search(symtab,line[i].operand1);
                            if(symbol!=NULL){
                                base_address=symbol->x;
                                break;
                            }
                        }
                    }
                    else if(line[i].code==OP_NOBASE){
                        base_relative=0;
                    }
                }
            }

            //Modified
            for(int k=0;k<index;k++){
                printf("\nM%06X%02X",modified[k].addr,modified[k].len);
            }
            //END
            if(line[line_count-1].code==OP_END){
                printf("\nE%06X\n",line[1].locctr);
            }
            else{
                printf("\nE000000\n");
            }
        }
    }
    return 0;
}