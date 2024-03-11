#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

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

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *next;  /* Pointer to the next _block of allocated memory  */
   bool   free;          /* Is this _block free?                            */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                   */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;
  
#if defined FIT && FIT == 0
   /* First fit */
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
   /** \TODO Implement best fit here */
   struct _block *bestFit = NULL;
   size_t minSize = SIZE_MAX; // Initialize minSize to the maximum possible size

   while (curr) 
   {
      if (curr->free && curr->size >= size && curr->size < minSize) 
      {
         bestFit = curr;
         minSize = curr->size;
      }

      *last = curr;
      curr = curr->next;
   }

   return bestFit;
#endif



#if defined WORST && WORST == 0
   /** \TODO Implement worst fit here */
   size_t maxSize = 0;
   struct _block *worstFit = NULL;
   while (curr) 
   {
      if (curr->free && curr->size >= size && curr->size > maxSize) 
      {
         worstFit = curr;
         maxSize = curr->size;
      }

      *last = curr;
      curr = curr->next;
   }

   return worstFit;
   
#endif

// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   static struct _block *lastChecked = NULL;  // Keep track of the last checked block 
   struct _block *current = (lastChecked != NULL) ? lastChecked->next : heapList;

    while (current) 
    {
        if (current->free && current->size >= size) 
        {
            lastChecked = current;  // Update the last checked block for the next iteration
            *last = current;
            return current;
        }

        *last = current;
        current = current->next;
    }

    // If no free block is found in the remaining list, start from the beginning
    lastChecked = NULL;

    return NULL;


  
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
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   num_grows++;
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{
   
   num_requested = num_requested + size;
   //max_heap = max_heap 

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

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */
   //printf("Before num_requested = %d\n",num_mallocs);
  
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */
   if (next && next->size)
   {

      num_reuses++;
      if ((next->size) > size)
      {
         size_t remainingSize = next->size - size;
         if (remainingSize>(sizeof(struct _block)+4))
         {
               struct _block *newBlock = (struct _block *)((char *)next + size);
               newBlock->size = remainingSize - sizeof(struct _block);
               newBlock->next = next->next;
               newBlock->free = true;

               next->size = size;
               next->next = newBlock;

               num_splits++;
      
         }

      }
   }


   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      max_heap = max_heap+size;
      
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
   next->free = false;

   /* Return data address associated with _block to the user */
   num_mallocs++;
   return BLOCK_DATA(next);
 
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
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

   /* Make _block as free */
   num_frees++;
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;


 
     /* Check if the previous block is free */
   struct _block *prev = heapList;  // Start from the beginning of the heap
   while (prev && prev->next != curr) {
      prev = prev->next;
   }

   if (prev && prev->free) 
   {
      /* Combine with the previous block */
      prev->size += curr->size;
      prev->next = curr->next;
      num_coalesces++;
      curr = prev;  // Update curr to the combined block
   }

   /* TODO: Coalesce free _blocks.  If the next block 
            are free then combine them with this block being freed.
   */
   if (curr && curr->next)
   {
      struct _block *next = curr->next;
      curr->size += next->size;
      curr->next = (next)->next;
      num_coalesces++;
      next = curr;
   }


   //For counting the number of blocks
   struct _block *LinkedList = heapList; 
   num_blocks = 0;
   while (LinkedList != NULL) 
   {
      num_blocks++;
      LinkedList = LinkedList->next;  // Move to the next block
      
   }

}

void *calloc( size_t nmemb, size_t size )
{
   size_t totalSize = nmemb * size;

    // Check for overflow
    if (nmemb != 0 && totalSize / nmemb != size) {
        return NULL;
    }

    // Allocate memory using malloc
    void *ptr = malloc(totalSize);

    if (ptr != NULL) {
        // Clear the allocated memory to zero
        memset(ptr, 0, totalSize);
    }

    return ptr;
}

void *realloc( void *ptr, size_t size )
{
   // If ptr is NULL, this is equivalent to malloc(size)
    if (ptr == NULL) {
        return malloc(size);
    }

    // If size is zero, this is equivalent to free(ptr)
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    // Retrieve the current _block from the given pointer
    struct _block *curr = BLOCK_HEADER(ptr);

    // Check if the current _block can accommodate the new size
    if (curr->size >= size) {
        // The current _block is large enough, no need to allocate new memory
        return ptr;
    }

    // Allocate a new _block of the requested size
    void *newPtr = malloc(size);

    if (newPtr != NULL) {
        // Copy the data from the old _block to the new _block
       memcpy(newPtr, ptr, curr->size);

        // Free the old _block
        free(ptr);

        return newPtr;
    }

    return NULL; // Allocation or copying failed
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwMjM= -----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
