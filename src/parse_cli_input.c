/*----------------------------- Private Includes -----------------------------*/
#include "parse_cli_input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Vector.h"
#include "redirection.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline char *varnmdup(const char **ptr);
static inline int parse_envar(const char **in_iter, Vector str);
static inline int parse_dquote(const char **in_iter, Vector str);
static inline int parse_token(const char **in_iter, Vector str);
static inline int parse_redir_output(const char **in_iter, Vector str);
static int redir_table_clear(void);
static void redir_table_destroy(void);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Variables -----------------------------*/
int mysh_argc = 0;
char **mysh_argv = NULL;
void *parsed_redir_table = NULL;
/*--------------------------- Public Variables END ---------------------------*/

/*---------------------------- Private Variables -----------------------------*/
static Vector redir_table = NULL;
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
   int i, ret = 0, err_code;

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
   for (in_iter = input; *in_iter && *in_iter == ' '; in_iter++) ;
   while (*in_iter)
   {
      err_code = parse_token(&in_iter, str);
      switch (err_code)
      {
      case -1:
         ret = 0;
         goto END_LOOP;
      case 0:
         break;
      case 1:
      case 2:
         ret = err_code;
         goto ENDING_SECTION;
      case 10:
         i = vector_size(str);
         vector_push(arg_idx, &i);
         break;
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
END_LOOP:

   if (vector_size(str) == 0)
   {
      mysh_argc = 0;
      goto ENDING_SECTION;
   }
   /* end str with '\0' */
   vector_push(str, "\0");
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
   for (int *iter = vector_begin(arg_idx), *ed = vector_end(arg_idx);
        iter < ed; iter++)
      *argv_iter++ = mysh_argv[0] + *iter;
   /* last ptr is NULL so that it can be used by execv directly */
   *argv_iter = NULL;
   parsed_redir_table = redir_table;
   return 0;

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
         default: return 20;
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
         return 31;
DEFAULT_CASE:
      default:
         if (vector_push(str, (*in_iter)++)) return 1;
         break;
      }
   }
}

static inline int parse_token(const char **in_iter, Vector str)
{
   int err_code;
   const char *envar;

   switch (**in_iter)
   {
   case '$':
      err_code = parse_envar(in_iter, str);
      switch (err_code)
      {
         case 1:
            return 1;
         case 2:
            ++*in_iter;
            return 0;
         default:
            return 20;
         case 0:
            return 0;
      }
   case '~':
      ++*in_iter;
      envar = getenv("HOME");
      if (envar == NULL) return 0;
      if (vector_insert(str, vector_end(str), envar, strlen(envar)))
         return 1;
      break;
   case ' ':
      /* skip contiguous spaces */
      while (*++*in_iter == ' ') ;
      /* end loop if it's merely a sequence of spaces at the end */
      if (**in_iter == '\0') return 0;
      /* separate args */
      if (vector_push(str, "\0"))
         return 1;
      /* Should update arg_idx */
      return 10;
   case '\'':
      for (++*in_iter; **in_iter != '\''; ++*in_iter)
      {
         if (**in_iter == '\0') return 2;
         vector_push(str, *in_iter);
      }
      ++*in_iter;
      return 0;
   case '\"':
      return parse_dquote(in_iter, str);
   case '>':
      /* TODO */
      return parse_redir_output(in_iter, str);
   case '\\':
      ++*in_iter;
   default:
      if (vector_push(str, (*in_iter)++))
         return 1;
      return 0;
   }

   /* always return in switch */
   return 22;
}

static inline int parse_redir_output(const char **in_iter, Vector str)
{
   int fd_des;
   struct redir_info redir_info;
   const char *str_red, *str_iter;
   Vector filename;
   int err_code, ret = 0;

   if (*++*in_iter == '\0')
   {
      fputs("mysh: syntax error near unexpected token `newline'", stderr);
      return 30;
   }
   else if (**in_iter == ' ')
   {
      /* skip separating spaces */
      while (*++*in_iter == ' ') ;
      if (**in_iter == '\0')
      {
         fputs("mysh: syntax error near unexpected token `newline'", stderr);
         return 30;
      }
   }

   /* reading filename which will be opened for file descriptor */
   filename = vector_init(char);
   if (filename == NULL) return 1;
   while (**in_iter)
      switch (err_code = parse_token(in_iter, filename))
      {
      case 0:
         break;
      case 1:
      case 2:
         ret = err_code;
         goto ENDING_SECTION;
      case 10:
         ret = 0;
         goto END_LOOP;
      case 22:
      default:
         return 20;
      }
END_LOOP:
   if (*((char*)vector_end(filename) + 1) != '\0'
       && vector_push(filename, "\0"))
      return 1;

   str_iter = (char*)vector_end(str) - 1;
   str_red = (char*)vector_begin(str) - 1;
   /* file descriptor redirected should be 1 if not specified */
   if (str_iter == str_red || *str_iter == '\0')
      fd_des = 1;
   else
   {
      /* parse file descriptor */
      fd_des = 0;
      for (int exp = 1; str_iter > str_red && *str_iter; str_iter--, exp *= 10)
      {
         if (!isdigit(*str_iter))
         {
            fd_des = 1;
            /* treat all characters before '>' as an individual argument */
            if (vector_push(str, "\0"))
            {
               ret = 1;
               goto ENDING_SECTION;
            }
            /* inform main parser to update arg_idx */
            ret = 10;
            break;
         }
         fd_des += (*str_iter - '0') * exp;
      }
      str_iter++;
      vector_erase(str, (void*)str_iter, (char*)vector_end(str) - str_iter);
   }

ENDING_SECTION:
   if (ret && (ret > 19 || ret < 10))
      vector_destroy(filename);
   else
   {
      /* release filename and store in redirection list */;
      redir_info.type = REDIR_PATH_FD | REDIR_OUTPUT;
      vector_shrink_to_fit(filename);
      redir_info.filename = vector_release(filename);
      redir_info.des = fd_des;
      vector_push(redir_table, &redir_info);
   }
   return ret;
}


#define redir_table_submem_cleanup() do { \
   for (struct redir_info *iter = vector_begin(redir_table), \
                          *ed = vector_end(redir_table); \
        iter < ed; iter++) \
   { \
      if (iter->type & REDIR_PATH_FD) \
         free((void*)iter->filename); \
   } \
} while (0)

static int redir_table_clear(void)
{
   redir_table_submem_cleanup();
   return vector_resize(redir_table, 0u);
}


static void redir_table_destroy(void)
{
   redir_table_submem_cleanup();
   vector_destroy(redir_table);
}
/*-------------------------- Private Functions END ---------------------------*/
