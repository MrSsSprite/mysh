#ifndef REDIRECTION_H
#define REDIRECTION_H

/*----------------------------- Public Includes ------------------------------*/
#include "Vector.h"
/*--------------------------- Public Includes END ----------------------------*/

/*------------------------------- Public Enums -------------------------------*/
enum redir_type
{
   REDIR_FD_FD = 0x0u,
   REDIR_PATH_FD = 0x1u,
   REDIR_NODES = 0x2u,
   REDIR_NOKEEP = 0x8u,
   REDIR_INPUT = 0x10u,
   REDIR_OUTPUT = 0x20u,
   REDIR_APPEND = 0X40u,
};
/*----------------------------- Public Enums END -----------------------------*/

/*------------------------------ Public Structs ------------------------------*/
struct redir_info
{
   union
   {
      const char *filename;
      int srcfd;
   };
   int des;
   enum redir_type type;
};
/*---------------------------- Public Structs END ----------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
int redirect_init(void);
void redirect_cleanup(void);
int redirect_vec(Vector redir_table);
int redirect(struct redir_info info);
int redir_recover(void);
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
