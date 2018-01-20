#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include "util.h"
#include <signal.h>

//Global Variables
int cmd_server = 0 ;
int cmd_user = 0 ;
char buf[MSG_SIZE] ;
int num_users = 0 ;



/*
 * Identify the command used at the shell 
 */
int parse_command(char *buf)
{
	int cmd;
	if (starts_with(buf, CMD_CHILD_PID))
		cmd = CHILD_PID;
	else if (starts_with(buf, CMD_P2P))
		cmd = P2P;
	else if (starts_with(buf, CMD_LIST_USERS))
		cmd = LIST_USERS;
	else if (starts_with(buf, CMD_ADD_USER))
		cmd = ADD_USER;
	else if (starts_with(buf, CMD_EXIT))
		cmd = EXIT;
	else if (starts_with(buf, CMD_KICK))
		cmd = KICK;
	else
		cmd = BROADCAST;
	return cmd;
}

/*
 * List the existing users on the server shell
 */
int list_users(user_chat_box_t *users, int fd)
{
	/* 
	 * Construct a list of user names
	 * Don't forget to send the list to the requester!
	 */
	 /***** Insert YOUR code *******/
	 int i , count = 0 ;
	 char *users_list[num_users] ;
	 if (num_users <= 0) {
	 	if(write(fd, "<no users>", sizeof("<no users>"))<0)
			perror("failed to write\n");	
	 }
	 else {
	 	for (i = 0; i < MAX_USERS; i++) {	
	 		if(users[i].status == SLOT_FULL) {
	 			if(write(fd, users[i].name, sizeof(users[i].name))<0)
					perror("failed to write\n");
	 		}
	 	}
 	}
}

/*
 * Add a new user
 */
int add_user(user_chat_box_t * users, char *buf, int server_fd)
{
	/***** Insert YOUR code *******/
	
	/* 
	 * Check if user limit reached.
	 *
	 * If limit is okay, add user, set up non-blocking pipes and
	 * notify on server shell
	 *
	 * NOTE: You may want to remove any newline characters from the name string 
	 * before adding it. This will help in future name-based search.
	*/
	int new_user = -1 ;
	if (num_users < MAX_USERS) {
		num_users++ ;
		int flags ;

		/*Start: find new user index*/
		int i ;
		for(i = 0 ; i < 10 ; i++){
			if(users[i].status == SLOT_EMPTY) {
				new_user = i ;
				break ;
			}
		}
		/*End: find new user index*/
		i = 0 ;
		char * temp ;
		for (temp = strtok(buf, " \n\t") ; temp != NULL ; temp = strtok(NULL, " \n\t")) {
			i++ ;
			if (i == 2) {
				strncpy(users[new_user].name, temp, (sizeof(temp)-1)) ;
			}
		}
		//temp = extract_name(ADD_USER, buf);
		//strncpy(users[new_user].name, temp, sizeof(temp));
		//fprintf(stderr, "this is supposed name: %s\n", temp);
		users[new_user].status = SLOT_FULL ;

		pipe(users[new_user].ptoc) ;
		pipe(users[new_user].ctop) ;
		for (i = 0; i < 3; i++) {
			flags = fcntl(users[new_user].ptoc[i], F_GETFL) ;
			fcntl (users[new_user].ptoc[i], F_SETFL, flags | O_NONBLOCK) ;
			flags = fcntl(users[new_user].ctop[i], F_GETFL) ;
			fcntl (users[new_user].ctop[i], F_SETFL, flags | O_NONBLOCK) ;
		}
		char succ_msg[MSG_SIZE] ;
		sprintf(succ_msg, "%s%s%s", "Adding user, ", users[new_user].name, "...") ;
		if(write(server_fd, succ_msg, sizeof(succ_msg))<0)
			perror("failed to write\n");

		int status ;
		pid_t pid = fork() ;
		
		if(pid == 0) {
			/* Inside the child */
			/*
			 * Start an xterm with shell program running inside it.
			 * execl(XTERM_PATH, XTERM, "+hold", "-e", <path for the SHELL program>, ..<rest of the arguments for the shell program>..);
			*/

			
			/*Start: close pipe ends that are not needed*/
			
		 	/*Server Process =====> User Shell: Read*/
			close(users[new_user].ptoc[1]) ;
		 	/*Server Process <=====  User Shell: Write*/
			close(users[new_user].ctop[0]) ;
			
			/*End: close pipe ends that are not needed*/
			

			/*Start: Setup values for argv*/		
			char input_fd[MSG_SIZE] ; 
		    sprintf(input_fd, "%d", users[new_user].ptoc[0]) ;

			char output_fd[MSG_SIZE] ;
			sprintf(output_fd, "%d", users[new_user].ctop[1]) ;
			/*End: Setup values for argv*/

			
			execl(XTERM_PATH, XTERM, "+hold", "-e", SHELL_PATH, SHELL_PROG, "user", input_fd, output_fd, users[new_user].name, (char *) NULL) ;
		}
		else if(pid > 0) {
			/* Back to our main while loop for the "parent" */

			users[new_user].pid = pid ;

			/*Start: close pipe ends that are not needed*/
			/*Server Process: Write =====> User Shell*/	 	
		 	close(users[new_user].ptoc[0]) ;
		 	/*Server Process: Read <===== User Shell*/
		 	close(users[new_user].ctop[1]) ;
			/*End: close pipe ends that are not needed*/
		}
		else {
			perror("fork() failed: ") ;
			exit(-1) ; 
		}

		/* 
		 * Now read messages from the user shells (ie. LOOP) if any, then:
		 * 		1. Parse the command
		 * 		2. Begin switch statement to identify command and take appropriate action
		 *
		 * 		List of commands to handle here:
		 * 			CHILD_PID
		 * 			LIST_USERS
		 * 			P2P
		 * 			EXIT
		 * 			BROADCAST
		 *
		 * 		3. You may use the failure of pipe read command to check if the 
		 * 		user chat windows has been closed. (Remember waitpid with WNOHANG 
		 * 		from recitations?)
		 * 		Cleanup user if the window is indeed closed.
		 */
	} //close if(num_users <= 10)
	else {
		char failure_msg[MSG_SIZE] ;
		sprintf(failure_msg, "%s%s%s", "Failed to add user (", users[new_user].name, "): maximum users met") ;
		if((write(server_fd, failure_msg, sizeof(failure_msg)))<0) 
			perror("failed to write\n");
		return -1 ;
	}
	return new_user ;
}

