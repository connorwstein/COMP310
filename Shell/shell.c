/*
 * shell.c
 *
 *  Created on: Jan 19, 2015
 *      Author: connorstein
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_COMMAND_HISTORY 10 /* Only store 10 most recent commands.*/
#define ARGS_SUCCESS 1 /*Indicates args array has been created successfully. */
#define ARGS_FAILURE 0 /*Args array has not been created successfully, occurs when error using history, reading command, or args is all NULL.*/
#define MAX_PATH_SIZE 512 /*Maximum path size for the pwd command.*/

/*past_command struct includes PID for jobs command.*/
typedef struct command{
	int command_number;
	int background;
	pid_t pid;
	char inputBuffer[MAX_LINE];
}past_command;


past_command command_history[MAX_COMMAND_HISTORY]; /*Global command history of 10 most recent commands.*/
int command_history_index=0; /*Points to a free spot in command history, once command history is full this is capped at 10. */
int command_count=0; /* Global command count.*/

void update_history(char inputBuffer[MAX_LINE],int command_count){

	if(command_history_index<MAX_COMMAND_HISTORY){
		/*If less than 10 commands have been entered.*/
		memcpy(command_history[command_history_index].inputBuffer,inputBuffer, MAX_LINE);
		command_history[command_history_index].command_number=command_count;
		command_history_index++;
	}
	else{
		/* More than 10 commands have been entered, command_history is full.*/
		/* command_history index is 10. */
		/* Shift past_command's at index 1-9 to indices 0-8, freeing up the last spot for the new command.*/
		int i;
		for(i=1;i<MAX_COMMAND_HISTORY;i++)
		{
			command_history[i-1]=command_history[i];			
		}
		/* Put newest command in the free spot at index 9 (last index in command_history).*/
		memcpy(command_history[command_history_index-1].inputBuffer,inputBuffer, MAX_LINE);
		command_history[command_history_index-1].command_number=command_count;
	}
}
void update_background_history(int background){
	/* Add the background information to the command history.*/
	/* This method is called after update_history in setup, so command_history_index-1 always points to the 
	most recent command added to history. */
	command_history[command_history_index-1].background=background;
}
void update_pid_history(pid_t pid){
	/* Same as update_background_history, but for the process ID.*/
	command_history[command_history_index-1].pid=pid;
}
int is_alive(int pid){
	/* Simply checks if a process is still running. */
    int status = 0;
    return (waitpid(pid, &status, WNOHANG) == 0);
}
void print_history(void){
	int i;
	for(i=0;i<MAX_COMMAND_HISTORY;i++){
		printf("Command number: %d, Command: ", command_history[i].command_number);
		/* Print the command char by char to get past the null chars. */
		int j;
		for(j=0;j<MAX_LINE;j++){
			if((command_history[i].inputBuffer)[j]!='\n')
			printf("%c",(command_history[i].inputBuffer)[j]);
		}
		printf(", Background: %d", command_history[i].background);	
		printf(", PID: %d\n", command_history[i].pid);	
	}

}
void clean_args(char *args[MAX_LINE/2]){
	/* Set all args pointers to NULL as the default state. */
	int i;
	for(i=0;i<MAX_LINE/2;i++){
		args[i]=NULL;
	}
}
void clean_inputBuffer(char inputBuffer[MAX_LINE]){
	/* Set all the inputBuffer chars to null chars as the default state. */
	int i;
	for(i=0;i<MAX_LINE;i++){
		inputBuffer[i]='\0';
	}	
}
int setup(char inputBuffer[], char *args[],int *background){
	int length, /* # of characters in the command line */
		i,        /* loop index for accessing inputBuffer array */
		start,    /* index where beginning of next command parameter is */
		ct;       /* index of where to place the next parameter into args[] */

	ct = 0;

	/* read what the user enters on the command line */
	length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

	start = -1;
	if (length == 0) {
		/* ctrl-d was entered, quit the shell normally */
		printf("\n");
		exit(0);
	} 
	if (length < 0) {
		/* something went wrong */
		perror("Reading the command");
		return ARGS_FAILURE; 
	}
	
  	int history_call=0; /* Indicates if the call is from history or not. 0=Not from history, 1=From history.*/
  	/*If a history call is made, overwrite the inputBuffer with the appropriate past inputBuffer, before parsing.*/
	if(inputBuffer[0]=='r'&&inputBuffer[1]=='\n'){
		/*History call for most recent has been made.*/
		/*First check if the command number in the last position of command history is not equal to 0 (0 is the default command number).*/
		/*If more than 10 commands have been entered, there will be a non-zero command number in the last position of command history
		and that command is the most recent (see how update_command_history works). */
		if(command_history[MAX_COMMAND_HISTORY-1].command_number!=0){
			memcpy(inputBuffer, command_history[MAX_COMMAND_HISTORY-1].inputBuffer, MAX_LINE);
			history_call=1;
			/* Echo the history call to the user. Print everything except the newline char.*/
			printf("History call matches command ");
			int k;
			for(k=0;k<MAX_LINE;k++){
				if((command_history[MAX_COMMAND_HISTORY-1].inputBuffer)[k]!='\n') printf("%c",(command_history[MAX_COMMAND_HISTORY-1].inputBuffer)[k]);			
			}
			printf(". Executing...\n");
		}
		else{
			/* Less than 10 commands have been entered. Need to loop until we find a command with a non-zero command number (i.e.
			the most recent command. */
			int j=MAX_COMMAND_HISTORY;
			while(command_history[--j].command_number==0);
			if(j!=-1){
				memcpy(inputBuffer, command_history[j].inputBuffer, MAX_LINE);
				history_call=1;
				printf("History call matches command ");
				int k;
				for(k=0;k<MAX_LINE;k++){
					if((command_history[j].inputBuffer)[k]!='\n') printf("%c",(command_history[j].inputBuffer)[k]);			
				}
				printf(". Executing...\n");
			}
			else{
				/*If j=-1, the user has attempted to call the most recent command without having entered any commands yet.*/
				printf("No history yet\n");
				return ARGS_FAILURE;
			}
		}
	}
	else if(inputBuffer[0]=='r'&&inputBuffer[1]==' '){
		/*History call with an input.*/
		/*Loop from the most recent to the least recent looking for a match with the first char of the past_command's inputBuffer.*/
		int j=MAX_COMMAND_HISTORY-1;
		while(j>=0 &&(command_history[j].inputBuffer)[0]!=inputBuffer[2])j--;
		if(j!=-1){
			memcpy(inputBuffer, command_history[j].inputBuffer, MAX_LINE);
			history_call=1;
			printf("History call matches command ");
			int k;
			for(k=0;k<MAX_LINE;k++){
				if((command_history[j].inputBuffer)[k]!='\n') printf("%c",(command_history[j].inputBuffer)[k]);			
			}
			printf(". Executing...\n");
		}
		else{
			/*No commands have a matching first char of any past_command's inputBuffer.*/
			printf("No command matches that history call\n");
			return ARGS_FAILURE;
		}
	}
	command_count++; /*After checking for history calls (and overwriting the inputBuffer with an old inputBuffer 
	if necessary), increment the global command_count*/
  	update_history(inputBuffer, command_count);  
	/* examine every character in the inputBuffer */
	for (i = 0; i < length || (history_call==1 && !(inputBuffer[i]=='\0'&&inputBuffer[i+1]=='\0')); i++) { 
		switch (inputBuffer[i]){
			case ' ':
			case '\t':               /* argument separators */
				if(start != -1){
					args[ct] = &inputBuffer[start];    /* set up pointer */
					ct++;
				}
				inputBuffer[i] = '\0'; /* add a null char; make a C string */
				start = -1;
				break;
			case '\n':                 /* should be the final char examined */
				if (start != -1){
					args[ct] = &inputBuffer[start];     
					ct++;
				}
				inputBuffer[i] = '\0';
				args[ct] = NULL; /* no more arguments to this command */
				break;
			default :             /* some other character */			
				if (inputBuffer[i] == '&'){
					*background  = 1;
					inputBuffer[i] = '\0';
				} 
				else if (start == -1){
				start = i;
				}					
		} 
	}    
	args[ct] = NULL; /* just in case the input line was > MAX_LINE */
	update_background_history(*background); /*After parsing, now we can add the background information to the command history.*/
	if(args[0]==NULL){
		/*Need at least args[0] to exec.*/
		return ARGS_FAILURE;
	}
	return ARGS_SUCCESS;
	
}

