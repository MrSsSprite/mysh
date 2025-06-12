/*---------------------------- Privavte Includes -----------------------------*/
#include "parse_fns.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Vector.h"
/*-------------------------- Privavte Includes END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static inline char *varnmdup(const char **ptr);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Functions -----------------------------*/
inline int parse_envar(const char **in_iter, Vector str)
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

int parse_dquote(const char **in_iter, Vector str)
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

int parse_token(const char **in_iter, Vector str)
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
      return 0;
   case ' ':
      /* skip contiguous spaces */
      while (*++*in_iter == ' ') ;
      /* end loop if it's merely a sequence of spaces at the end */
      if (**in_iter == '\0') return 0;
      /* separate args */
      if (vector_push(str, "\0"))
         return 1;
      /* Tell caller that it's a space */
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
   case '<':
   case '>':
      return 11;
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
/*--------------------------- Public Functions END ---------------------------*/

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
/*-------------------------- Private Functions END ---------------------------*/
