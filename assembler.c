#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Inst {
	char name[6];
	char op[7];
	char type;
	char funct[7];
};

struct Inst inst[22] = {
	{"addiu", "001001", 'I',""},
	{"addu", "000000", 'R',"100001"},
	{"and", "000000", 'R',"100100"},
	{"andi", "001100", 'I', ""},
	{"beq","000100", 'I', ""},
	{"bne", "000101", 'I', ""},
	{"j", "000010", 'J', ""},
	{"jal", "000011", 'J', ""},
	{"jr", "000000", 'R',"001000"},
	{"lui", "001111", 'I', ""},
	{"lw", "100011", 'I', ""},
	{"nor", "000000", 'R' , "100111"},
	{"or", "000000", 'R', "100101"},
	{"ori", "001101", 'I', ""},
	{"sltiu", "001011",'I' ,""},
	{"sltu", "000000", 'R', "101011"},
	{"sll", "000000", 'S' ,"000000"},
	{"srl", "000000", 'S', "000010"},
	{"sw", "101011", 'I', ""},
	{"subu", "000000", 'R', "100011"}
};

struct Data {			//data의 symname과 data value와 주소값 저장할 구조체 변수(data부분은 data부분에서 관여) ex) data1, array1
	int* value;
	long address;
	char name[40];
};

struct Text {
	int idx;
	char* d;
	char* s;
	char* t;
	char* im;
	char* j;
};

struct Sym {				//symbol의 이름과 주소값을 저장할 구조체(sym은 text부분만 관여)		ex) main, lab1
	char name[40];
	long address;
};

struct Sym* Symbols;
struct Text t_list[33];
int datasize, textsize;
struct Data d_list[10] = { {0,0,""},{ 0,0,"" }, { 0,0,"" }, { 0,0,"" }, { 0,0,"" }, { 0,0,"" }, { 0,0,"" }, { 0,0,"" },{0,0,""} };		//초기화로 셋업
struct Sym s_list[10] = { {"",0},{"",0},{"",0},{"",0},{"",0},{"",0},{"",0},{"",0} };		//각 symbol의 정보 저장해두는 곳
char* str[100] = { NULL, };		//처음 한번 읽어서 문자를 공백기준으로 끊겨서 저장할 변수
int jfind;
int sym_cnt;
int t_size;
/*
 * You may need the following function
 */
char* NumToBits(unsigned int num, int len) {
	char* bits = (char*)malloc(len);
	int idx = len - 1, _i;

	while (num > 0 && idx >= 0) {
		if (num % 2 == 1) {
			bits[idx--] = '1';
		}
		else {
			bits[idx--] = '0';
		}
		num /= 2;
	}

	for (_i = idx;_i >= 0; _i--) {
		bits[_i] = '0';
	}

	return bits;
}
int ishex_dec(char* str) {			//문자열이 16진수 꼴이면 true return 10진수 꼴이면 false return
	if (str[0] == '0' && str[1] == 'x') return 1;
	else return 0;
}
void remove_endchar(char* str) {		//끝 문자 제거해주는 함수
	int len = strlen(str);
	str[len - 1] = '\0';
}
void delchar(char* str, char tok) {			//토큰 parameter로 받아서 str에서 지우는
	for (int i = 0;i < strlen(str);i++) {
		if (str[i] == tok) {
			for (int j = i;j < strlen(str);j++) {
				str[j] = str[j + 1];
				if (j == strlen(str) - 1) {
					str[j] = '\0';
				}
			}
		}
	}
}
int isInst(char *n){
	int result=0;
	if(strcmp(n,"la")==0) result =1;
	else {
		for(int search=0;search<20;search++){
			if(strcmp(n,inst[search].name)==0){
				result =1;
				break;
			}
		}
	}
	return result;
}
int* DecToBin(int num,int len) {
	int i = len-1;
	int sign = 1;
	int* binary = (int*)malloc(sizeof(int) * len);
	if (num < 0) {
		sign = 0;
		num = num * (-1);
	}
	for(i = len-1;i >= 0;i--){
		if(num>=0){
			binary[i] = num % 2;
			num = num/2;
		}
		else binary[i]=0;
	}		
	
	if (sign == 0) {
		for (i = 0;i < len;i++) {
			(binary[i] == 0) ? (binary[i] = 1) : (binary[i] = 0);
		}
		binary[len - 1] += 1;
		

		for (i = len-1;i >=0;i--) {
			if (binary[i] == 2) {
				binary[i] = 0;
				binary[i - 1] += 1;
			}
			else break;
		}
	}
	return binary;
}

