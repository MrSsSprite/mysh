/*----------------------------- Private Includes -----------------------------*/
#include "List.h"
#include "__List_struct__.h"
#include "../wrapper_fn/cstdlib.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Structs ------------------------------*/
/*--------------------------- Private Structs END ----------------------------*/

/*---------------------------- Exported Functions ----------------------------*/
struct list *list_init_elem_sz(size_t elem_size)
{
   struct list *this = malloc(sizeof (struct list));
   if (this == NULL)
      return NULL;
   this->elem_size = elem_size;
   this->size = 0;
   this->head = NULL;

   return this;
}

void list_destroy(struct list *this)
{
   struct list_node *iter[2];
   int i = 0;

   if (this == NULL) return;

   iter[0] = this->head;
   while (iter[i])
   {
      iter[i ^ 1] = iter[i]->next;
      free(iter[i]);
      i ^= 1;
   }
   free(this);
}

/**
 * @brief   push an element to the head of the list
 * @param   elem  element to be pushed; the new node is uninitialized if elem
 *                is NULL.
 * @return  0 on success; non-0 otherwise
 */
int list_push(struct list *this, const void *restrict elem)
{
   struct list_node *new_node;
   new_node = malloc(sizeof(struct list_node) + this->elem_size);
   if (new_node == NULL)
      return 1;
   new_node->next = this->head;
   if (elem)
      memcpy(new_node->value, elem, this->elem_size);
   this->head = new_node;
   this->size++;
   return 0;
}

void list_pop(struct list *this)
{
   struct list_node *iter = this->head;
   this->head = iter->next;
   this->size--;
   free(iter);
}

struct list_node **list_head(struct list *this)
{
   return &this->head;
}

size_t list_size(struct list *this)
{
   return this->size;
}

int list_insert (struct list *this, struct list_node **pos,
                 const void *restrict first, size_t n)
{
   struct list_node **iter = pos;
   struct list_node *new_node, *orig_next = *iter;
   struct list_node *err_iter;

   for (const void *ed = first + n * this->elem_size;
        first < ed; first += this->elem_size, iter = &(*iter)->next)
   {
      new_node = malloc(sizeof(struct list_node) + this->elem_size);
      *iter = new_node;
      if (new_node == NULL)
      {
         /* remove every node after preceder but before orig_next */
         new_node = *pos;
         while ((err_iter = new_node))
         {
            new_node = err_iter->next;
            free(err_iter);
         }
         *pos = orig_next;
         return 1;
      }

      memcpy(new_node->value, first, this->elem_size);
   }
   *iter = orig_next;

   this->size += n;
   return 0;
}

/**
 * @brief   erase n list_nodes following preceder
 * @param   preceder the list_node preceding the first to be deleted.
 *          It should be NULL if the first to be deleted is head
 * @param   n # nodes to be deleted
 * @remark  It's the user's responsibility to ensure that all arguments are 
 *          valid, otherwise the behavior is undefined.
 */
void list_erase(struct list *this, struct list_node **pos, size_t n)
{
   struct list_node *del_node = *pos, *next_node;

   for (size_t i = 0; i < n; i++)
   {
      next_node = del_node->next;
      free(del_node);
      del_node = next_node;
   }
   /* Do not use next_node as it's not initialized if n == 0 */
   *pos = del_node;
   this->size -= n;
}

/**
 * @brief   find the node with value equivalent to val
 * @param   pos   position indicator; *pos should point to the first node you
 *                want to start searching. It's either &this->head or
 *                &node->next.
 * @param   val   value to compare the elements to
 * @param   cmp   compare function. should act like memcmp. should be NULL if 
 *                you want the exact memcmp behavior.
 * @return  position indicator indicating the first matching node on success;
 *          NULL on failure.
 */
struct list_node **list_find
(struct list *this, struct list_node **pos, const void *val,
 int(*cmp)(const void*, const void*))
{
   if (cmp == NULL)
   {
      _wrapper_n = this->elem_size;
      cmp = _wpr_memcmp_count;
   }

   for (; *pos; pos = &(*pos)->next)
      if (cmp(val, (*pos)->value) == 0)
         return pos;
   return NULL;
}
/*-------------------------- Exported Functions END --------------------------*/
