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
void remove_free(block* block);
void add_free(block* block);

linked_list* free_list;
linked_list* in_use;

//Search through the free_list for a block that fulfills the size request
block* search(int size)
{
    //Iterate through list
    //At each block, check if size requested > block size
    //If not, check if needs to be split
    //Otherwise, we found the tightest fit block
	node *index = free_list->head;
	while( index->block->size < size && index!=free_list->tail)
		index = index->next;
	
	block *smallest_fit = index->block;
	if (smallest_fit->size < size)
		//error... no blocks big enough
		return NULL;//?
	if (smallest_fit->size > size<<1) { 
		split(smallest_fit);
		return search(size);	
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
	if (b1->free == false || b1->buddy == NULL || b1->buddy->free == false)
		return b1;
	remove_free(b1);
	remove_free(b1->buddy);

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
	block* index = in_use->head;
	while(index->block->front != ptr && index != in_use->block->tail){
		index = index->next;
	}
	
	if (index->block->front != ptr){
		//error... we didnt have their thing
		return;//?
	}

	index->free = true;
	coalesce(index);
	return;
}

void remove_free(block* block) {
	//simply take the block out of the free list
	
}

void add_free(block* block) {
	//simply add the block to the free list in the right spot.
}
