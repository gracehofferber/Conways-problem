//Created by Grace Hofferber
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "ezipc.h"

#define card_size 20
#define print_size 25

int main(){
	SETUP();
	int count = 1;
	int c = 1;
	int status;
	int squash_pid, print_pid;
	FILE *file;//input
	//shared memory: one character buffer.
	char* squash_buffer = SHARED_MEMORY(1);
	char* print_buffer = SHARED_MEMORY(1);
	//semaphores
	int squash, S_mutex, print, P_mutex;
	squash  = SEMAPHORE(SEM_BIN,0);
	S_mutex = SEMAPHORE(SEM_BIN,1);
	print = SEMAPHORE(SEM_BIN,0);
	P_mutex = SEMAPHORE(SEM_BIN,1);


	file = fopen("lab5.txt","r");
	if(file ==NULL){
		perror("ERROR: can't open file \n");
		exit(1);
	}
	
//Squash: reading in from buffer, looking for ** to replace w #.
	squash_pid = fork();
	if(squash_pid == 0){
		char first_star = 0;
		char second_star = 0;
		char replace = '#';
		while(1){
			P(squash);
			first_star = *squash_buffer;
			V(S_mutex);
			if(first_star == '*'){
				second_star = first_star;
				P(squash);
				first_star = *squash_buffer;
				V(S_mutex);
				if(first_star == '*' && second_star == '*'){
					P(P_mutex);
					*print_buffer = replace;
					V(print);
				}
				else{
					P(P_mutex);
					*print_buffer = second_star;
					V(print);

					P(P_mutex);
					*print_buffer = first_star;
					V(print);
				}
		}
			else{ 
			P(P_mutex);
			*print_buffer = first_star;
			V(print);
			}
		}
	
	}
//Print: prints out the buffer(1 char @ time) for 25 char lines.
	print_pid = fork();
	if(print_pid == 0){
		int i = 1;
		while(1){
		P(print);
		printf("%c", *print_buffer);
		if( i % print_size == 0){//max chars per line has been reached.
			printf("\n");
		}
		fflush(stdout);
		i++;

		V(P_mutex);
		}
	}
//Producer: reading in the fil 1 char at a time into the buffer.
	c = fgetc(file);
	while(c != EOF){
		if( c== '\n'){ //reached end of line..insert <EOL>
			P(S_mutex);
			*squash_buffer = '<';
			V(squash);
		
			P(S_mutex);
			*squash_buffer = 'E';
			V(squash);

			P(S_mutex);
			*squash_buffer = 'O';
			V(squash);

			P(S_mutex);
			*squash_buffer = 'L';
			V(squash);

			P(S_mutex);
			*squash_buffer = '>';
			V(squash);
		}
		else{
			P(S_mutex);
			*squash_buffer = c;
			V(squash);
		}
		c =fgetc(file);
	}
	wait(&status);	
 	return 0;
}//end of main.
