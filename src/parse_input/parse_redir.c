/*----------------------------- Private Includes -----------------------------*/
#include "parse_redir.h"
#include "redirection.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "parse_fns.h"
#include <string.h>
/*--------------------------- Private Includes END ---------------------------*/

/*------------------------------ Private Macros ------------------------------*/
#define redir_table_submem_cleanup() do { \
   for (struct redir_info *iter = vector_begin(redir_table), \
                          *ed = vector_end(redir_table); \
        iter < ed; iter++) \
   { \
      if (iter->type & REDIR_PATH_FD) \
         free((void*)iter->filename); \
   } \
} while (0)
/*---------------------------- Private Macros END ----------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline enum redir_type parse_redir_get_oflag(const char **in_iter);
/*--------------------- Private Function Prototypes END ----------------------*/

/*------------------------- Private Extern Variables -------------------------*/
extern Vector redir_table;
/*----------------------- Private Extern Variables END -----------------------*/

/*----------------------------- Public Functions -----------------------------*/
int parse_redirection(const char **in_iter, Vector str)
{
   int fd_des;
   struct redir_info redir_info;
   const char *str_red, *str_iter;
   Vector filename = NULL;
   int err_code, ret = 0;

   /* parse fd_des */
   str_iter = (char*)vector_end(str) - 1;
   str_red = (char*)vector_begin(str) - 1;
   /* file descriptor redirected should be 0 or 1 if not specified */
   if (str_iter == str_red || *str_iter == '\0')
      fd_des = **in_iter == '<' ? 0 : 1;
   else
   {
      /* parse file descriptor */
      fd_des = 0;
      for (int exp = 1; str_iter > str_red && *str_iter; str_iter--, exp *= 10)
      {
         if (!isdigit(*str_iter))
         {
            /* treat all characters before '>' as an individual argument */
            fd_des = **in_iter == '<' ? 0 : 1;
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

   redir_info.type = parse_redir_get_oflag(in_iter);

   if (**in_iter == '\0')
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

   if (**in_iter == '&')
   {
      redir_info.type |= REDIR_FD_FD;
      ++*in_iter;
   }
   else
      redir_info.type |= REDIR_PATH_FD;
   /* read filename which will be opened for file descriptor */
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
      case 11:
         goto END_LOOP;
      case 22:
      default:
         return 20;
      }
END_LOOP:
   if (*((char*)vector_end(filename) + 1) != '\0'
       && vector_push(filename, "\0"))
   {
      ret = 1;
      goto ENDING_SECTION;
   }


ENDING_SECTION:
   if (ret && (ret > 19 || ret < 10))
      vector_destroy(filename);
   else
   {
      /* release filename and store in redirection list */;
      vector_shrink_to_fit(filename);
      redir_info.filename = vector_release(filename);
      redir_info.des = fd_des;
      vector_push(redir_table, &redir_info);
   }
   return ret;
}

int redir_table_clear(void)
{
   redir_table_submem_cleanup();
   return vector_resize(redir_table, 0u);
}


void redir_table_destroy(void)
{
   redir_table_submem_cleanup();
   vector_destroy(redir_table);
}
/*--------------------------- Public Functions END ---------------------------*/

/*---------------------------- Private Functions -----------------------------*/
static inline enum redir_type parse_redir_get_oflag(const char **in_iter)
{
   enum redir_type ret = 0u;

   if (*(*in_iter)++ == '<')
   {
      ret |= REDIR_INPUT;
      switch (*(*in_iter)++)
      {
      case '>':
         ret |= REDIR_OUTPUT;
         break;
      case '<':
         /* TODO: Here Document */
      default:
         --*in_iter;
         break;
      }
   }
   else
   {
      ret |= REDIR_OUTPUT;
      switch (*(*in_iter)++)
      {
      case '>':
         ret |= REDIR_APPEND;
         break;
      default:
         --*in_iter;
         break;
      }
   }

   return ret;
}
/*-------------------------- Private Functions END ---------------------------*/
