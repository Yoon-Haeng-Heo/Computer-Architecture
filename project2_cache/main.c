#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for getopt()

#define BYTES_PER_WORD 4
// #define DEBUG

/*
 * Cache structures
 */
int time = 0;

typedef struct {
	int age;	//LRU bit 
	int valid;	//when cache data filled valid bit = 1
	int modified;	// after write - hit modified bit = 1
	uint32_t tag;	// cache tag
} cline;

typedef struct {
	cline *lines;	//lines[number of sets]
} cset;

typedef struct {
	int s;		//set
	int E;		//associativity
	int b;		//block
	cset *sets;	//
} cache;

static int index_bit(int n) {
	int cnt = 0;
	while(n){
		cnt++;
		n = n >> 1;
	}
	return cnt-1;
}

cache build_cache(int capacity, int way, int blocksize) {
	
	cache C={};
	int set = capacity/blocksize/way;
	
	C.s = set;
	
	C.E = way;
	
	C.b =blocksize;
	
	
	int cc =0;
	int i=0;
	int j=0;
	C.sets = malloc(sizeof(cset)*set);
	for(i=0;i<set;i++){
		C.sets[i].lines = malloc(sizeof(cline)*way);
	}
	for(i=0;i<set;i++){
		for(j=0;j<way;j++){
			C.sets[i].lines[j].age = 0;
			C.sets[i].lines[j].valid = 0;
			C.sets[i].lines[j].modified = 0;
			C.sets[i].lines[j].tag = 0;
			
		}
	}
	
	return C;
}


int access_cache(char *op,uint32_t addr,cache C) { 	
						//return 1~6 (Write hit,Write back,read hit,WR misses)
	int offset = 0;
	int index = 0;
	uint32_t tag = 0;

	int i=0,j=0;
	int s = index_bit(C.s);
	int b = index_bit(C.b);
	int way = C.E;
	int set = 1 << s;
	int max = 0;
	int a1=0,a2=0;
	char c1[3] = "R";
	char c2[3] = "W";
	uint32_t save = addr;
	offset = addr % (1 << b);
	addr = addr / (1 << b);		//addr => tag+index;
	index = addr % (1 << s);
	tag = addr / (1 << s);		
	for(int a=0;a<set;a++){
		for(int q=0;q<way;q++){
			C.sets[a].lines[q].age++;
		}
	}
	for(i=0;i<way;i++){
		if(tag == C.sets[index].lines[i].tag && C.sets[index].lines[i].valid == 1){
			if(strcmp(op,c1) == 0){	//read hit
				C.sets[index].lines[i].age = 0;
				return 1;
			}
			else if(strcmp(op,c2) == 0){	//write hit
				C.sets[index].lines[i].age = 0;
				C.sets[index].lines[i].modified = 1;
				return 2;
			}
		}
		else if(tag != C.sets[index].lines[i].tag && C.sets[index].lines[i].valid == 0 && C.sets[index].lines[i].tag == 0){
			C.sets[index].lines[i].tag = tag;
			C.sets[index].lines[i].valid = 1;
			C.sets[index].lines[i].age=0;
			
			if(strcmp(op,c1)==0){	//read miss
				C.sets[index].lines[i].modified = 0;	
				return 3;
			}
			else if(strcmp(op,c2)==0){	//write miss
				C.sets[index].lines[i].modified = 1;				
				return 4;
			}
		}
		else {	//(tag != C.sets[index].lines[i].tag && C.sets[index].lines[i].valid == 1){
			continue;	
		}
	}
	if(i == way){
		max = C.sets[index].lines[0].age;
		for(j=0;j<way;j++){
			if(C.sets[index].lines[j].age >= max){		//find minimum
				max = C.sets[index].lines[j].age;
				a1 = j;
			}
		}
		if(C.sets[index].lines[a1].modified){	//write-back
			if(strcmp(op,c1)==0){ //read miss and write-back
				C.sets[index].lines[a1].tag =tag;
				C.sets[index].lines[a1].age = 0;
				C.sets[index].lines[a1].modified = 0;
				C.sets[index].lines[a1].valid = 1;				
				return 5;
			}
			else if(strcmp(op,c2)==0){	//write miss and write-back
				C.sets[index].lines[a1].tag =tag;
				C.sets[index].lines[a1].age = 0;
				C.sets[index].lines[a1].modified = 1;
				C.sets[index].lines[a1].valid = 1;			
				return 6;
			}
		}
		else{
			
			if(strcmp(op,c1)==0){		//read-miss;
				C.sets[index].lines[a1].tag =tag;
				C.sets[index].lines[a1].age = 0;
				C.sets[index].lines[a1].modified = 0;
				C.sets[index].lines[a1].valid = 1;
				return 7;	
			}
			else if(strcmp(op,c2)==0){
				C.sets[index].lines[a1].tag =tag;
				C.sets[index].lines[a1].age = 0;
				C.sets[index].lines[a1].modified = 1;
				C.sets[index].lines[a1].valid = 1;
				return 8;		//write miss
			}

		}		
	}
		
}

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                           */
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */
/* 					                            		       */
/* Cache Design						                           */
/*  							                               */
/* 	    cache[set][assoc][word per block]		               */
/*                                						       */
/*      				                        		       */
/*       ----------------------------------------	           */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*                              						       */
/*                                                             */
/***************************************************************/
void xdump(cache* L)
{
	int i,j,k = 0;
	int b = index_bit(L->b), s = index_bit(L->s);
	int way = L->E, set = 1 << s;
	int E = index_bit(way);
	
	uint32_t line;
	
	
	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");
		
	for(i = 0 ; i < set;i++)
	{	
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			if(L->sets[i].lines[j].valid){
				line = L->sets[i].lines[j].tag << (s+b);	//make tag 32bit
				line = line|(i << b);//add set number to line ->why cache address multiple of 4
			}
			else{
				line = 0;
			}
			printf("0x%08x  ", line);
		}
		printf("\n");
	}
	printf("\n");
}