/*
 * Broadcast message to all users. Completed for you as a guide to help with other commands :-).
 */
int broadcast_msg(user_chat_box_t *users, char *buf, int fd, char *sender)
{
	int i;
	const char *msg = "Broadcasting...", *s;
	char text[MSG_SIZE];

	/* Notify on server shell */
	if (write(fd, msg, strlen(msg) + 1) < 0)
		perror("writing to server shell\n");

	/* Send the message to all user shells */
	s = strtok(buf, "\n");
	sprintf(text, "%s: %s", sender, s);
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue ;
		if (write(users[i].ptoc[1], text, strlen(text) + 1) < 0)
			perror("write to child shell failed\n");
	}
}

/*
 * Close all pipes for this user
 */
void close_pipes(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
	close(users[idx].ptoc[0]) ;
	close(users[idx].ctop[1]) ;
}

/*
 * Cleanup single user: close all pipes, kill user's child process, kill user 
 * xterm process, free-up slot.
 * Remember to wait for the appropriate processes here!
 */
void cleanup_user(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
	num_users-- ;	
	close_pipes(idx, users) ;
	users[idx].status = SLOT_EMPTY ;
	strncpy(users[idx].name, "", strlen(users[idx].name)) ;
	kill(users[idx].child_pid, SIGKILL) ;
	kill(users[idx].pid, SIGKILL) ;

}

/*
 * Cleanup all users: given to you
 */
void cleanup_users(user_chat_box_t *users)
{
	int i ;

	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue ;
		cleanup_user(i, users) ;
	}
}

/*
 * Cleanup server process: close all pipes, kill the parent process and its 
 * children.
 * Remember to wait for the appropriate processes here!
 */
void cleanup_server(server_ctrl_t server_ctrl, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
	cleanup_users(users) ;
	kill(server_ctrl.child_pid, SIGKILL) ;
	kill(server_ctrl.pid, SIGKILL) ;
	
}

/*
 * Utility function.
 * Find user index for given user name.
 */
