#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

void witsshell();
void batchmode(char *MainArgv);
void storeCommand(char* CommandLine);
void excecuteCommand(char *MainArgv);
int check_for_EOF();

#define LENGTH 15

char *commands[LENGTH];
int ncom = 0;
char* exitString = "exit";
int exitInt = 1;

int main(int MainArgc, char *MainArgv[]){
	if(MainArgc == 1){
		witsshell();
	}else if(MainArgc == 2){
		batchmode(MainArgv[1]);
	}else{
		printf("Invalid number of arguments. Must be either 0 (./witsshell) or 1 (./witsshell batch.txt)");
	}

	return(0);
}

void witsshell(){
	char *buffer = NULL;
    size_t bufsize = 32;
	do {
		printf("witsshell> ");

		if(check_for_EOF()){  //if CTRL-D then exit gracefully
			printf("^D\n");
			exit(0);
		}
		
		getline(&buffer,&bufsize,stdin);
		buffer[strcspn(buffer, "\n")] = 0;
		
		storeCommand(buffer);

		for(int i=0;i<ncom;i++){
			printf("%s\n", commands[i]);
		}
	}while(exitInt != 0);
	free(buffer);
	exit(0);
}

void batchmode(char *MainArgv){
	printf("Opening %s ", MainArgv);
}

void storeCommand(char* CommandLine){  //function to store commands into a global string array 
	int i = 0;

	char * token = strtok(CommandLine, " "); //extract first word of commands

	exitInt = strcmp(token,exitString); //if command is "exit" then exit gracefully
	if(exitInt==0){
		exit(0);
	}

   	// loop through the string to extract all other tokens
   	while( token != NULL ) {
      	commands[i] = token;
      	token = strtok(NULL, " ");
		i++;
   	}
	ncom = i;
}

void excecuteCommand(char *MainArgv){
	printf("Excecuting %s ", MainArgv);
}

int check_for_EOF(){  //checks for eof or CTRL-D
    if (feof(stdin)) return 1;
    int c = getc(stdin);
    if (c == EOF) return 1;
    ungetc(c, stdin);
	return 0;
}