int main(int argc, char *argv[]) {
	int i, j, k;
	int capacity=1024;
	int way=8;
	int blocksize=8;
	int set = 16;
	
	//cache
	cache simCache={};
	// counts
	int read=0, write=0, writeback=0;
	int readhit=0, writehit=0;
	int readmiss=0, writemiss = 0;

	// Input option
	int opt = 0;
	char* token;
	int xflag = 0;

	// parse file
	char *trace_name = (char*)malloc(32);
	FILE *fp;
    	char line[16];
    	char *op;
    	uint32_t addr;
	int num = 0;
    /* You can define any variables that you want */
	
	trace_name = argv[argc-1];
	if (argc < 3) {
		printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
		exit(1);
	}
	while((opt = getopt(argc, argv, "c:x")) != -1){
		switch(opt){
			case 'c':
                // extern char *optarg;
				token = strtok(optarg, ":");
				capacity = atoi(token);
				token = strtok(NULL, ":");
				way = atoi(token);
				token = strtok(NULL, ":");
				blocksize  = atoi(token);
				break;
			case 'x':
				xflag = 1;
				break;
			default:
			printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
			exit(1);

		}
	}
	// allocate
	set = capacity/way/blocksize;

	
	
    /* TODO: Define a cache based on the struct declaration */
	
    	simCache = build_cache(capacity,way,blocksize);
	
	// simulate
	fp = fopen(trace_name, "r"); // read trace file
	if(fp == NULL){
		printf("\nInvalid trace file: %s\n", trace_name);
		return 1;
	}
	cdump(capacity, way, blocksize);
	
    /* TODO: Build an access function to load and store data from the file */
    	while (fgets(line, sizeof(line), fp) != NULL) {
        	op = strtok(line, " ");			//R,W
        	addr = strtoull(strtok(NULL, ","), NULL, 16);		//address
		
#ifdef DEBUG  	// You can use #define DEBUG above for seeing traces of the file.
        	fprintf(stderr, "op: %s\n", op);
        	fprintf(stderr, "addr: %x\n", addr);
#endif
       	 	// ...
       	 	num = access_cache(op,addr,simCache);
		
		switch(num){
			case 1:
				readhit++;
				read++;
				break;
			case 2:
				writehit++;
				write++;
				break;
			case 3:
				readmiss++;
				read++;
				break;
			case 4:
				writemiss++;
				write++;
				break;
			case 5: 
				readmiss++;
				writeback++;
				read++;
				break;
			case 6:
				writemiss++;
				writeback++;
				write++;
				break;
			case 7:
				readmiss++;
				read++;
				break;
			case 8:
				writemiss++;
				write++;
				break;
			
			default:
				printf("num == 0\n");
		}
		
        // ...
    	}

    // test example
	sdump(read, write, writeback, readhit, writehit, readmiss, writemiss);

	if (xflag){
	    	xdump(&simCache);
	}
    	return 0;
}
