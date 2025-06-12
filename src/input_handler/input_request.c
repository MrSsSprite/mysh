/*----------------------------- Private Includes -----------------------------*/
#include "input_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "Vector.h"
/*--------------------------- Private Includes END ---------------------------*/


/*----------------------------- Private Defines ------------------------------*/
#define INPUT_REQUEST_PROMPT "[mysh]$ "
/*--------------------------- Private Defines END ----------------------------*/


/*----------------------------- Public Variables -----------------------------*/
char *input_buf = NULL;
size_t input_buf_sz = 0u;
/*--------------------------- Public Variables END ---------------------------*/

/*---------------------------- Private Variables -----------------------------*/
static const char *_pattern;
/*-------------------------- Private Variables END ---------------------------*/


/*----------------------- Private Function Prototypes ------------------------*/
char **command_generator(const char *text);
char **argument_generator(const char *text);
char **readline_completion_generator(const char *text, int start, int end);
int readline_init(void);
/*--------------------- Private Function Prototypes END ----------------------*/


/*----------------------------- Public Functions -----------------------------*/
int input_request_init(void)
{
   if (readline_init()) return 1;
   return 0;
}


int input_request(void)
{
   char *line, *str;
   Vector input = vector_init(char);
   size_t i = 0, sz;
   int in_squotes = 0, in_dquotes = 0, escaped = 0;

   free(input_buf);
   input_buf_sz = 0u;
   if (input == NULL) return 1;

   do
    {
      line = readline(INPUT_REQUEST_PROMPT);
      if (line == NULL)
         break;
      if (vector_insert(input, vector_end(input), line, strlen(line)))
       {
          vector_destroy(input);
          return 1;
       }
      sz = vector_size(input);
      str = vector_begin(input);
      while (i < sz)
       {
         if (escaped) goto NO_PRETREATMENT;
         switch (str[i++])
         {
            case '\\':
               if (!in_squotes) escaped = 2;
               break;
            case '\"':
               if (!in_squotes) in_dquotes ^= 1;
               break;
            case '\'':
               if (!in_dquotes) in_squotes ^= 1;
               break;
            default:
               break;
         }
NO_PRETREATMENT:
         escaped >>= 1;
       }
      if (escaped)
       {
         escaped = 0;
         continue;
       }
    }
   while (in_squotes || in_dquotes);

   if (vector_push(input, "\0") || vector_shrink_to_fit(input))
    {
      vector_destroy(input);
      return 1;
    }
   input_buf_sz = vector_size(input);
   input_buf = vector_release(input);
   vector_destroy(input);

   return 0;
}
/*--------------------------- Public Functions END ---------------------------*/


/*---------------------------- Private Functions -----------------------------*/
int readline_init(void)
{
   rl_attempted_completion_function = readline_completion_generator;
   return 0;
}


char **readline_completion_generator(const char *text, int start, int end)
{
   if (start == 0)
      return command_generator(text);
   for (const char *it = rl_line_buffer, *ed = it + start;
        it < ed; it++)
      /* If any non-space character before the argument */
      if (*it != ' ') return argument_generator(text);
   return command_generator(text);
}


int patcmp(const char *pat, const char *ent);
int patmky(const char *pat, const char *ent)
{
   for (pat++; *ent; ent++)
      if (patcmp(pat, ent)) return 1;
   return 0;
}
int patcmp(const char *pat, const char *ent)
{
   for (; *pat && *ent; pat++, ent++)
    {
      if (*pat == '?') continue;
      if (*pat == '*') return patmky(pat, ent);
      if (*pat != *ent) return 0;
    }

   /* as if pattern always end with '*' */
   if (*pat) return 0;
   return 1;
}
int pattern_match(const struct dirent *ent)
{
   return patcmp(_pattern, ent->d_name);
}