int find_user_index(user_chat_box_t *users, char *name)
{
	int i, user_idx = -1;

	if (name == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		if (strncmp(users[i].name, name, strlen(name)) == 0) {
			user_idx = i;
			break;
		}
	}
	return user_idx;
}

/*
 * Utility function.
 * Given a command's input buffer, extract name.
 */
char *extract_name(int cmd, char *buf)
{
	char *s = NULL;

	s = strtok(buf, " ");
	s = strtok(NULL, " ");
	if (cmd == P2P)
		return s;	/* s points to the name as no newline after name in P2P */
	s = strtok(s, "\n");	/* other commands have newline after name, so remove it*/
	return s;
}

/*
 * Send personal message. Print error on the user shell if user not found.
 */
void send_p2p_msg(int idx, user_chat_box_t *users, char *buf)
{
	/* get the target user by name (hint: call (extract_name() and send message */
	
	/***** Insert YOUR code *******/
	char name[MSG_SIZE];
	char x[MSG_SIZE];
	char msg[MSG_SIZE];
	int sendee;
	char * temp;
	strcpy(x, buf);

	
	
	strcpy(name, extract_name(P2P, buf));
	sendee=find_user_index(users, name);
	
	if(sendee==-1){
		if(write(users[idx].ptoc[1], "user does not exist", sizeof("user does not exist")+1) < 0)
			perror("failed to write\n");
	}
	temp = strtok(x, " ");
	temp = strtok(NULL, " ");
	temp = strtok(NULL, "\n");
	
	sprintf(msg, "%s: %s", users[idx].name, temp);
	
	if(write(users[sendee].ptoc[1], msg, strlen(msg)+1) < 0){
		perror("unable to send p2p message\n");
	}
}

