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
void excecuteCommand(char *commands[]);
void excecutels(char* commands[]);
int check_for_EOF();

#define LENGTH 15
#define PATH "/bin/"

char* currPath[LENGTH];
char* commands[LENGTH];
int ncom = 0;
char* exitString = "exit";
int exitInt = 1;

int main(int MainArgc, char *MainArgv[]){
	currPath[0] = PATH;
	if(MainArgc == 1){
		witsshell();
	}else if(MainArgc == 2){
		batchmode(MainArgv[1]);
	}else{
		//printf("Invalid number of arguments. Must be either 0 (./witsshell) or 1 (./witsshell batch.txt)");
		exit(1);
	}

	return(0);
}

void witsshell(){
	char *buffer = NULL;
    size_t bufsize = 32;
	do {
		printf("witsshell> ");

		if(check_for_EOF()){  //if CTRL-D then exit gracefully
			//printf("^D\n");
			exit(0);
		}
		
		getline(&buffer,&bufsize,stdin);
		buffer[strcspn(buffer, "\n")] = 0;
	
		storeCommand(buffer);

		/*for(int i=0;i<ncom;i++){
			printf("command: %s\n", commands[i]);
		}*/
		excecuteCommand(commands);
	}while(exitInt != 0);
	free(buffer);
	exit(0);
}

void batchmode(char *MainArgv){
	int i =0;
	int fileAccess;
	fileAccess = access(MainArgv, X_OK);
	if(fileAccess==0){
		FILE* readFile = fopen(MainArgv, "r");
		if(!readFile){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		char* contents = NULL;
		size_t len = 0;
		while (getline(&contents, &len, readFile) != -1){
			contents[strcspn(contents, "\n")] = 0;

			exitInt = strcmp(contents,exitString); //if command is "exit" then exit gracefully
			if(exitInt==0){
				exit(0);
			}
			commands[0] = contents;
			excecuteCommand(commands);
			i++;
		}

		fclose(readFile);
		free(contents);

		exit(0);
	}else{
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
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

void excecuteCommand(char *commands[]){
	if(!strcmp(commands[0], "ls")){
		excecutels(commands);
	}else if(strcmp(commands[0],exitString)){
		exit(0);
	}
}

int check_for_EOF(){  //checks for eof or CTRL-D
    if (feof(stdin)) return 1;
    int c = getc(stdin);
    if (c == EOF) return 1;
    ungetc(c, stdin);
	return 0;
}

void excecutels(char* commands[]){
	int er = execv("/bin/ls", commands);
	if(er == -1){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}