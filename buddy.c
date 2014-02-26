/*
 * A buddy allocator for user-space memory allocation
 * By Ethan Shernan, Steven Wojcio, Chris Gordon AKA "The Ballmers"
 */

#include "buddy.h"
#include <sys/mman.h>
#include <stdlib.h>

block* search(int size);
block* split(block *curr_block);
block* coalesce(block *b1);
void* gtmalloc(size_t size);
void gtfree(void *ptr);
int remove_free(block *b1);
void add_free(block* block);
void remove_in_use(block* block);
void add_in_use(block* block);

linked_list* free_list;
linked_list* in_use;

//Search through the free_list for a block that fulfills the size request
block* search(int size)
{
    //Iterate through list
    //At each block, check if size requested > block size
    //If not, check if needs to be split
    //Otherwise, we found the tightest fit block
    //
    //TODO: Check if free list is empty
	node *index = free_list->head;
	while( index->block->size < size && index!=NULL)
		index = index->next;
	if (index == NULL) {
		return NULL; //error... no blocks big enough
	}
	block *smallest_fit = index->block;
	
	while (smallest_fit->size >= size<<1) { 
		smallest_fit = split(smallest_fit);	
	}
	return smallest_fit;	
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
	block* new_block1 = mmap(NULL,sizeof(block),0,0,0,0);
	block* new_block2 = mmap(NULL,sizeof(block),0,0,0,0);
	new_block1->size = curr_block->size>>1;
	new_block1->buddy = new_block2;
	new_block1->front = curr_block->front;
	new_block1->parent = curr_block;
	new_block2->size = new_block1->size;
	new_block2->buddy = new_block1;
	new_block2->front = new_block1->front + new_block1->size;
	new_block2->parent = curr_block;

	curr_block->free = false;
	new_block1->free = true;
	new_block2->free = true;
	
	remove_free(curr_block);
	add_free(new_buddy1);
	add_free(new_buddy2);

	return new_buddy1;
}

//Coalesces two blocks together, then checks the parent ptrs and determines if they need to be coalesced
block* coalesce(block *b1)
{
    //If buddy is not free, or don't have a buddy, return b1
    //Remove b1, b1's buddy from free_list
    //Add parent to free_list
    //recurse with parent and parent's buddy
    //
    //munmap b1 and its buddy... I was thinking we have garbage collection but this isn't java.
	if (b1->free == false || b1->buddy == NULL || b1->buddy->free == false)
		return b1;
	remove_free(b1);
	remove_free(b1->buddy);

	munmap(b1->buddy,sizeof(block));
	munmap(b1,sizeof(block));

	add_free(b1->parent);
	return coalesce(b1->parent);
}

void* gtmalloc(size_t size)
{
   //Search for block
   //Put block in_use list
   //return void pointer 
	block* smallest_fit = search(size);
	add_in_use(smallest_fit);
	remove_free(smallest_fit);
	return smallest_fit->front;
}

void gtfree(void *ptr)
{
    //Iterate through in_use list until we find ptr
    //Mark that block as free
    //Coalesce up all the way
    //Put into free_list
    //Sort the in_use list
    //Return
    //
    //TODO: Add a check for if there are no things in the in use list.
	node* index = in_use->head;
	while(index->block->front != ptr && index != NULL){
		index = index->next;
	}
	
	if (index == NULL){
		//error... we didnt have their thing
		return;//?
	}

	index->free = true;
	remove_in_use(index->block);
	add_free(index->block);
	coalesce(index);
	return;
}

int remove_free(block *b1) {
	//simply take the block out of the free list
    node *curr_node = free_list->head;
    node *prev_node = NULL;

    while(curr_node != NULL)
    {
        if(curr_node->block == b1)
        {
            //Actually remove   
            prev_node->next = curr_node->next;
            curr_node->next = NULL;
            
            //Free
            return munmap(curr_node, sizeof(node)); 
        }
        
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    return 0;
}

void add_free(block* block) {
	//simply add the block to the free list in the right spot.
}

void remove_in_use(block* block) { 

}

void add_in_use(block* block) {
	
}
