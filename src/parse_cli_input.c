/*----------------------------- Private Includes -----------------------------*/
#include "parse_cli_input.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Vector.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline char *varnmdup(const char **ptr);
static inline int parse_envar(const char **in_iter, Vector str);
static inline int parse_dquote(const char **in_iter, Vector str);
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
   char **argv_iter;
   const char *envar;
   int i, ret = 0, err_code;

   free(mysh_argv);
   mysh_argv = NULL;
   if (input == NULL)
   {
      vector_destroy(str);
      str = NULL;
      return 0;
   }
   if (str && vector_resize(str, 2u))
      return 1;
   else if ((str = vector_init(char)) == NULL)
      return 1;

   arg_idx = vector_init(int);
   if (arg_idx == NULL) return 1;

   for (in_iter = input; *in_iter; )
   {
      switch (*in_iter)
      {
      case '$':
         err_code = parse_envar(&in_iter, str);
         switch (err_code)
         {
         case 1:
            ret = 1;
            goto ENDING_SECTION;
         case 2:
            continue;
         default:
            ret = -1;
            goto ENDING_SECTION;
         case 0:
            break;
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
      case '\'':
         for (in_iter++; *in_iter != '\''; in_iter++)
         {
            if (*in_iter == '\0') return 2;
            vector_push(str, in_iter);
         }
         in_iter++;
         break;
      case '\"':
         err_code = parse_dquote(&in_iter, str);
         if (err_code)
         {
            ret = err_code;
            goto ENDING_SECTION;
         }
         break;
      case '\\':
         in_iter++;
      default:
         if (vector_push(str, in_iter++))
         {
            ret = 1;
            goto ENDING_SECTION;
         }
         break;
      }
   }
END_LOOP:

   if (vector_size(str) == 0)
   {
      mysh_argc = 0;
      goto ENDING_SECTION;
   }
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

   for (ed = *ptr; isalnum(*ed) || *ed == '_'; ed++) ;
   ret_len = ed - *ptr;
   ret = malloc(ret_len + 1);

   if (ret)
      strncpy(ret, *ptr, ret_len);
   ret[ret_len] = '\0';
   *ptr = ed;
   return ret;
}

static inline int parse_envar(const char **in_iter, Vector str)
{
   char *var_nm;
   const char *envar;

   ++*in_iter;
   if (**in_iter == ' ')
   {
      if (vector_push(str, "$")) return 1;
      return 0;
   }
   var_nm = varnmdup(in_iter);
   if (var_nm == NULL) return 1;
   envar = getenv(var_nm);
   free(var_nm);
   if (envar == NULL) return 2;
   if (vector_insert(str, vector_end(str), envar, strlen(envar)))
      return 1;

   return 0;
}

static inline int parse_dquote(const char **in_iter, Vector str)
{
   int err_code;
   int ch, tmpch;
   int escaped = 0;

   ++*in_iter;
   while (1)
   {
      if (escaped)
      {
         escaped = 0;
         goto DEFAULT_CASE;
      }
      switch ((ch = **in_iter))
      {
      case '$':
         err_code = parse_envar(in_iter, str);
         switch (err_code)
         {
         case 1:  return 1;
         case 2:  continue;
         default: return -1;
         case 0:  break;
         }
         break;
      case '`':
         /* TODO */
         ++*in_iter;
         break;
      case '\\':
         tmpch = *(*in_iter + 1);
         if (tmpch == '$' || tmpch == '`' || tmpch == '\"' || tmpch == '\\' ||
             tmpch == '\n')
         {
            escaped = 1;
            ++*in_iter;
         }
         else goto DEFAULT_CASE;
         break;
      case '\"':
         ++*in_iter;
         return 0;
      case '\0':
         return -1;
DEFAULT_CASE:
      default:
         if (vector_push(str, (*in_iter)++)) return 1;
         break;
      }
   }
}
/*-------------------------- Private Functions END ---------------------------*/
