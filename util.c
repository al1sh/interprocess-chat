#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "util.h"

/*
 * Print shell prompt. Pass the username as argument.
 */
void empty_buffer(char buffer[MSG_SIZE]){
	int i;
	for(i = 0; i < MSG_SIZE; i++){
		buffer[i] = ' ';
	}
}

void print_prompt(char *name)
{
	printf("%s >> ", name);
   fflush(NULL) ;
}

/*
 * Checks if the first string starts with the second. Return 1 if true.
 */
int starts_with(const char *a, const char *b)
{
	if (strncmp(a, b, strlen(b)) == 0)
		return 1;
	else
		return 0;
}

// makeargv
/* Taken from Unix Systems Programming, Robbins & Robbins, p37 */
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }

   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}
