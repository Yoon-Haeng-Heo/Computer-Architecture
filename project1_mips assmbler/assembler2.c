#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Inst {
	char* name;
	char* op;
	char type;
	char* funct;
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
	{"jr", "000000", 'J',"001000"},
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
struct Data d_list[10] = { {0,0," "},{ 0,0,"" }, { 0,0,"" }, { 0,0,"" }, { 0,0," " }, { 0,0," " }, { 0,0," " }, { 0,0," " },{0,0," "} };		//초기화로 셋업
struct Sym s_list[10] = { {"main",0},{"lab1",0},{"lab2",0},{"lab3",0},{"lab4",0},{"lab5",0},{"loop",0},{"exit",0} };		//각 symbol의 정보 저장해두는 곳

/*
 * You may need the following function
 */
char* NumToBits(unsigned int num, int len) {
	char* bits = (char*)malloc(len + 1);
	int idx = len - 1, i;

	while (num > 0 && idx >= 0) {
		if (num % 2 == 1) {
			bits[idx--] = '1';
		}
		else {
			bits[idx--] = '0';
		}
		num /= 2;
	}

	for (i = idx; i >= 0; i--) {
		bits[i] = '0';
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
int* DecToBin(int num,int len) {
	int i = 0;
	int sign = 1;
	int* binary = (int*)malloc(sizeof(int) * len);

	if (num < 0) {
		sign = 0;
		num = num * (-1);
	}
	for (i = 0;i < len;i++) {
		binary[i] = num % 2;
		num = num >> 1;
	}
	if (sign == 0) {
		for (i = 0;i < len;i++) {
			(binary[i] == 0) ? (binary[i] = 1) : (binary[i] = 0);
		}
		binary[0] += 1;

		for (i = 0;i < len-1;i++) {
			if (binary[i] == 2) {
				binary[i] = 0;
				binary[i + 1] += 1;
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
	unsigned int address;
	char temp[0x1000] = { 0 };
	char* str[100] = { NULL, };		//처음 한번 읽어서 문자를 공백기준으로 끊겨서 저장할 변수
	int cnt = 0;		//단어 갯수
	int word_cnt = 0;	//data의 value갯수 ex) array에 word가 3개면 3
	int cur_pc = 0;			//pc의 위치값을 나타내주는 변수
	char lui[33] = { 0 };

	for (int i = 0;scanf("%s", temp) == 1;i++) {			//str배열에 단어별로 저장
		strcpy(str[i], temp);
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
		address = address + word_cnt * 4;
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

//Read .text region
address = 0x400000;
cur_pc = address;
for (i = 0; str[i] != NULL;i++) {
	if (str[i][strlen(str[i]) - 1] == ':') {
		delchar(str[i], ':');
		for (int k = 0;k < 10;k++) {
			if (strcmp(s_list[k].name, str[i]) == 0) {
				s_list[k].address = cur_pc;
				
			}
		}
	}
	else {
		for (int b = 0;b < 22;b++) {
			if (strcmp(str[i], "la") == 0) {
				for (int c = 0;c < 10;c++) {
					if (strcmp(str[i + 2], d_list[c].name) == 0) {
						if (d_list[c].address >= 0x10000) {		//lui 사용할 때
							if (d_list[c].address % 0x10000 == 0) {		//lui만 사용
								
								cur_pc = cur_pc + 4;
								a++;
								b = 0;
							}
							else
							{		//lui랑 ori 둘다 사용
								
								cur_pc = cur_pc + 8;
								a = a + 2;
								b = 0;
							}
						}
					}
				}
			}
			else if (strcmp(str[i], inst[b].name) == 0) {
				t_list[a].idx = b;
				a++;
				cur_pc = cur_pc + 4;
			}

		}

	}
}


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
	char* str[100] = { NULL, };
	int cnt = 0;		//단어 갯수
	int i = 0;
	int line = 0;
	char *s[5] = { 0, };		//
	int wordcount = 0;
	int z = 0;
	int forprint[17] = {0,0,0,0,0,0,0,0,0,};
	int pc = 0x4000000;
	int printadd = 0;
	int luiadd = 0;
	int oriadd = 0;
	printf("%s", NumToBits(textsize, 32));
	printf("%s", NumToBits(datasize, 32));
	for (int j = 0;scanf("%s", temp) == 1;j++) {
		strcpy(fstr[j], temp);
	}
	for (int j = 0;scanf("%s", temp) == 1;j++) {			//str배열에 단어별로 저장
		if (strcmp(temp, ".text") == 0) {
			wordcount = j;
		}
	}
	for(int j=wordcount; ;j++){
		if (fstr[j][strlen(fstr[j]) - 1] != ':') {
			strcpy(str[i], fstr[j]);
			cnt = cnt + 1;		//str의 단어갯수 세기(text부분의 sym제외한 단어수 세기)
		}	
	}
	while (str[i] != NULL) {
		for (int j = 0;j < 33;j++) {
			if (inst[t_list[j].idx].type == 'R') {
				printf("%s", inst[t_list[j].idx].op);
				delchar(str[i + 2], '$');
				delchar(str[i + 2], ',');
				printf("%s", NumToBits(atoi(str[i + 2]), 5));	//rs
				delchar(str[i + 3], '$');
				delchar(str[i + 3], ',');
				printf("%s", NumToBits(atoi(str[i + 3]), 5));	//rt
				delchar(str[i + 1], '$');
				printf("%s", NumToBits(atoi(str[i + 1]), 5));	//rd
				printf("00000");								//shamt
				printf("%s", inst[t_list[j].idx].funct);		//funct
				i = i + 4;
				line = line + 1;
				pc = pc + 4;
			}
			else if (inst[t_list[j].idx].type == 'I') {
				printf("%s", inst[t_list[j].idx].op);
				if (strcmp(str[i], "lw") == 0) {
					while (str[i] != NULL) {
						strcpy(s[z], str[i]);
						z++;
						str[i] = strtok(NULL, ",()$");
					}
					printf("%s", NumToBits(atoi(s[2]), 5));		//rs
					printf("%s", NumToBits(atoi(s[0]), 5));		//rt
					for (int t = 0;t < 16;t++)
						printf("%d", DecToBin(atoi(s[1]), 16)[t]);		//괄호 앞 수 im
					i = i + 3;
					line = line + 1;
					pc = pc + 4;
				}
				else if (strcmp(str[i], "sw") == 0) {
					while (str[i] != NULL) {
						strcpy(s[z], str[i]);
						z++;
						str[i] = strtok(NULL, ",()$");
					}
					printf("%s", NumToBits(atoi(s[2]), 5));		//rs
					printf("%s", NumToBits(atoi(s[0]), 5));		//rt
					for (int t = 0;t < 16;t++)
						printf("%d", DecToBin(atoi(s[1]), 16)[t]);		//괄호 앞 수 im
					i = i + 3;
					line = line + 1;
					pc = pc + 4;
				}
				else if (strcmp(str[i], "lui") == 0) {

					printf("%s", "00000");
					delchar(str[i + 1], '$');
					delchar(str[i + 1], ',');
					printf("%s", NumToBits(atoi(str[i + 1]), 5));

					if (ishex_dec(str[i + 2])) {
						printf("%s", NumToBits(strtol(str[i + 2], NULL, 16), 16));
					}
					else printf("%s", NumToBits(atoi(str[i + 2]), 16));
					i = i + 3;
					line = line + 1;
					pc = pc + 4;
				}
				else if (strcmp(str[i], "beq") == 0) {
					delchar(str[i + 1], '$');
					delchar(str[i + 1], ',');
					printf("%s", NumToBits(atoi(str[i + 1]), 5));
					delchar(str[i + 2], '$');
					delchar(str[i + 2], ',');
					printf("%s", NumToBits(atoi(str[i + 2]), 5));
					for (int h = 0;h < 10;h++) {
						if (strcmp(str[i + 3], s_list[h].name) == 0) {
							printadd = s_list[h].address - (pc + 4);
							break;
						}
					}
					for (int t = 0;t < 16;t++) {
						printf("%d", DecToBin(printadd, 16)[t]);
					}
					i = i + 4;
					pc = pc + 4;
				}
				else if (strcmp(str[i], "bne") == 0) {
					delchar(str[i + 1], '$');
					delchar(str[i + 1], ',');
					printf("%s", NumToBits(atoi(str[i + 1]), 5));
					delchar(str[i + 2], '$');
					delchar(str[i + 2], ',');
					printf("%s", NumToBits(atoi(str[i + 2]), 5));
					for (int h = 0;h < 10;h++) {
						if (strcmp(str[i + 3], s_list[h].name) == 0) {
							printadd = s_list[h].address - (pc + 4);
							break;
						}
					}
					for (int t = 0;t < 16;t++) {
						printf("%d", DecToBin(printadd, 16)[t]);
					}
					i = i + 4;
					pc = pc + 4;
				}
				else {
					delchar(str[i + 2], '$');
					delchar(str[i + 2], ',');
					printf("%s", NumToBits(atoi(str[i + 2]), 5));	//rs
					delchar(str[i + 1], '$');
					delchar(str[i + 1], ',');
					printf("%s", NumToBits(atoi(str[i + 1]), 5));	//rt
					if (ishex_dec(str[i + 3])) {
						printf("%s", NumToBits(strtol(str[i + 3], NULL, 16), 16));
					}
					else printf("%s", NumToBits(atoi(str[i + 3]), 16));
					i = i + 4;
					line = line + 1;
					pc = pc + 4;
				}
			}
			else if (inst[t_list[j].idx].type == 'J') {
				printf("%s", inst[t_list[j].idx].op);
				for (int t = 0;t < 10;t++) {
					if (strcmp(str[i + 1], s_list[t].name) == 0) {
						printf("%s", NumToBits(s_list[t].address / 4, 26));
					}
				}
				i = i + 2;
				pc = pc + 4;
			}
			else if (inst[t_list[j].idx].type == 'S') {
				printf("%s", inst[t_list[j].idx].op);
				printf("00000");				//rs
				delchar(str[i + 2], '$');
				delchar(str[i + 2], ',');
				printf("%s", NumToBits(atoi(str[i + 2]), 5));		//rt
				delchar(str[i + 1], '$');
				delchar(str[i + 1], ',');
				printf("%s", NumToBits(atoi(str[i + 1]), 5));			//rd
				printf("%s", NumToBits(atoi(str[i + 3]), 5));		//shamt
				printf("%s", inst[t_list[j].idx].funct);			//funct
				i = i + 4;
				pc = pc + 4;
			}
			else	//la경우
			{
				if (strcmp(str[i], "la") == 0) {
					for (int c = 0;c < 10;c++) {
						if (strcmp(str[i + 2], d_list[c].name) == 0) {
							if (d_list[c].address >= 0x10000) {		//lui 사용할 때
								if (d_list[c].address % 0x10000 == 0) {		//lui만 사용
									printf("%s", "00000");
									delchar(str[i + 1], '$');
									delchar(str[i + 1], ',');
									printf("%s", NumToBits(atoi(str[i + 1]), 5));

									luiadd = d_list[c].address / 0x10000;

									printf("%s", NumToBits(luiadd, 16));
									pc = pc + 4;

								}
								else
								{		//lui랑 ori 둘다 사용
									luiadd = d_list[c].address / 0x10000;
									oriadd = d_list[c].address % 0x10000;
									printf("%s", "00000");
									delchar(str[i + 1], '$');
									delchar(str[i + 1], ',');
									printf("%s", NumToBits(atoi(str[i + 1]), 5));
									printf("%s", NumToBits(luiadd, 16));

									printf("001101");
									printf("%s", NumToBits(atoi(str[i + 1]), 5));
									printf("%s", NumToBits(atoi(str[i + 1]), 5));

									printf("%s", NumToBits(oriadd, 16));
									pc = pc + 8;
								}
							}
						}
					}
				}
				i = i + 3;
			}
		}
	}
	for (int c = 0;c < 10;c++) {
		int d = 0;
		while (d_list[c].value[d] != NULL) {
			printf("%s", NumToBits(d_list[c].value[d], 32));
			d = d + 1;
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

		// freeing the memory
		free(filename);
	}
	return 0;
}
