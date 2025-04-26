/*----------------------------- Private Includes -----------------------------*/
#include "Vector.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
/*--------------------------- Private Includes END ---------------------------*/

/*----------------------------- Private Structs ------------------------------*/
struct vector
{
   void *st, *cav, *ed;
   size_t elem_sz;
};
/*--------------------------- Private Structs END ----------------------------*/

/*----------------------- Private Function Prototypes ------------------------*/
static int _vector_resize(struct vector *this, size_t new_size);
static inline int _vector_expand(struct vector *this);
static inline int _vector_shrink(struct vector *this);
/*--------------------- Private Function Prototypes END ----------------------*/

/*---------------------------- Exported Functions ----------------------------*/
struct vector *vector_init_elem_sz(size_t elem_sz)
{
   struct vector *this = malloc(sizeof(struct vector));
   
   if (this == NULL)
      return NULL;

   this->elem_sz = elem_sz;
   this->st = this->cav = this->ed = NULL;

   return this;
}

void vector_destroy(struct vector *this)
{
   free(this->st);
   free(this);
}

size_t vector_size(struct vector *this)
{ return (this->cav - this->st) / this->elem_sz; }
size_t vector_capacity(struct vector *this)
{ return (this->ed - this->st) / this->elem_sz; }

int vector_push(struct vector *this, const void *item)
{
   if (this->cav == this->ed && _vector_expand(this))
      return 1;

   memcpy(this->cav, item, this->elem_sz);
   this->cav += this->elem_sz;
   return 0;
}

int vector_pop(struct vector *this)
{
   this->cav -= this->elem_sz;
   if (_vector_shrink(this)) return 1;
   return 0;
}

void *vector_begin(struct vector *this)
{ return this->st; }

void *vector_end(struct vector *this)
{ return this->cav; }

int vector_insert(struct vector *this, void *restrict dest,
                  const void *restrict first, size_t n)
{
   size_t vec_space = this->ed - this->cav, req_space = n * this->elem_sz,
          pos = dest - this->st;

   /* Ensure that the vector has enough cavity */
   if (vec_space < req_space &&
       _vector_resize(this, (this->ed - this->st + (req_space - vec_space))))
      return 1;
   dest = this->st + pos;
   /* move existing data backward */
   memmove(dest + req_space, dest, this->cav - dest);
   /* copy the data from first */
   memcpy(dest, first, req_space);

   this->cav += req_space;

   return 0;
}

int vector_erase(struct vector *this, void *first, size_t n)
{
   size_t saved_space = n * this->elem_sz;
   void *src = first + saved_space;
   size_t src_sz = src < this->cav ? this->cav - src : 0;

   /* move data forward */
   memmove(first, src, src_sz);
   /* update vector */
   this->cav -= saved_space;
   /* shrink size (the function check whether it's necessary) */
   return _vector_shrink(this);
}
/*-------------------------- Exported Functions END --------------------------*/

/*---------------------------- Private Functions -----------------------------*/
static int _vector_resize(struct vector *this, size_t new_size)
{
   void *new_mem;

   if (new_size == 0)
   {
      free(this->st);
      this->st = this->cav = this->ed = NULL;
      return 0;
   }

   new_mem = realloc(this->st, new_size);
   if (new_mem == NULL) return 1;

   this->cav = new_mem + (this->cav - this->st);
   this->st = new_mem;
   this->ed = new_mem + new_size;

   return 0;
}

static inline int _vector_expand(struct vector *this)
{
   size_t new_size = this->ed - this->st;
   if (new_size) new_size *= 2u;
   else new_size = 1u * this->elem_sz;
   return _vector_resize(this, new_size);
}

static inline int _vector_shrink(struct vector *this)
{
   size_t cap = vector_capacity(this);
   if (vector_size(this) > cap / 4) return 0;

   return _vector_resize(this, cap / 2);
}
/*-------------------------- Private Functions END ---------------------------*/