int main(int argc, char **argv)
{
	
	/***** Insert YOUR code *******/
	int i, flags ;
	user_chat_box_t userArr[MAX_USERS] ;
	char buf_pid[MSG_SIZE] ;
	//Allocate space for our Array: userArr, where each indice is an instance of a user chat box 
	for(i = 0 ; i < MAX_USERS; i++) {
		userArr[i].status = SLOT_EMPTY ;
	}
	
	/*Start: open non-blocking bi-directional pipes for communication with server shell*/
	server_ctrl_t server_shell ;
	pipe(server_shell.ptoc) ;
	pipe(server_shell.ctop) ;
	for (i = 0; i < 3; i++) {
		flags = fcntl(server_shell.ptoc[i], F_GETFL) ;
		fcntl (server_shell.ptoc[i], F_SETFL, flags | O_NONBLOCK) ;
		flags = fcntl(server_shell.ctop[i], F_GETFL) ;
		fcntl (server_shell.ctop[i], F_SETFL, flags | O_NONBLOCK) ;
	}
	/*End: open non-blocking bi-directional pipes for communication with server shell*/

	/* Fork the server shell */
	pid_t pid;
	pid = fork() ;
	
	/* 
 	 * Inside the child.
	 * Start server's shell.
 	 * exec the SHELL program with the required program arguments.
 	 */
	if(pid == 0) {
		/*Start: close pipe ends that are not needed*/
	 	/*Server Process =====> Server Shell: Read*/
		close(server_shell.ptoc[1]) ;
	 	/*Server Process <=====  Server Shell: Write*/
		close(server_shell.ctop[0]) ;
		/*End: close pipe ends that are not needed*/

		/*Start: Setup values for argv*/		
		//sprinf()
		char input_fd[MSG_SIZE] ; 
	    sprintf(input_fd, "%d", server_shell.ptoc[0]) ;

		//sprintf()
		char output_fd[MSG_SIZE] ;
		sprintf(output_fd, "%d", server_shell.ctop[1]) ;
		/*End: Setup values for argv*/
		
		//Start the SERVER SHELL
		execl(SHELL_PATH, SHELL_PROG, input_fd, output_fd, "Server", (char *) NULL) ;

		return 0 ;
	 }
	/* Inside the parent. This will be the most important part of this program. */
	 else if (pid > 0) {
	 	//Assing the values of the pid to our struct
		server_shell.pid = pid ;

		/*Start: close pipe ends that are not needed*/
		/*Server Process: Write =====> Server Shell*/	 	
	 	close(server_shell.ptoc[0]) ;
	 	/*Server Process: Read <===== Server Shell*/
	 	close(server_shell.ctop[1]) ;
		/*End: close pipe ends that are not needed*/

		/* Start a loop which runs every 1000 usecs.
	 	 * The loop should read messages from the server shell, parse them using the 
	 	 * parse_command() function and take the appropriate actions. */
		int live = true ;
		while (live) {
			/* Let the CPU breathe */
			usleep(1000) ;

			/* 
			 * 1. Read the message from server's shell, if any
			 * 2. Parse the command
			 * 3. Begin switch statement to identify command and take appropriate action
			 *
			 * 		List of commands to handle here:
			 * 			CHILD_PID
			 * 			LIST_USERS
			 * 			ADD_USER
			 * 			KICK
			 * 			EXIT
			 * 			BROADCAST 
			 */
			if (read(server_shell.ctop[0], buf, MSG_SIZE) > 0) {
				cmd_server = parse_command(buf) ;
				char *temp ;
				int index ;
				char user_name[MSG_SIZE] ;
				printf("%d /n", cmd_server);
				 switch(cmd_server) {
					case CHILD_PID:
						//server_shell.child_pid = getpid();
						strcpy(buf_pid, extract_name(cmd_user, buf)) ;
						server_shell.child_pid = atoi(buf_pid) ;
						empty_buffer(buf_pid) ;
						break ;
					// Send a list of users to the shell of the requester
					case LIST_USERS:
						list_users(userArr, server_shell.ptoc[1]) ;
						break ;
					// Add a user (open a new xterm)	
					case ADD_USER:
						index = add_user(userArr, buf, server_shell.ptoc[1]) ;
						break ;
					// Kick the spcified user (a..k.a. kill the specified users process/shell)	
					case KICK:
						// extract user name
						strcpy(user_name, extract_name(KICK, buf));
						
						//find_user_index(user_chat_box_t *users, char *name)
						int curr_user = find_user_index(userArr, user_name) ;
						if(curr_user == -1) {
							perror("User does not exit.") ;
							break ;
						}
						cleanup_user(curr_user, userArr) ;
						break ;
					// If request from a user shell, just Kick them.  Otherwise, kick all.	
					case EXIT:
						// kill children before we kill ourselves
						live = false ;
						cleanup_server(server_shell, userArr) ;
						break ;
					// Broadcast the given string to the user shells (Server >> Broadcasting...)	
					case BROADCAST:
						broadcast_msg(userArr, buf, server_shell.ptoc[1], "Server") ;
						break ;
				 }
			}	
			/*Start: Checking for user input and acting accordingly */
			for (i = 0; i < MAX_USERS; i++) {
				if (userArr[i].status == SLOT_FULL) {						
					if (read(userArr[i].ctop[0], buf, MSG_SIZE) > 0) {
						cmd_user = parse_command(buf) ;
						switch(cmd_user) {
							case CHILD_PID:
								strcpy(buf_pid, extract_name(cmd_user, buf)) ;
								userArr[i].child_pid = atoi(buf_pid) ;
								empty_buffer(buf_pid) ;
								break ;
							// Send a list of users to the shell of the requester
							case LIST_USERS:
								list_users(userArr, userArr[i].ptoc[1]) ;
								break ;
							// If p2p command is selected, send message to specific user
							case P2P:
								send_p2p_msg(i, userArr, buf);
								break ;	
							// If request from a user shell, just Kick them.  Otherwise, kick all.	
							case EXIT:
								// kill ourselves
								cleanup_user(i, userArr) ;
								break ;
							// Broadcast the given string to the user shells (Server >> Broadcasting...)	
							case BROADCAST:
								broadcast_msg(userArr, buf, server_shell.ptoc[1], userArr[i].name);
								break ;
						}
					}//end of if( read() > 0)
					else { //if(read() < 0) -> the user is no longer there???
						//Check to see if the xterm has exited abruptly
						int status ;
						if(waitpid(userArr[i].pid, &status, WNOHANG) == -1) {
							//cleanup the user that is no longer functioning properly
							cleanup_user(i, userArr) ;
						}
					}
				}
			}/*End: Checking for user input and acting accordingly */
		}/* while loop ends when server shell sees the \exit command */
	}
	// fork failed
	else {
		fprintf(stderr, "Fork failed!\n") ;
		exit(-1) ;
	}
	return 0;
}
