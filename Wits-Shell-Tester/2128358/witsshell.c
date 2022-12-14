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
//char *trimwhitespace(char *str);

#define LENGTH 15
#define PATH "/bin/"

char* currPath[LENGTH];
char* commands[LENGTH];
char* allCommands[LENGTH];
int ncom = 0;
int nAllCom = 0;
int nPaths = 0;
int pathNum = 0;

int main(int MainArgc, char *MainArgv[]){
	currPath[0] = PATH;
	if(MainArgc == 1){
		witsshell();
	}else if(MainArgc == 2){
		int fileAccess = access(MainArgv[1], X_OK);
		if(fileAccess==0){
			batchmode(MainArgv[1]);
		}else{
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			return 1;
		}
		
	}else{
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		return 1;
	}

	return 0;
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

	FILE* readFile = fopen(MainArgv, "r");
	if(!readFile){
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
	}else{
		char* contents = NULL;
		size_t len = 0;
		while (getline(&contents, &len, readFile) != -1){
			if(!strcspn(contents, "\n")){
				continue;
			}
			contents[strcspn(contents, "\n")] = 0;
			allCommands[i] = strdup(contents);
			i++;
		}
		nAllCom=i;
		fclose(readFile);
		free(contents);
		excecuteCommand(allCommands);
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
		//Allcommands[i] = trimwhitespace(Allcommands[i]);
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
				char temp[255] ="";
				for(int j=1;j<ncom-1;j++){
					strcat(temp, commands[j]);
					strcat(temp, " ");
				}
				strcat(temp, commands[ncom-1]);
				printf("%s\n", temp);
			}else{
				printf("\n");
			}
		}else if(!strcmp(commands[0], "cd")){
			excecutecd(commands);
		}else if(!strcmp(commands[0], "path")){
			excecutePath(commands);
		}else if(haveAccess(commands[0])){
			excecuteSH(commands);
		}else if(!strcmp(commands[0], "&")){
			continue;
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
		for(int i=0;i<ncom;i++){
			if(!strcmp(commands[i], ">")){
				if(i==ncom-1 || i<ncom-1){
					char error_message[30] = "An error has occurred\n";
					write(STDERR_FILENO, error_message, strlen(error_message));
					return;
				}
			}
		}
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
	if(ncom==1){
		memset(currPath, 0, sizeof(currPath));
		nPaths = 0;
	}else{
		for(int i=0;i<ncom-1;i++){
			currPath[nPaths+1] = commands[i+1];
			nPaths++;
		}
	}
}

void excecuteSH(char* commands[]){
	char temp[50] = "";
	strcat(temp, currPath[pathNum]);
	strcat(temp, commands[0]);
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
		if(a==0){
			pathNum = i;
			return true;
		} 
		i++;
	}
	return false;
}

// char *trimwhitespace(char *str)
// {
//   char *end;

//   // Trim leading space
//   while(isspace((unsigned char)*str)) str++;

//   if(*str == 0)  // All spaces?
//     return str;

//   // Trim trailing space
//   end = str + strlen(str) - 1;
//   while(end > str && isspace((unsigned char)*end)) end--;

//   // Write new null terminator character
//   end[1] = '\0';

//   return str;
// }