/*
 * read the assmebly code
 */
void read_asm() {
	int tmp, i;
	int j = 0;
	int a = 0;
	int b=0;
	unsigned int address;
	char temp[0x1000] = { 0 };
			//처음 한번 읽어서 문자를 공백기준으로 끊겨서 저장할 변수
	int cnt = 0;		//단어 갯수
	int word_cnt = 0;	//data의 value갯수 ex) array에 word가 3개면 3
	int cur_pc = 0;			//pc의 위치값을 나타내주는 변수
	char lui[33] = { 0 };
	int k=0;
	for (i = 0;scanf("%s", temp) == 1;i++) {			//str배열에 단어별로 저장
		str[i] = (char*)malloc(sizeof(char)*100);
		strcpy(str[i], temp);
		if(strcmp(temp,".text")==0) jfind = i;
		
		cnt = cnt + 1;
	}
	//여기서부터는 읽은 str배열에서부터 data섹션부터 읽어오기부터해서 진행할 것


//Read .data region
address = 0x10000000;
for (i = 0; str[i] != NULL;i++) {
	if (str[i][strlen(str[i]) - 1] == ':') {
		delchar(str[i], ':');
		strcpy(d_list[j].name, str[i]);
		d_list[j].address = address;
	
		for (int k = 1;str[k] != NULL;k = k + 2) {
			if (strcmp(str[i + k], ".word") != 0) {
				word_cnt = (k + 1) / 2;
				
				break;
			}
		}
		address = address + (word_cnt-1) * 4;
		d_list[j].value = (int*)malloc(sizeof(int) * (word_cnt+1));
		
		for (int k = 0;k < word_cnt;k++) {
			if (ishex_dec(str[i + 2 * (k + 1)])) {			//data 값이 16진수이면
				d_list[j].value[k] = strtol(str[i + 2 * (k + 1)], NULL, 16);
			}
			else d_list[j].value[k] = strtol(str[i + 2 * (k + 1)], NULL, 10);		//data 값이 10진수이면	
		}
		
		j++;
	}
	else if (strcmp(str[i], ".text") == 0)
		break;
	else continue;
}

datasize = address - 0x10000000;
i=i+1;
//Read .text region
address = 0x400000;
cur_pc = address;

for (; str[i] != NULL;i++) {
	if (str[i][strlen(str[i]) - 1] == ':') {
		delchar(str[i], ':');
		
		strcpy(s_list[k].name, str[i]);
		s_list[k].address = cur_pc;
		k++;
	
		
	}
	else {
		
		if(isInst(str[i])){

			if (strcmp(str[i], "la") == 0) {
				for (int c = 0;c < 10;c++) {
					if (strcmp(str[i + 2], d_list[c].name) == 0) {
						if (d_list[c].address >= 0x10000) {		//lui 사용할 때
							if (d_list[c].address % 0x10000 == 0) {		//lui만 사용
								
								t_list[a].idx = 20;	
								a = a+1;
									
								break;
							}
							else
							{		//lui랑 ori 둘다 사용
								t_list[a].idx = 21;
												
								cur_pc = cur_pc + 4;
								a = a + 1;
									
								
								break;
							}
						}
					}
				}
			}
			else {
				for(b=0;b<22;b++){
					if (strcmp(str[i], inst[b].name) == 0) {
					t_list[a].idx = b;
					a++;	
					break;	
					}
				}
			}			
			cur_pc = cur_pc +4;
		}

	}

}
t_size = a;
sym_cnt = k;
textsize = cur_pc - 0x400000;
}
//여기까지가 주소값저장과 명령어개수 파악 완료 두번 읽으면서
//이제 해야할 일은 숫자로 변환 하는것
/*
  convert the assembly code to num

void subst_asm_to_num() {
	struct Text* text;
	struct Sym* sym;

	for (text = Texts->next; text != NULL; text = text->next) {
		/* blank */

		/*
		 * print the results of assemble
		 */
