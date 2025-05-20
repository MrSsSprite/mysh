/*----------------------------- Private Includes -----------------------------*/
#include "redirection.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "List.h"
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Structs ------------------------------*/
struct redir_remap
{
   int bkup, orig;
};
/*--------------------------- Private Structs END ----------------------------*/

/*---------------------------- Private Variables -----------------------------*/
static List redir_backup;
/*-------------------------- Private Variables END ---------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
int redir_bkup_cmp(const void *lhs, const void *rhs);
/*--------------------- Private Function Prototypes END ----------------------*/

/*----------------------------- Public Functions -----------------------------*/
int redirect_init(void)
{
   redir_backup = list_init(struct redir_remap);
   if (redir_backup == NULL)
      return 1;
   return 0;
}

void redirect_cleanup(void)
{
   list_destroy(redir_backup);
}

int redirect_vec(Vector redir_table)
{
   int err_code;

   for (struct redir_info *iter = vector_begin(redir_table),
                          *ed = vector_end(redir_table);
        iter < ed; iter++)
   {
      if ((err_code = redirect(*iter)))
         return err_code;
   }
   return 0;
}

int redirect(struct redir_info info)
{
   int tmp_fd;
   int oflags;
   List_Node bkup_node;

   if (info.type & REDIR_NODES)
      return close(info.srcfd);

   /* if des fd is currently opened */
   if (!(info.type & REDIR_NOKEEP) && fcntl(info.des, F_GETFD) != -1)
   {
      bkup_node = (void*)list_find(redir_backup, list_head(redir_backup),
                                   &info.des, redir_bkup_cmp);
      if (bkup_node)
         bkup_node = *(List_Node*)bkup_node;
      else
      {
         if (list_push(redir_backup, NULL))
            return 1;
         bkup_node = *list_head(redir_backup);
         ((struct redir_remap*)bkup_node->value)->orig = info.des;
      }
      /* backup the original des fd */
      tmp_fd = fcntl(info.des, F_DUPFD, 0);
      if (tmp_fd == -1)
      {
         perror("redirect: ");
         return -1;
      }
      ((struct redir_remap*)bkup_node->value)->bkup = tmp_fd;
   }

   if (info.type & REDIR_PATH_FD)
   {
      if (info.type & REDIR_OUTPUT)
      {
         oflags = O_CREAT;
         if (info.type & REDIR_APPEND)
            oflags |= O_APPEND;
         else
            oflags |= O_TRUNC;

         if (info.type & REDIR_INPUT)
            oflags |= O_RDWR;
         else
            oflags |= O_WRONLY;
      }
      else
         oflags = O_RDONLY;

      tmp_fd = open(info.filename, oflags, 0754);
      if (tmp_fd == -1)
      {
         perror("open: ");
         return -1;
      }
   }
   else
      tmp_fd = info.srcfd;

   if (dup2(tmp_fd, info.des) == -1)
   {
      perror("dup2: ");
      return -1;
   }
   if (info.type & REDIR_PATH_FD && close(tmp_fd) == -1)
   {
      perror("close: ");
      return -1;
   }

   return 0;
}

int redir_recover(void)
{
   struct redir_remap *bkup_info;
   List_Node node;

   while ((node = *list_head(redir_backup)))
   {
      bkup_info = (void*)node->value;
      if (dup2(bkup_info->bkup, bkup_info->orig) == -1)
         return -1;
      list_pop(redir_backup);
   }

   return 0;
}
/*--------------------------- Public Functions END ---------------------------*/

/*---------------------------- Privatet Functions ----------------------------*/
int redir_bkup_cmp(const void *lhs, const void *rhs)
{
   return ((struct redir_remap*)lhs)->bkup - ((struct redir_remap*)rhs)->bkup;
}
/*-------------------------- Privatet Functions END --------------------------*/
