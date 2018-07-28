/* filesys.c
  
  provides interface to virtual disk
  
 Written by Bradley Scott
 Student ID: 51661169
 Email: b.scott.16@aberdeen.ac.uk
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS] ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;

/* writedisk : writes virtual disk out to physical disk
 * 
 * in: file name of stored virtual disk
 */

void writedisk ( const char * filename )
{
   printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;
   
}

void readdisk ( const char * filename )
{
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}


/* the basic interface to the virtual disk
 * this moves memory around
 */

void writeblock ( diskblock_t * block, int block_address )
{
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}


/* read and write FAT
 * 
 * please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 * 
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
 *                FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
 */

/*
Implementation of copyFAT() 

*/
void copyFAT()

{
   diskblock_t block;
   int x;
   int y;
   int fatblocksneeded = (MAXBLOCKS / FATENTRYCOUNT);

   for (x = 0; x < fatblocksneeded; x++)
   {
      for (y = 0; y < FATENTRYCOUNT; y++)
      {
         block.fat[y] = FAT[((x*FATENTRYCOUNT)+y)];
         
      }
      writeblock(&block, x + 1);
   }
}

/* implement format()
 */

void format ( )
{
   diskblock_t block ;
   direntry_t  rootDir ;
   
   int         pos             = 0 ;
   int         fatentry        = 0 ;
   int         fatblocksneeded =  (MAXBLOCKS / FATENTRYCOUNT ) ;

   /* prepare block 0 : fill it with '\0',
    * use strcpy() to copy some text to it for test purposes
	* write block 0 to virtual disk
	*/
	
	
	for (int i =0; i < BLOCKSIZE; i++) block.data[i] = '\0';
	
	//add the string to the block
	strcpy(block.data, "CS3026 Operating Systems Assessment");
	
	//write block to virtual disk
	writeblock(&block,0);
	
		 
	/* prepare FAT table
	 * write FAT blocks to virtual disk
	 */
	 
	 //set all positions in fat to unused
	 for (pos = 0; pos < BLOCKSIZE; pos++)
	 	FAT[pos] = UNUSED;
	 
	 //set known fat properties 
	 
	 FAT[0] = ENDOFCHAIN;
	 FAT[1] = 2;
	 FAT[2] = ENDOFCHAIN;
	 FAT[3] = ENDOFCHAIN;
	 
	 //write fat to virtual disk
	 copyFAT();
	 
	 

	 // prepare root directory & write root directory block to virtual disk
	  
	 //clearing block data for use 
	  for (int i =0; i < BLOCKSIZE; i++) block.data[i] = '\0';
	  
	  //set directory entries to empty 
	  for(int i = 0; i < DIRENTRYCOUNT; i++) block.dir.entrylist[i].unused = TRUE;
	  	
	  //directory block
	  block.dir.isdir = 1;
	  //first element in entry list
	  block.dir.nextEntry = 0;
	  
	  //writes to position 3 where the diretory should be 
	  writeblock(&block,fatblocksneeded+1);
	  
	  //block number of root directory
	  rootDirIndex = fatblocksneeded+1;
	  
	  
}


/* use this for testing
 */



/*
Creating a new file 

*/


MyFILE * myfopen(char * filename,const char *mode){
  	
  	int found = FALSE;
  	int pos; 
  	diskblock_t block;
  	
  	
  	
  	//create free memory for file
  	MyFILE * newFile = malloc(sizeof(MyFILE));
  	
  	//set mode of file to write 
  	strcpy(newFile->mode, mode);
 
  	//get directory entry block 
  	block = virtualDisk[3];
  	
  	
  	//loop to see if file exists in directory block
  	
  	int i = 0;
  	
  	for (i = 0; i < DIRENTRYCOUNT; i++){
  		if(block.dir.entrylist[i].unused) continue;
  		//strcmp returns 0 if true
  		if(strcmp(block.dir.entrylist[i].name, filename) == 0 ){
  			found = TRUE;
  			pos = i;
  			break;
  		
  		}
  }
	
	if(found) {
		//sets first number in blockchain for file to what the entrylist has stored about that file
		newFile -> blockno = block.dir.entrylist[pos].firstblock;
	
		//file starts at start of block
		newFile -> pos = 0;
	}
	
	else{
		// look for empty directory 
  		for (i = 0; i < DIRENTRYCOUNT; i++)
  			if(block.dir.entrylist[i].unused){
  				//printf("free block found");
  				 break;
  			}
	
		//looks for a free fat entry
		for (pos = 0; pos < MAXBLOCKS; pos++)
  			if(FAT[pos] == UNUSED) break;
  		
  		FAT[pos] = ENDOFCHAIN;
		
		//set first block pos of file
		newFile -> blockno = pos;
	  //set position of first block in directory list 
		block.dir.entrylist[i].firstblock = pos;
	
		copyFAT();
	
	
		//set filename to block
	    strcpy(block.dir.entrylist[i].name, filename);
		block.dir.entrylist[i].unused = FALSE;
	
		writeblock(&block,3);
		

	}
	
	return(newFile);
	
	
	
}

/*
writing data to the block


*/

