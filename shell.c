/*
 Written by Bradley Scott
 Student ID: 51661169
 Email: b.scott.16@aberdeen.ac.uk
*/



#include "filesys.h"
#include <stdio.h>

//MyFILE * myfopen(char * filename,const char mode);


void cgsD(){
	format();
	writedisk("virtualdiskD3_D1");
	printf("D task completed \n ");
}

void cgsC(){

	printf("\n output for c task below \n ");
	MyFILE * thefile = myfopen("testfile.txt", "w");


	char * alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int x = 0;
	int y = 0; 
	//insert each character of alphabet to the file 4096 times(4KB file)
	for(x=0; x<(4*BLOCKSIZE);x++){
		if(y==26)
			y=0;
		myfputc(alphabet[y], thefile);
		y++;	
	}
	//insert EOF to end of the file
	myfputc(EOF, thefile);
	
	//close the file
	myfclose(thefile);
	
	//write complete virtual disk to a file
	writedisk("virtualdiskC3_C1");

	
	
	thefile = myfopen("testfile.txt", "w");
		
	char character;
	
	FILE * printfile;
	
	printfile = fopen("testfileC3_C1_copy.txt", "w");
	
	
	printf("testfile data below: \n");
	
	while(character != EOF){
		character = myfgetc(thefile);
		
		//printf("%c", character);
		if(character != EOF){
			fprintf(printfile, "%c", character);
			printf("%c", character);
			}
		}
	
	

printf("\n");	
		
}

void cgsB() {

printf("\n output from the B task below \n");

char * path = "/myfirstdir/myseconddir/mythirddir";

mymkdir(path);

//write complete virtual disk to a file
writedisk("virtualdiskB3_B1");

/*
for debugging shows how the directories are linked in mymkdir
testprint();

*/


}

int main() {
	
	cgsD();
	
	cgsC();
	
	cgsB();
	
	return 0;
}
