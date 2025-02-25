#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;
struct block* temppointer = NULL;
/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct block
{
   size_t      size;  /* Size of the allocated block of memory in bytes */
   struct block *next;  /* Pointer to the next block of allcated memory   */
   bool        free;  /* Is this block free?                     */
};


struct block *FreeList = NULL; /* Free list to track the blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free blocks
 * \param size size of the block needed in bytes
 *
 * \return a block that fits the request or NULL if no free block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct block *findFreeBlock(struct block **last, size_t size)
{
   struct block *curr = FreeList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0 //puts in smallest block of mem
   //printf("TODO: Implement best fit here\n");
   struct block* tracker = NULL;
   int smallest = INT_MAX;
   while (curr)
   {
     if (curr->free)
     {
       if ((int)(curr->size - size) < smallest)
       {
         tracker = curr;
       }
     }
     curr = curr->next;
   }
   curr = tracker;
#endif

#if defined WORST && WORST == 0 //puts in biggest block of mem
   //printf("TODO: Implement worst fit here\n");
   struct block* tracker = NULL;
   int biggest = INT_MIN;
   while (curr)
   {
     if (curr->free)
     {
       if ((int)(curr->size - size) > biggest)
       {
         tracker = curr;
         biggest = curr->size;
       }
     }
     curr = curr->next;
   }
   curr = tracker;

#endif

#if defined NEXT && NEXT == 0 //first fit with a tracker
   //printf("TODO:1 Implement next fit here\n");

   //check where last left off using tempointer
   if(temppointer)
   {
     curr = temppointer;
   }
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }
   temppointer = curr;

#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated block of NULL if failed
 */
struct block *growHeap(struct block *last, size_t size)
{
   /* Request more space from OS */
   struct block *curr = (struct block *)sbrk(0);
   struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct block *)-1)
   {
      return NULL;
   }

   /* Update FreeList if not set */
   if (FreeList == NULL)
   {
      FreeList = curr;
   }

   /* Attach new block to prev block */
   if (last)
   {
      last->next = curr;
   }

   /* Update block metadata */
   num_grows++;
   num_blocks++;
   max_heap+=size;
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free block of heap memory for the calling process.
 * if there is no free block that satisfies the request then grows the
 * heap and returns a new block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process
 * or NULL if failed
 */
void *malloc(size_t size)
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0)
   {
      return NULL;
   }

   /* Look for free block */
   struct block *last = FreeList;
   struct block *next = findFreeBlock(&last, size);


   /* TODO: Split free block if possible */
   // if (next != NULL)
   // {
   //   if(next->size > size)
   //   {
   //     int tempNext;
   //     tempNext = next->size - size;
   //     while(!(last->free))
   //     {
   //       last=last->next;
   //     }
   //     last->size = tempNext + last->size;
   //   }

   if (next && next->size > size)
   {
     int overshoot = (int)(next->size - size);
     next->size -= overshoot;
     bool compensated = false;
     while (FreeList && !compensated)
     {
       if (FreeList->free && FreeList != next)
       {
         FreeList->size += overshoot;
         compensated = !compensated;
       }
       FreeList = FreeList->next;
     }
   }

   /* Could not find free block, so grow heap */

   if (next == NULL)
   {
      next = growHeap(last, size);
   }
   else
   {
     num_reuses++;
   }

   /* Could not find free block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }

   /* Mark block as in use */
   next->free = false;
   num_mallocs++;
   num_requested = num_requested + size;
   /* Return data address associated with block */
   return BLOCK_DATA(next);
}


/*
 * \brief free
 *
 * frees the memory block pointed to by pointer. if the block is adjacent
 * to another block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
   if (ptr == NULL)
   {
      return;
   }

   /* Make block as free */
   struct block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;

   /* TODO: Coalesce free blocks if needed */
   if(curr->next != NULL)
   {
     if(curr->next->free)
     {
      curr->size = curr->size + curr->next->size;

      if(curr->next->next != NULL)
      {
        curr->next = curr->next->next;
      }
      else
      {
        curr->next = NULL;
      }
      num_coalesces++;
    }
   }


}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