void myfputc(int b, MyFILE * stream){
	int fatpos;
	int found =FALSE;
	int i;
	
	//start position of file in fat 
	fatpos = stream->blockno; 
	
//finds last block in file by looping through FAT blockchain of file so that character can be inserted 
	while(!found){
		if(FAT[fatpos] == ENDOFCHAIN) {
			found = TRUE;
		}
		else {
			fatpos = FAT[fatpos];
		}
	}
	
	// sets buffer to final block of file
	stream->buffer = virtualDisk[fatpos];
	
	//finds end position of data in block
	for(i=0; i<BLOCKSIZE; i++){
		if(stream->buffer.data[i] == '\0'){
			//pos is the position in the block that is free/where to start placing more data
			stream->pos = i;
			break;
		}
	}
	
	// add new data to the open file block
	stream->buffer.data[stream->pos]=b;
	
	// write buffer block to the virtualDisk
	writeblock(&stream->buffer, fatpos);
	
	
	
	// increment end position
	stream->pos++;
	
	// looks to see if at the end of block and finds next free pos in FAT
	if(stream->pos==BLOCKSIZE){
		stream->pos=0;
		for(i=0; i<BLOCKSIZE; i++)
			if(FAT[i]==UNUSED) 
				break;
	
	//set next position in fat and write to virtual disk			
	FAT[fatpos] = i;
	FAT[i] = ENDOFCHAIN;
	copyFAT();
	}
	
	
	
	//clear the buffer block for new data
	for(i=0; i<MAXBLOCKS; i++)
		stream->buffer.data[i] = '\0';
	
	

	
}
int myfgetc ( MyFILE * stream ){
//Returns the next byte of the open file, or EOF (EOF == -1)

char character;


//if at end of block reset file position and set block number of file to next in fat block 
if ((stream -> pos) == BLOCKSIZE){
	stream -> pos = 0;
	stream -> blockno = FAT[stream -> blockno];
	return character;
	}
//otherwise set the buffer to the current block
else{
	stream -> buffer = virtualDisk[stream->blockno];
//set character to be the next character in the block
	character = stream -> buffer.data[stream->pos];
//increment position in file/character
	stream -> pos ++;
	return character;
}



}





void mymkdir( char * path) {

char str[strlen(path)+1];

strcpy(str,path);

char *token;
char *rest = str;
int blocknumber = 3;
int newblock;
int freeentry;
while ((token = strtok_r(rest, "/" , &rest))){

    printf("mymkdir > block number is %i\n", blocknumber);
    

	
	int found = FALSE;
  	diskblock_t block;
  	
  	//mem copy block
  	block = virtualDisk[blocknumber];
  	
  	//first element in entry list
	block.dir.nextEntry = 0;
  	
  	//initialising that it is a directory block
  	
  	block.dir.isdir = 1;
  		
  if (blocknumber > 3){
  	 for(int i = 0; i < DIRENTRYCOUNT; i++) block.dir.entrylist[i].unused = TRUE;
  
  }
  
  for (int j = 0; j < DIRENTRYCOUNT; j++){
  		printf("i get here in this loop yes yes");
  		if(block.dir.entrylist[j].unused){
  			freeentry = j;
  			printf("mymkdir > free is found");
  			break;
  			}
  	}	
  	
  	
  	
  	printf(" \n mymkdir > free dir entry found at pos %i \n", freeentry);	
  		//set filename to block
	strcpy(block.dir.entrylist[freeentry].name, token);
	    //set block to be used
	block.dir.entrylist[freeentry].unused = FALSE;
	
		//looks for a free fat entry
	for (int pos = 0; pos < MAXBLOCKS; pos++)
  		if(FAT[pos] == UNUSED) {
  			newblock = pos;
  			break;
  			}
  		
  	printf(" \n mymkdir > %s is going to fat pos %i \n",token, newblock);
  		
  	FAT[newblock] = ENDOFCHAIN;
		
		
	 	 //set position of first block in directory list 
	block.dir.entrylist[freeentry].firstblock = newblock;
	
	copyFAT();
	
	writeblock(&block,blocknumber);
	
	blocknumber = newblock;



}



}



char * mylistdir (char * path) {
/*
tokenize path 
look for first token 

if found 

look for next until last token 

if not found exit 

*/

char str[strlen(path)+1];

strcpy(str,path);
char* filename;
char *token;
char *rest = str;
int blocknumber = 3;
int newblock;

while ((token = strtok_r(rest, "/" , &rest))){

    printf("block number is %i\n", blocknumber);
   
    int target;
	
	int found = FALSE;
  	diskblock_t block;
  	
  
  	block = virtualDisk[blocknumber];
 
  		
  	
	// look for token
	
	
	for (int i = 0; i < DIRENTRYCOUNT; i++){
  			
  			if(block.dir.entrylist[i].unused) continue;
  		
			
  			if(strcmp(block.dir.entrylist[i].name, token) == 0 ){
  				
  				found = TRUE;
  				target = i;
  				break;
  		
  		}
		

  			
  	if (found) {
  	
  	filename = block.dir.entrylist[target].name;
  	continue;
  	
  	}
  	
  	else {
  	
  	 filename = "target not found";
  	 break;
  	
  	
  	}
	   


blocknumber = block.dir.entrylist[target].firstblock;
}



}

printf("%s",filename);
}

void testprint (){
diskblock_t block;
block = virtualDisk[3];
printf("the name at block 3 entry 1 is %s \n",block.dir.entrylist[1].name);
printf("entry list of next block points to block  %hd \n",block.dir.entrylist[1].firstblock);
block = virtualDisk[9];
printf("the name at block 9 entry 0 is %s \n",block.dir.entrylist[1].name);
printf("entry list of next block points to block  %hd \n",block.dir.entrylist[1].firstblock);
block = virtualDisk[10];
printf("the name at block 10 entry 0 is %s \n",block.dir.entrylist[1].name);
printf("entry list of next block points to block  %hd \n",block.dir.entrylist[1].firstblock);
}


void myfclose(MyFILE * stream) {
	// closes the file
	free(stream);
}


void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

