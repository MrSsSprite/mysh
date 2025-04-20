/*----------------------------- Private Includes -----------------------------*/
#include "builtin.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Defines ------------------------------*/
#define RECOMMANDED_FULLPATH_SIZE   (128u)
/*--------------------------- Private Defines END ----------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
int mysh_exit(int argc, char **argv);
int mysh_echo(int argc, char **argv);
int mysh_pwd(int argc, char **argv);
int mysh_type(int argc, char **argv);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Variables -----------------------------*/
const struct fn_map builtin_fn_map[] =
{
   { .name = "echo", .fn = &mysh_echo },
   { .name = "exit", .fn = &mysh_exit },
   { .name = "pwd",  .fn = &mysh_pwd },
   { .name = "type", .fn = &mysh_type },
};
const size_t builtin_fn_map_sz = sizeof builtin_fn_map / sizeof *builtin_fn_map;
/*--------------------------- Public Variables END ---------------------------*/

/*---------------------------- Private Variables -----------------------------*/
static char *full_path = NULL;
static size_t full_path_sz = 0;
/*-------------------------- Private Variables END ---------------------------*/

/*---------------------------- Private Functions -----------------------------*/
int mysh_exit(int argc, char **argv)
{
   long exit_code;
   char *endptr;

   if (argc == 1)
      exit(0);
   if (argc != 2)
   {
      fputs("exit: too many arguments\n", stderr);
      return 1;
   }

   
   exit_code = strtol(argv[1], &endptr, 10);
   if (exit_code >= INT_MAX || exit_code <= INT_MIN)
   {
      fprintf(stderr, "exit: %ld exceeds the valid range\n", exit_code);
      return 2;
   }
   if (*endptr != '\0')
   {
      fprintf(stderr, "exit: %s is not a valid argument\n", argv[1]);
      return 3;
   }
   exit(exit_code);
}

int mysh_echo(int argc, char **argv)
{
   int err_code;

   if (argc == 1)
   {
      putchar('\n');
      return 0;
   }

   for (int i = 1; i < argc; i++)
   {
      err_code = fputs(argv[i], stdout);
      if (err_code < 0) return err_code;
      putchar(' ');
   }
   putchar('\n');

   return 0;
}

int mysh_pwd(int argc, char **argv)
{
   char *pwd = getenv("PWD");
   if (pwd == NULL) return 1;
   puts(pwd);
   return 0;
}

int mysh_type(int argc, char **argv)
{
   int has_failure = 0;
   const struct fn_map *fn_ptr;
   const char *exe_path;

   for (int i = 1; i < argc; i++)
   {
      fn_ptr = builtin_fn_find(argv[i]);
      fputs(argv[i], stdout);
      if (fn_ptr)
         puts(" is a shell builtin");
      else if ((exe_path = find_exe_full_path(argv[i])))
         printf(" is %s\n", exe_path);
      else
      {
         puts(": not found");
         has_failure = 1;
      }
   }

   return has_failure;
}
/*-------------------------- Private Functions END ---------------------------*/

/*---------------------------- Exported Functions ----------------------------*/
const struct fn_map *builtin_fn_find(const char *fn_name)
{
   const struct fn_map *iter, *const ed = builtin_fn_map + builtin_fn_map_sz;

   for (iter = builtin_fn_map; iter < ed; iter++)
      if (strcmp(fn_name, iter->name) == 0) break;
   if (iter == ed)
      return NULL;
   else
      return iter;
}

void builtin_cleanup(void)
{
   free(full_path);
   full_path_sz = 0u;
}

/**
 * @brief   return full path of a command's executable file, if any.
 *          PATH env is used
 * @param   cmd: command name
 * @return  the full path. (\c NULL if not found or any error occurred)
 * @remark  do not free the returned address directly
 */
char *find_exe_full_path(const char *cmd)
{
   int success = 0;
   const char *PATH = getenv("PATH");
   char *path;
   size_t fp_len;

   if (PATH == NULL) return NULL;
   path = strdup(PATH);
   if (path == NULL) return NULL;

   for (const char *dir = strtok(path, ":"); dir; dir = strtok(NULL, ":"))
   {
FP_ASSIGNMENT:
      fp_len = snprintf(full_path, full_path_sz,
                        dir[strlen(dir) - 1] == '/' ? "%s%s" : "%s/%s",
                        dir, cmd)
               + 1;
      if (fp_len > full_path_sz)
      {
         free(full_path);
         full_path = malloc(fp_len);
         if (full_path == NULL)
         {
            full_path_sz = 0u;
            break;
         }
         full_path_sz = fp_len;
         goto FP_ASSIGNMENT;
      }

      if (access(full_path, X_OK) == 0)
      {
         success = 1;
         break;
      }
   }

   free(path);
   if (success)
   {
      /* shrink memory allocated for full_path so that it does not occupy
       * large space after occational long path */
      if (full_path_sz > RECOMMANDED_FULLPATH_SIZE &&
          fp_len < RECOMMANDED_FULLPATH_SIZE &&
          (path = realloc(full_path, RECOMMANDED_FULLPATH_SIZE)))
      {
         full_path = path;
         full_path_sz = RECOMMANDED_FULLPATH_SIZE;
      }
      return full_path;
   }
   else
      return NULL;
}
/*-------------------------- Exported Functions END --------------------------*/
