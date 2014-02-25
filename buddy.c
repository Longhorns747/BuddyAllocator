/*
 * A buddy allocator for user-space memory allocation
 * By Ethan Shernan, Steven Wojcio, Chris Gordon AKA "The Ballmers"
 */

#include <"buddy.h">
#include <sys/mman.h>

block* search(int size);
block* split(block *curr_block);
block* coalesce(block *b1);
void* gtmalloc(size_t size);
void gtfree(void *ptr);

//Search through the free_list for a block that fulfills the size request
block* search(int size)
{
    //Iterate through list
    //At each block, check if size requested > block size
    //If not, check if needs to be split
    //Otherwise, we found the tightest fit block
}

//Splits a block into two blocks of equal size and makes them buddies
block* split(block *curr_block)
{
    //Create new block
    //Adjust block sizes
    //Make beginning ptr of new block mid of curr_block
    //Connect Buddy pointers
    //Create parent ptr
    //Sort free_list
}

//Coalesces two blocks together, then checks the parent ptrs and determines if they need to be coalesced
block* coalesce(block *b1)
{
    //If buddy is not free, or don't have a buddy, return b1
    //Remove b1, b1's buddy from free_list
    //Add parent to free_list
    //recurse with parent and parent's buddy
}

void* gtmalloc(size_t size)
{
   //Search for block
   //Put block in_use list
   //return void pointer 
}

void gtfree(void *ptr)
{
    //Iterate through in_use list until we find ptr
    //Mark that block as free
    //Coalesce up all the way
    //Put into free_list
    //Sort the in_use list
    //Return
}
