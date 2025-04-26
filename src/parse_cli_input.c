/*----------------------------- Private Includes -----------------------------*/
#include "parse_cli_input.h"
#include <stdlib.h>
#include <string.h>
#include "Vector.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline char *varnmdup(const char **ptr);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Variables -----------------------------*/
int mysh_argc;
char **mysh_argv = NULL;
/*--------------------------- Public Variables END ---------------------------*/

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
   char *var_nm, **argv_iter;
   const char *envar;
   int i, ret = 0;

   if (str) vector_destroy(str);
   free(mysh_argv);
   mysh_argv = NULL;
   if (input == NULL)
   {
      str = NULL;
      return 0;
   }
   str = vector_init(char);
   if (str == NULL) return 1;

   arg_idx = vector_init(int);
   if (arg_idx == NULL) return 1;

   for (in_iter = input; *in_iter; )
   {
      switch (*in_iter)
      {
      case '$':
         in_iter++;
         if (*in_iter == ' ')
         {
            in_iter--;
            goto COPY_CHAR;
         }
         var_nm = varnmdup(&in_iter);
         if (var_nm == NULL)
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         envar = getenv(var_nm);
         free(var_nm);
         if (envar == NULL) continue;
         if (vector_insert(str, vector_end(str), envar, strlen(envar)))
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         break;
      case '~':
         in_iter++;
         envar = getenv("HOME");
         if (envar == NULL) continue;
         if (vector_insert(str, vector_end(str), envar, strlen(envar)))
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         break;
      case ' ':
         /* skip contiguous spaces */
         in_iter++;
         while (*in_iter == ' ') in_iter++;
         /* pass to END_LOOP if it's merely a sequence of spaces at the end */
         if (*in_iter == '\0') goto END_LOOP;
         /* separate args */
         if (vector_push(str, "\0"))
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         i = vector_size(str);
         vector_push(arg_idx, &i);
         break;
      case '\\':
         in_iter++;
      default:
COPY_CHAR:
         if (vector_push(str, in_iter++))
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         break;
      }
   }
END_LOOP:

   /* end str with '\0' */
   vector_push(str, "\0");
   /* TODO: assign approriate values to mysh_argc and mysh_argv */
   mysh_argc = vector_size(arg_idx) + 1;
   mysh_argv = malloc(sizeof *mysh_argv * (mysh_argc + 1));
   if (mysh_argv == NULL)
   {
      ret = 1;
      goto ENDING_SECTION;
   }
   argv_iter = mysh_argv;
   *argv_iter++ = vector_begin(str);
   for (int *iter = vector_begin(arg_idx), *ed = vector_end(arg_idx);
        iter < ed; iter++)
      *argv_iter++ = mysh_argv[0] + *iter;
   /* last ptr is NULL so that it can be used by execv directly */
   *argv_iter = NULL;

ENDING_SECTION:
   vector_destroy(arg_idx);
   return ret;
}
/*-------------------------- Exporte Functions END ---------------------------*/

/*---------------------------- Private Functions -----------------------------*/
static inline char *varnmdup(const char **ptr)
{
   const char *ed;
   size_t ret_len;
   char *ret;

   for (ed = *ptr; *ed != '\0' && *ed != ' ' && *ed != '/'; ed++) ;
   ret_len = ed - *ptr;
   ret = malloc(ret_len + 1);

   if (ret)
      strncpy(ret, *ptr, ret_len);
   ret[ret_len] = '\0';
   *ptr = ed;
   return ret;
}
/*-------------------------- Private Functions END ---------------------------*/