void print_bits() {
	// print the header

	char temp[0x1000] = { 0 };
	char* fstr[100] = { NULL, };		//처음 한번 읽어서 문자를 공백기준으로 끊겨서 저장할 변수
	
	int cnt = 0;		//단어 갯수
	int i = 0;	
	int j=0;
	int d =0;
	int index=0;
	char s[5][30] = { 0, };		//
	int wordcount = 0;
	int z = 0;
	int forprint[17] = {0,0,0,0,0,0,0,0,0,};
	int pc = 0x4000000;
	int printadd = 0;
	int luiadd = 0;
	int oriadd = 0;
	printf("%s", NumToBits(textsize, 32));
	printf("%s", NumToBits(datasize, 32));
	
	for(int j=jfind+2;str[j] != NULL ;j++){
	
		
			fstr[i] = (char*)malloc(sizeof(char)*100);
			strcpy(fstr[i], str[j]);
			cnt = cnt + 1;
			i=i+1;		//str의 단어갯수 세기(text부분의 sym제외한 단어수 세기)
			
			
	}
	i=0;
	while (fstr[i] != NULL) {
		
		for (j = 0;j < t_size;j++) {
			
			if(inst[t_list[j].idx].type == '\0')
			{
				
				if (t_list[j].idx == 20) {
					for (int c = 0;c < (datasize)/4 ;c++) {
						if (strcmp(fstr[i + 2], d_list[c].name) == 0) {
										//lui만 사용			
							printf("001111");						
							printf("%s", "00000");
							delchar(fstr[i + 1], '$');
							delchar(fstr[i + 1], ',');
							printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
							luiadd = d_list[c].address / 0x10000;
							printf("%s", NumToBits(luiadd, 16));
							pc = pc + 4;
							i = i + 3;
						
						}
					break;
							
						
					}
				}	//else if 20
				else if(t_list[j].idx == 21){		//lui랑 ori 둘다 사용
					
					for (int c = 0;c < (datasize)/4 ;c++){
						if (strcmp(fstr[i + 2], d_list[c].name) == 0){
						
							
							luiadd = d_list[c].address / 0x10000;
							oriadd = d_list[c].address % 0x10000;
							printf("001111");						
							printf("%s", "00000");
							delchar(fstr[i + 1], '$');
							delchar(fstr[i + 1], ',');
							printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
							printf("%s", NumToBits(luiadd, 16));
							
							printf("001101");
							printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
							printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
							printf("%s", NumToBits(oriadd, 16));
													
							i=i+3;
							pc =pc + 8;
							break;
						}
					
					}
				}//else if 21
			}
			else if (inst[t_list[j].idx].type == 'R') {
				if(strcmp(fstr[i],"jr") == 0){
					printf("%s", inst[t_list[j].idx].op);
					delchar(fstr[i+1],'$');
					printf("%s",NumToBits(atoi(fstr[i+1]),5));
					printf("%s",NumToBits(0,15));
					printf("%s", inst[t_list[j].idx].funct);
					i=i+2;
				        pc =pc+4;
					for(int e=0;e<7;e++){					
						if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}
					}
				}
				else{	
					printf("%s", inst[t_list[j].idx].op);
					delchar(fstr[i + 2], '$');
					delchar(fstr[i + 2], ',');
					printf("%s", NumToBits(atoi(fstr[i + 2]), 5));	//rs
					delchar(fstr[i + 3], '$');
					printf("%s", NumToBits(atoi(fstr[i + 3]), 5));	//rt
					delchar(fstr[i + 1], '$');
					delchar(fstr[i + 1], ',');
					printf("%s", NumToBits(atoi(fstr[i + 1]), 5));	//rd
					printf("00000");				//shamt
					printf("%s", inst[t_list[j].idx].funct);		//funct
					i = i + 4;
					for(int e=0;e<7;e++){					
						if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						}
					}
					pc = pc + 4;
				}			
			}
			else if (inst[t_list[j].idx].type == 'I') {
				printf("%s", inst[t_list[j].idx].op);
				if (strcmp(fstr[i], "lw") == 0) {
					char* token;
					token = strtok(fstr[i+2],",()$");
					strcpy(s[z],token);
					z++;
					token = strtok(NULL, ",()$");		
					while (token != NULL) {
						strcpy(fstr[i+2],token);
						token = strtok(NULL, ",()$");
					}
					delchar(fstr[i+1],'$');
					delchar(fstr[i+1],',');	
					printf("%s", NumToBits(atoi(fstr[i+2]), 5));		//rs
					printf("%s", NumToBits(atoi(fstr[i+1]), 5));		//rt
					for (int t = 0;t < 16;t++)
						printf("%d", DecToBin(atoi(s[0]), 16)[t]);		//괄호 앞 수 im
					i = i + 3;
					for(int e=0;e<7;e++){					
						/*if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}*/
					}
					pc = pc + 4;
					z=0;
				}
				else if (strcmp(fstr[i], "sw") == 0) {
					char* token;
					token = strtok(fstr[i+2],",()$");
					strcpy(s[z],token);
					z++;
					token = strtok(NULL, ",()$");		
					while (token != NULL) {
						strcpy(fstr[i+2],token);
						token = strtok(NULL, ",()$");
					}
					
					delchar(fstr[i+1],'$');
					delchar(fstr[i+1],',');
					printf("%s", NumToBits(atoi(fstr[i+2]), 5));		//rs
					printf("%s", NumToBits(atoi(fstr[i+1]), 5));		//rt
					for (int t = 0;t < 16;t++)
						printf("%d", DecToBin(atoi(s[0]), 16)[t]);		//괄호 앞 수 im
					i = i + 3;
					for(int e=0;e<7;e++){					
						if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}
					}
					pc = pc + 4;
					z=0;
				}
				else if (strcmp(fstr[i], "lui") == 0) {

					printf("%s", "00000");
					delchar(fstr[i + 1], '$');
					delchar(fstr[i + 1], ',');
					printf("%s", NumToBits(atoi(fstr[i + 1]), 5));

					if (ishex_dec(fstr[i + 2])) {
						printf("%s", NumToBits(strtol(fstr[i + 2], NULL, 16), 16));
					}
					else printf("%s", NumToBits(atoi(fstr[i + 2]), 16));
					i = i + 3;
					for(int e=0;e<7;e++){					
						/*if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}*/
					}
					pc = pc + 4;
				}
				else if (strcmp(fstr[i], "beq") == 0) {
					delchar(fstr[i + 1], '$');
					delchar(fstr[i + 1], ',');
					printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
					delchar(fstr[i + 2], '$');
					delchar(fstr[i + 2], ',');
					printf("%s", NumToBits(atoi(fstr[i + 2]), 5));
					for (int h = 0;h < 10;h++) {
						if (strcmp(fstr[i + 3], s_list[h].name) == 0) {
							printadd = s_list[h].address - (pc+4);
							break;
						}
					}
					for (int t = 0;t < 16;t++) {
						printf("%d", DecToBin(printadd/4, 16)[t]);
					}

					i = i + 4;
					for(int e=0;e<7;e++){					
						if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}
					}
					
					pc = pc + 4;
				}
				else if (strcmp(fstr[i], "bne") == 0) {
					delchar(fstr[i + 1], '$');
					delchar(fstr[i + 1], ',');
					printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
					delchar(fstr[i + 2], '$');
					delchar(fstr[i + 2], ',');
					printf("%s", NumToBits(atoi(fstr[i + 2]), 5));
					for (int h = 0;h < 10;h++) {
						if (strcmp(fstr[i + 3], s_list[h].name) == 0) {
							printadd = s_list[h].address - (pc + 4);
							break;
						}
					}
					for (int t = 0;t < 16;t++) {
						printf("%d", DecToBin(printadd/4, 16)[t]);
					}
					i = i + 4;
					for(int e=0;e<7;e++){					
						if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}
					}
					pc = pc + 4;
				}
				else if(strcmp(fstr[i],"ori") == 0){
					delchar(fstr[i+2],'$');
					delchar(fstr[i+2],',');
					
					printf("%s",NumToBits(atoi(fstr[i+2]),5));
					delchar(fstr[i+1],'$');
					delchar(fstr[i+1],',');
					
					printf("%s",NumToBits(atoi(fstr[i+1]),5));
					if (ishex_dec(fstr[i + 3])) {
						
						printf("%s", NumToBits(strtol(fstr[i + 3], NULL, 16), 16));
					}
					else printf("%s", NumToBits(atoi(fstr[i + 3]), 16));
					
					i = i + 4;
					for(int e=0;e<7;e++){					
						/*if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}*/
					}
					
					pc = pc + 4;
					
				}
				else {	
										
					delchar(fstr[i + 2],'$');
					delchar(fstr[i + 2],',');
					printf("%s",NumToBits(atoi(fstr[i+2]),5));	//rs
					delchar(fstr[i + 1], '$');
					delchar(fstr[i + 1], ',');
					printf("%s", NumToBits(atoi(fstr[i + 1]), 5));	//rt
					if (ishex_dec(fstr[i + 3])) {
						printf("%s", NumToBits(strtol(fstr[i + 3], NULL, 16), 16));
					}
					else printf("%s", NumToBits(atoi(fstr[i + 3]), 16));
					i = i + 4;
					for(int e=0;e<7;e++){					
						/*if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
						
						}*/
					}
					pc = pc + 4;
				}
			}
			else if (inst[t_list[j].idx].type == 'J') {
				
				printf("%s", inst[t_list[j].idx].op);
				for (int t = 0;t < 10;t++) {
					if (strcmp(fstr[i + 1], s_list[t].name) == 0) {	
						printf("%s", NumToBits(s_list[t].address/4, 26));	
					}
				}
				i = i + 2;
				for(int e=0;e<7;e++){					
					if(strcmp(fstr[i],s_list[e].name)==0) {
					i++;
						
					}
				}
				pc = pc + 4;
			}//else if J
			else if (inst[t_list[j].idx].type == 'S') {
				
				printf("%s", inst[t_list[j].idx].op);
				printf("00000");				//rs
				delchar(fstr[i + 2], '$');
				delchar(fstr[i + 2], ',');
				printf("%s", NumToBits(atoi(fstr[i + 2]), 5));		//rt
				delchar(fstr[i + 1], '$');
				delchar(fstr[i + 1], ',');
				printf("%s", NumToBits(atoi(fstr[i + 1]), 5));			//rd
				printf("%s", NumToBits(atoi(fstr[i + 3]), 5));		//shamt
				printf("%s", inst[t_list[j].idx].funct);			//funct
				i = i + 4;
				for(int e=0;e<7;e++){					
					if(strcmp(fstr[i],s_list[e].name)==0) {
						i++;
					}
				} 
				pc = pc + 4;
			}//else if S
			/*else if (t_list[j].idx == 20) {
				for (int c = 0;c < (datasize)/4 ;c++) {
					if (strcmp(fstr[i + 2], d_list[c].name) == 0) {
									//lui만 사용			
						printf("001111");						
						printf("%s", "00000");
						delchar(fstr[i + 1], '$');
						delchar(fstr[i + 1], ',');
						printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
						luiadd = d_list[c].address / 0x10000;
						printf("%s", NumToBits(luiadd, 16));
						pc = pc + 4;
						i = i + 3;
						
					}
						break;
							
					
				}
			}	//else if 20
			else if(t_list[j].idx == 21){		//lui랑 ori 둘다 사용
				for (int c = 0;c < (datasize)/4 ;c++){
					if (strcmp(fstr[i + 2], d_list[c].name) == 0){
						
						
						luiadd = d_list[c].address / 0x10000;
						oriadd = d_list[c].address % 0x10000;
						printf("001111");						
						printf("%s", "00000");
						delchar(fstr[i + 1], '$');
						delchar(fstr[i + 1], ',');
						printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
						printf("%s", NumToBits(luiadd, 16));
						
						printf("001101");
						printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
						printf("%s", NumToBits(atoi(fstr[i + 1]), 5));
						printf("%s", NumToBits(oriadd, 16));
												
						i=i+3;
						pc =pc + 8;
						break;
					}
					
				}
			}//else if 21*/
						
			
						
		}

	}
	for (int c = 0;c < 8;c++) {
		
		if(strcmp(d_list[c].name,"")!= 0){
			while (d_list[c].value[d] != '\0') {
				
				printf("%s", NumToBits(d_list[c].value[d], 32));
				d = d + 1;
			}
			d=0;
		}
	
	}
	printf("\n");
}

/*
 * main function
 */
int main(int argc, char* argv[]) {

	if (argc != 2) {

		printf("Usage: ./assembler <assembly file>\n");
		exit(0);

	}
	else {

		// reading the input file
		char* filename = (char*)malloc(strlen(argv[1]) + 3);
		strncpy(filename, argv[1], strlen(argv[1]));

		if (freopen(filename, "r", stdin) == 0) {
			printf("File open Error\n");
			exit(1);
		}
		

		// creating the output file (*.o)
		
		filename[strlen(filename) - 1] = 'o';
		freopen(filename, "w", stdout);
		// Let's complete the below functions!
		read_asm();
		print_bits();

		//S freeing the memory
		free(filename);
	}
	return 0;
}
