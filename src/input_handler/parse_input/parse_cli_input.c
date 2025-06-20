/*----------------------------- Private Includes -----------------------------*/
#include "parse_cli_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Vector.h"
#include "redirection.h"
#include "parse_fns.h"
#include "parse_redir.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Variables -----------------------------*/
int mysh_argc = 0;
char **mysh_argv = NULL;
void *parsed_redir_table = NULL;
/*--------------------------- Public Variables END ---------------------------*/

/*---------------------------- Private Variables -----------------------------*/
Vector redir_table = NULL;
/*-------------------------- Private Variables END ---------------------------*/

/*---------------------------- Exporte Functions -----------------------------*/
void parse_cli_input_cleanup(void)
{
   parse_cmd_args(NULL);
}

int parse_cmd_args(const char *input)
{
   static Vector str = NULL;
   Vector arg_idx;
   const char *in_iter;
   char **argv_iter;
   int i, ret = 0, err_code, in_arg;

   /* Dynamic Memory Resources handling --------------------------------------*/
   free(mysh_argv);
   mysh_argv = NULL;
   if (input == NULL)
   {
      if (str) vector_destroy(str);
      if (redir_table) redir_table_destroy();
      str = NULL;
      redir_table = NULL;
      return 0;
   }
   if (str && vector_resize(str, 2u))
      return 1;
   else if ((str = vector_init(char)) == NULL)
      return 1;

   if (redir_table && redir_table_clear())
      return 1;
   else if ((redir_table = vector_init(struct redir_info)) == NULL)
      return 1;

   arg_idx = vector_init(int);
   if (arg_idx == NULL) return 1;

   /* Iterate and process the input string -----------------------------------*/
   /* skip preceding spaces */
   in_arg = 0;
   in_iter = input;
   while (*in_iter)
   {
      err_code = parse_token(&in_iter, str);
      switch (err_code)
      {
      case 0:
         in_arg = 1;
         break;
      case 10:
         if (in_arg)
          {
            i = vector_size(str);
            vector_push(arg_idx, &i);
            in_arg = 0;
          }
         break;
      case 11:
         if (in_arg && vector_size(arg_idx) == 0)
          {
            i = vector_size(str) + 1;
            if (vector_push(str, "\0") || vector_push(arg_idx, &i))
             {
                ret = 1;
                goto ENDING_SECTION;
             }
            in_arg = 0;
          }
         ret = parse_redirection(&in_iter, str);
         while (ret == 11)
            ret = parse_redirection(&in_iter, NULL);
         if (ret == 10)
         {
            i = vector_size(str);
            vector_push(arg_idx, &i);
         }
         else if (ret)
            goto ENDING_SECTION;
         ret = 0;
         break;
      /* Errors */
      case 1:
      case 2:
         ret = err_code;
         goto ENDING_SECTION;
      case 20:
         fputs("source code error: missing case handler for parse_envar "
               "return\n", stderr);
         ret = 20;
         goto ENDING_SECTION;
      case 21:
         fputs("source code error: missing case handler for parse_dquote "
               "return\n", stderr);
         ret = 20;
         goto ENDING_SECTION;
      case 22:
         fputs("source code error: parse_token: return outside switch\n",
               stderr);
         ret = 20;
         goto ENDING_SECTION;
      }
   }

   if (vector_size(str) == 0)
   {
      mysh_argc = 0;
      goto ENDING_SECTION;
   }
   /* end str with '\0' */
   if (in_arg)
    {
      if (vector_push(str, "\0"))
       {
         ret = 1;
         goto ENDING_SECTION;
       }
    }
   else if (vector_size(arg_idx))
      vector_pop(arg_idx);
   /* assign approriate values to mysh_argc and mysh_argv */
   mysh_argc = vector_size(arg_idx) + 1;
   mysh_argv = malloc(sizeof *mysh_argv * (mysh_argc + 1));
   if (mysh_argv == NULL)
   {
      ret = 1;
      goto ENDING_SECTION;
   }
   argv_iter = mysh_argv;
   *argv_iter++ = vector_begin(str);
   fputs("arg_idx: {", stdout);
   for (int *iter = vector_begin(arg_idx), *ed = vector_end(arg_idx);
        iter < ed; iter++)
      printf("%d, ", *iter);
   puts("}");
   for (int *iter = vector_begin(arg_idx), *ed = vector_end(arg_idx);
        iter < ed; iter++)
      *argv_iter++ = mysh_argv[0] + *iter;
   /* last ptr is NULL so that it can be used by execv directly */
   *argv_iter = NULL;

ENDING_SECTION:
   parsed_redir_table = redir_table;
   vector_destroy(arg_idx);
   return ret;
}
/*-------------------------- Exporte Functions END ---------------------------*/
