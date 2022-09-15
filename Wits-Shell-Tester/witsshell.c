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
void excecuteCommand(char *Allcommands[]);
void exec(char* commands[]);
void excecutels(char* commands[]);
void excecutecd(char* commands[]);
void excecutePath(char* commands[]);
void excecuteSH(char* commands[]);
int check_for_EOF();
bool haveAccess(char* command);

#define LENGTH 15
#define PATH "/bin/"

char* currPath[LENGTH];
char* commands[LENGTH];
char* allCommands[LENGTH];
int ncom = 0;
int nAllCom = 0;
int nPaths = 0;

int main(int MainArgc, char *MainArgv[]){
	currPath[0] = PATH;
	if(MainArgc == 1){
		witsshell();
	}else if(MainArgc == 2){
		batchmode(MainArgv[1]);
	}else{
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}

	return(0);
}

void witsshell(){
	char *buffer = NULL;
    size_t bufsize = 32;
	while(true){
		printf("witsshell> ");
		
		if(check_for_EOF()){  //if CTRL-D then exit gracefully
			exit(0);
		}

		getline(&buffer,&bufsize,stdin);

		if(!strcmp(buffer, "\n")){
			continue;
		}

		buffer[strcspn(buffer, "\n")] = 0;

		allCommands[0] = buffer;
		nAllCom = 1;
		
		excecuteCommand(allCommands);
	}
	free(buffer);
}

void batchmode(char *MainArgv){
	int i = 0;
	int fileAccess;
	fileAccess = access(MainArgv, X_OK);

	if(fileAccess==0){
		FILE* readFile = fopen(MainArgv, "r");
		if(!readFile){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}else{
			char* contents = NULL;
			size_t len = 0;
			while (getline(&contents, &len, readFile) != -1){
				contents[strcspn(contents, "\n")] = 0;
				allCommands[i] = strdup(contents);
				i++;
			}
			nAllCom=i;
			fclose(readFile);
			free(contents);
			excecuteCommand(allCommands);
		}
	}else{
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}
}

void storeCommand(char* CommandLine){  //function to store commands into a global string array 
	int i = 0;
	memset(commands, 0, sizeof(commands));

	char * token = strtok(CommandLine, " "); //extract first word of commands
	
   	// loop through the string to extract all other tokens
   	while( token != NULL ) {
      	commands[i] = token;
      	token = strtok(NULL, " ");
		i++;
   	}
	ncom = i;
}

void excecuteCommand(char *Allcommands[]){
	for(int i=0;i<nAllCom;i++){
		storeCommand(Allcommands[i]);
		
		if(!strcmp(commands[0], "ls")){
			excecutels(commands);
		}else if(!strcmp(commands[0],"exit")){
			if(ncom==1){
				exit(0);
			}else{ 
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}else if(!strcmp(commands[0], "echo")){
			if(ncom!=1){
				printf("%s\n", commands[1]);
			}else{
				printf("\n");
			}
		}else if(!strcmp(commands[0], "cd")){
			excecutecd(commands);
		}else if(!strcmp(commands[0], "path")){
			excecutePath(commands);
		}else if(haveAccess(commands[0])){
			excecuteSH(commands);
		}
		else{
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
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
	if(ncom>1){
		int fileAccess;
		fileAccess = access(commands[1], F_OK);

		if(fileAccess!=0){
			char error_message[100] = "ls: cannot access '";
			strcat(error_message, commands[1]);
			strcat(error_message, "': No such file or directory\n");
			write(STDERR_FILENO, error_message, strlen(error_message));
		}else{
			exec(commands);
		}
	}else{ //if(fork()==0) if you dont wanna close witsshell after ls
		exec(commands);
		//execv("/bin/ls", commands);
	}
}

void excecutecd(char* commands[]){
	if(ncom!=2){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}else{
		int fileAccess;
		fileAccess = access(commands[1], F_OK);

		if(fileAccess!=0){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}else{
			if(chdir(commands[1])!=0){
				char error_message[30] = "An error has occurred\n";
				write(STDERR_FILENO, error_message, strlen(error_message));
			}
		}
	}
}

void exec(char* commands[]){ 
	int failExec = 0;
    if (fork()== 0){
		int i=0;
		while(currPath[i]!=0){
			char run[50] = "";
			strcat(run, currPath[i]); //run = PATH = /bin/
			strcat(run, commands[0]); //run = PATH + commands[0] ie. /bin/ls
			if(!access(run, F_OK))
				execv(run, commands);	//runs command through execv
			else failExec++;
			i++;
		}
		if(failExec==nPaths){
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		exit(0);           /* terminate child */
	}else
        wait(NULL); /* reaping parent */

	
}

void excecutePath(char* commands[]){
	memset(currPath, 0, sizeof(currPath));
	nPaths = 0;
	for(int i=0;i<ncom-1;i++){
		currPath[i] = commands[i+1];
		nPaths++;
	}
}

void excecuteSH(char* commands[]){
	char* temp = commands[0];
	commands[0] = "sh";
	commands[1] = temp;
	
	exec(commands);
}

bool haveAccess(char* command){
	int i=0;
	
	while(currPath[i]!=0){
		char run[50] = "";
		strcat(run, currPath[i]); //run = PATH = /bin/
		strcat(run, command); //run = PATH + commands[0] ie. /bin/ls
		int a = access(run, F_OK);
		if(a==0) return true;
		i++;
	}
	return false;
}