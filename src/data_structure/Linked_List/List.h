#ifndef __LIST_H
#define __LIST_H

/*----------------------------- Public Includes ------------------------------*/
#include <stddef.h>
/*--------------------------- Public Includes END ----------------------------*/

/*------------------------------ Public Strcut -------------------------------*/
struct list_node
{
   struct list_node *next;
   char value[];
};
/*---------------------------- Public Strcut END -----------------------------*/

/*----------------------------- Public Typedefs ------------------------------*/
typedef struct list *List;
typedef struct list_node *List_Node;
/*--------------------------- Public Typedefs END ----------------------------*/

/*------------------------------ Public Macros -------------------------------*/
#define list_init(elem_type) list_init_elem_sz(sizeof(elem_type))
/*---------------------------- Public Macros END -----------------------------*/

/*------------------------ Public Function Prototypes ------------------------*/
List list_init_elem_sz(size_t elem_size);
void list_destroy(List this);
int list_push(List this, const void *restrict elem);
void list_pop(List this);
List_Node *list_head(List this);
size_t list_size(List this);
int list_insert (List this, List_Node *pos,
                 const void *restrict first, size_t n);
void list_erase(List this, List_Node *pos, size_t n);
struct list_node **list_find
(struct list *this, struct list_node **pos, const void *val,
 int(*cmp)(const void*, const void*));
/*---------------------- Public Function Prototypes END ----------------------*/

#endif