char **command_generator(const char *text)
{
   char *envar = getenv("PATH"), *path, *full_path;
   size_t abs_path_len;
   const char *path_it;
   char *str_it;
   Vector result = vector_init(char*);
   char **ret;
   int err_code, ret_status = 0;
   struct dirent **namelist, **ent_it, **ent_ed;

   if (result == NULL) return NULL;
   if (envar)
    {
      path = strdup(envar);
      if (path == NULL)
       {
         vector_destroy(result);
         return NULL;
       }
    }
   else
      path = NULL;
   _pattern = text;

   /* TODO: check builtin-commands */
   /* files in PATH */
   for (path_it = strtok(path, ":"); path_it; path_it = strtok(NULL, ":"))
    {
      err_code = scandir(path_it, &namelist, pattern_match, alphasort);
      if (err_code == -1)
       {
         perror("command_generator: ");
         ret_status = 1;
         goto CLEANUP_SECTION;
       }
      for (ent_it = namelist, ent_ed = namelist + err_code;
           ent_it != ent_ed; ent_it++)
       {
         abs_path_len = strlen(path_it);
         err_code = path_it[abs_path_len - 1] == '/';
         full_path = malloc(abs_path_len + strlen((*ent_it)->d_name) +
                            (err_code ? 1 : 2));
         if (full_path == NULL)
          {
            perror("command_generator: ");
            ret_status = 1;
            goto CLEANUP_SECTION;
          }
         str_it = stpcpy(full_path, path_it);
         if (!err_code) *str_it++ = '/';
         strcpy(str_it, (*ent_it)->d_name);
         if (access(full_path, X_OK) == -1)
            free(full_path);
         else if (free(full_path), full_path = strdup((*ent_it)->d_name),
                  (full_path == NULL || vector_push(result, &full_path)))
          {
            free(full_path);
            while (ent_it != ent_ed)
               free(*ent_it++);
            free(namelist);
            ret_status = 1;
            goto CLEANUP_SECTION;
          }
         free(*ent_it);
       }
      free(namelist);
    }

CLEANUP_SECTION:
   free(path);

   full_path = NULL;
   if (vector_size(result) == 0)
    {
      vector_destroy(result);
      return NULL;
    }
   if (ret_status || vector_push(result, &full_path) || vector_shrink_to_fit(result))
    {
      vector_destroy(result);
      return NULL;
    }
   ret = vector_release(result);
   vector_destroy(result);
   return ret;
}


char **argument_generator(const char *text)
{
   char *tmpstr = getcwd(NULL, 0);
   struct dirent **namelist, **ent_it, **ent_ed;
   int err_code, fail_code;
   Vector result;
   char **ret;

   if (tmpstr == NULL)
      return NULL;

   _pattern = text;
   err_code = scandir(tmpstr, &namelist, pattern_match, alphasort);
   ent_it = namelist, ent_ed = namelist + err_code;
   free(tmpstr);
   if (err_code == -1)
      return NULL;

   result = vector_init(char*);
   if (result == NULL)
    {
       fail_code = 0;
       goto FAIL_CLEANUP_SECTION;
    }

   for (; ent_it != ent_ed; ent_it++)
    {
      tmpstr = strdup((*ent_it)->d_name);
      if (tmpstr == NULL || vector_push(result, &tmpstr))
       {
         fail_code = 1;
         goto FAIL_CLEANUP_SECTION;
       }
      free(*ent_it);
    }
   free(namelist);

   if (vector_size(result) == 0)
    {
      vector_destroy(result);
      return NULL;
    }
   tmpstr = NULL;
   if (vector_push(result, &tmpstr), vector_shrink_to_fit(result))
    {
      vector_destroy(result);
      return NULL;
    }
   ret = vector_release(result);
   vector_destroy(result);
   return ret;

FAIL_CLEANUP_SECTION:
   switch (fail_code)
    {
    case 1:
      vector_destroy(result);
    case 0:
      while (ent_it != ent_ed)
         free(*ent_it);
      free(namelist);
      break;
    }
   return NULL;
}
/*-------------------------- Private Functions END ---------------------------*/