int main(void) {
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background,setup_success;
	pid_t pid;
	char *args[MAX_LINE/2];
	/* equals 1 if a command is followed by '&' */
	/* command line (of 80) has max of 40 arguments */
	while (1){
		background=0;
		printf(" COMMAND->\n"); 
		clean_args(args);
		clean_inputBuffer(inputBuffer);
		if((setup_success=setup(inputBuffer,args,&background))==ARGS_FAILURE){
			/*Something when wrong parsing, skip to prompt again.*/
			continue;
		}
		
		pid=fork();
		update_pid_history(pid);
		if (pid>0){	
			/*Parent process.*/
			/*BUILT IN SHELL COMMANDS*/
			/*Note the built in commands cannot be run in the background, because they must be run in the parent.*/
			if(strcmp(args[0],"cd")==0){
				char path[MAX_LINE];
				clean_inputBuffer(path);
				/*Since setup will separate args based on spaces, if the desired directory contains spaces
				we have to concatenate them all into one large string for chdir. */
				int args_count=0;
				while(args[++args_count]!=NULL); /* Loop until args is NULL to get number of args*/
				int i;
				/* Handle special case of the ~, replace ~ with the home directory*/
				if(*args[1]=='~'){
					strcat(path,getenv("HOME"));
					args[1]++; /*make args[1] point to char after the ~*/
				} 
				/*Concatenate all non NULL args into one string for chdir */
				for(i=1;i<args_count;i++){
					strcat(path, args[i]);
					if(i!=args_count-1)strcat(path, " ");
				}				
				if(chdir(path)==-1) printf("Error with path\n");				
				continue;		
			}				
			if(strcmp(args[0],"exit")==0){
				exit(0);
			}
			if(strcmp(args[0],"history")==0){
				if(background==1){
					printf("BACKGROUND\n");
				}
				print_history();
				continue;
			}
			if(strcmp(args[0],"pwd")==0){
				char cwd[MAX_PATH_SIZE];
				/* getcwd returns NULL on error.*/
				if(getcwd(cwd, sizeof(cwd))!=NULL){
					printf("%s\n",cwd);
				}
				else{
					printf("Error getting pwd");
				}
				continue;
			}
			if(strcmp(args[0],"jobs")==0){
				int i;
				for(i=0;i<MAX_COMMAND_HISTORY;i++){
					/* Look for child processes that are running in the background. */
					if(is_alive(command_history[i].pid) && command_history[i].pid!=0){
						printf("PID %d: ",command_history[i].pid);
						int j;
						for(j=0;j<MAX_LINE;j++){
							if((command_history[i].inputBuffer)[j]!='\n')
							printf("%c",(command_history[i].inputBuffer)[j]);
						}
						printf(" is alive!\n");
					}
				}
				continue;
			}
			if(strcmp(args[0],"fg")==0){
				int status;
				/*Convert integer entered by user to pid_t.
				Then bring that process into the foreground by waiting on it.*/
				pid_t casted_pid;
				casted_pid=(pid_t) atoi(args[1]);
				waitpid(casted_pid,&status,0);
				continue;
			}
			if(background==0){
				/*Wait for child to finish if not supposed to run in the background.*/
				int status;
				waitpid(pid,&status,0);
			}

		}
		else if(pid==0){
			/*Child process.*/
			/* Ensure that built in commands are not executed by the child (must be executed by the parent).*/
			if(strcmp(args[0],"cd")!=0&&strcmp(args[0],"fg")!=0 &&
			strcmp(args[0],"jobs")!=0&&strcmp(args[0],"history")!=0 &&
			strcmp(args[0],"pwd")!=0&&strcmp(args[0],"exit")!=0 ){	
				execvp(args[0],args);
				printf("Exec failed\n");
				exit(0);
			}
			exit(0);		
		}
		else{
			printf("Fork error\n");
		}
	} 
}