
Purpose:
  This project is supposed to build on our knowledge of forks, and in term learn about pipes. Pipes are used for interprocess communication in this project.  We rely on pipes to allows our server process to communicate with both our server shell and our user shells.  

How it works:
  Our program is broken down into two main parts, the server and the shell.  The server contains the server process (handles all input from the original user the server shell and the user shell) as well as handling the instantiation of the server shell and user shells.  The shell program is what handles the super user and regular users input and sends it to the server process and waits for feedback.  
  
  To following are instructions on how to run our program:
      - compile -> make all
      - run -> ./server
      - clean -> clean all
  
  From here, the server shell will be started in your terminal.  This server shell will have some commands that can be used, and they are as follows:
      - \add user -> add a new user with the name "user"
      - \list -> list all active users
      - \kick user -> kick a user with the name "user"
      - \exit -> close the entire program and kill all running processes within
      - message -> type a message you want to send to all active users 
  
  In the even that a user shell is added, similar commands can be used and they are follows:
      - \list -> list all active users 
      - \p2p user -> send a peer to peer message to a user with name "user"
      - \exit -> exit or leave the chat server (kills all processes related to the user shell)
      - \seg -> create a seg fault and see how the program will handle it accordingly
      - message -> type a message you want to send to all active users
      
Assumptions:
  We have made a few assumptions when it came to our program, and they are as follows:
    - We do not need to handle a segmentation fault if the \kick command is given in the server shell with no user specified
    - We do not need to handle process cleanup of a user when the close (x-button) is hit on the users shell

Error Handling:
	for each and every write statement that is done, we include it into an if statement that checks if its value is less than zero, if so then we output a perror with a message. otherwise it continues regularly. 
	
	within p2p, if the name of the person the message is being sent to. the user index of our variable would then be -1, thus falling into our if statement which outputs an error message and kills the p2p function
	
	another error handling move, was to change all the pipes to nonblocking so that it is not constantly looking one input, rather it looks, sees that there is none, and returns in 100 microseconds to check again.
	
	finally, we checked to see if each user is still alive and able to be read from. if a read fails from a specific user that means the user is dead and must be handled appropriately.
	
	
  
  
      
  
  
