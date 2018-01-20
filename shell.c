#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "util.h"

/* Global Variables */
char buf[MSG_SIZE] ;
char name[MSG_SIZE] ;

/*
 * Do stuff with the shell input here.
 */
int sh_handle_input(char *line, int fd_toserver)
{
	
	/***** Insert YOUR code *******/
	
 	/* Check for \seg command and create segfault */
 	
	/* Write message to server for processing */
	if(is_empty(line)){
		print_prompt(name);
	}
	else{
	 	if(starts_with(line, CMD_SEG)){
			char *n = NULL;
			*n = 1;
		}
		if(write(fd_toserver, line, MSG_SIZE)<0)
			perror("failed to write\n");
		empty_buffer(buf);
	}
	return 0;
}

/*
 * Check if the line is empty (no data; just spaces or return)
 */
int is_empty(char *line)
{
	while (*line != '\0') {
		if (!isspace(*line))
			return 0;
		line++;
	}
	return 1;
}

int main(int argc, char **argv)
{
	
	/***** Insert YOUR code *******/
	int i ;
	char temp[MSG_SIZE];
	/* Extract pipe descriptors and name from argv */

	//Retreive pipes from argv!
	int fd_read, fd_write ;
	if (0 == strncmp(argv[2], "user", sizeof("user"))) {
		fd_read = atoi(argv[3]) ;
		fd_write = atoi(argv[4]) ;
		strncpy(name, argv[5], sizeof(argv[5]));
	}
	else {
		fd_read = atoi(argv[1]) ;
		fd_write = atoi(argv[2]) ;
		strncpy(name, argv[3], sizeof(argv[5])) ;
	}

	/* Fork a child to read from the pipe continuously */
	pid_t pid ;
	int status ;
	pid = fork() ;
	if(pid == 0) {
		
		//if(0 == strncmp(name, "Server", sizeof("Server"))){
		sprintf(temp, "%s %d", CMD_CHILD_PID, getpid()) ;
		if(write(fd_write, temp, sizeof(temp)) <0)
			perror("failed to write\n");
		empty_buffer(temp) ;

		//Start while() loop for writing to the Server Process
		while(true) {
			/*
			 * Once inside the child
			 * look for new data from server every 1000 usecs and print it
			 */ 
			usleep(1000) ;
			if (read(fd_read, buf, MSG_SIZE) > 0){
				printf("%s\n", buf) ;
			}
		}
	}
	else if(pid > 0) {
		/* Inside the parent
		 * Send the child's pid to the server for later cleanup
		 * Start the main shell loop
		 */

		//Start while() loop for writing to the Server Process
		print_prompt(name) ;
		while(true) {
			// Give the processor a break!
			usleep(1000) ;

			if(read(0, buf, MSG_SIZE) > 0) {
				sh_handle_input(buf, fd_write);
			}
		}
	}
	else {
		perror("fork() failed: ") ;
		exit(-1) ;
	}
	
	return 0 ;
}
