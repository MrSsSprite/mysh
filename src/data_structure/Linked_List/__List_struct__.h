#ifndef __LIST_STRUCT___H
#define __LIST_STRUCT___H

/**
 * @file    __List_struct.h
 * @breif   this file is open for adaptors only. The content should not be
 *          touched by user directly!
 */
/*----------------------------- Private Includes -----------------------------*/
#include <stddef.h>
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private structs ------------------------------*/
struct list
{
   struct list_node *head;
   size_t size, elem_size;
};
/*--------------------------- Private structs END ----------------------------*/

#endif
