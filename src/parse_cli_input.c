/*----------------------------- Private Includes -----------------------------*/
#include "parse_cli_input.h"
#include <stdlib.h>
#include <string.h>
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline int expand_mysh_argv(void);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Variables -----------------------------*/
int mysh_argc;
char **mysh_argv;
size_t mysh_argv_sz = 0u;
/*--------------------------- Public Variables END ---------------------------*/

/*---------------------------- Exporte Functions -----------------------------*/
int parse_cmd_args(char *input)
{
   char *tmp;

   if (mysh_argv_sz == 0)
   {
      mysh_argv = malloc(sizeof(const char *) * 2);
      if (mysh_argv == NULL)
         return 1;
      mysh_argv_sz = 2u;
   }

   mysh_argv[0] = strtok(input, " ");
   if (mysh_argv[0] == NULL)
      return 2;
   for (mysh_argc = 1; (tmp = strtok(NULL, " ")); mysh_argc++)
   {
      if (mysh_argc == mysh_argv_sz && expand_mysh_argv())
         return 1;
      mysh_argv[mysh_argc] = tmp;
   }
   if (mysh_argc == mysh_argv_sz && expand_mysh_argv())
      return 1;
   mysh_argv[mysh_argc] = NULL;

   return 0;
}
/*-------------------------- Exporte Functions END ---------------------------*/

/*---------------------------- Private Functions -----------------------------*/
static inline int expand_mysh_argv(void)
{
   void *tmp = realloc(mysh_argv, sizeof *mysh_argv * mysh_argv_sz * 2);
   if (tmp == NULL) return 1;
   mysh_argv_sz *= 2;
   mysh_argv = tmp;
   return 0;
}
/*-------------------------- Private Functions END ---------------------------*/
