/*
 * A buddy allocator for user-space memory allocation
 * By Ethan Shernan, Steven Wojcio, Chris Gordon AKA "The Ballmers"
 */

#include "buddy.h"

block* search(int size);
block* split(block *curr_block);
block* coalesce(block *b1);
void* gtmalloc(size_t size);
void gtfree(void *ptr);
int remove_free(block *b1);
void add_free(block* b1);
int remove_in_use(block *b1);
void add_in_use(block* b1);
int remove_from_ll(block* b1, node *head); 
void add_in_order(block* b1, linked_list *list);

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
		block *new_block = mmap(NULL, 4096, PROT_WRITE, 0, 0, 0);
		add_in_use(new_block);
		index = new_block;	
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
	block* new_block1;
	block* new_block2;
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
	add_free(new_block1);
	add_free(new_block2);

	return new_block1;
}

//Coalesces two blocks together, then checks the parent ptrs and determines if they need to be coalesced
block* coalesce(block *b1)
{
    //If buddy is not free, or don't have a buddy, return b1
    //Remove b1, b1's buddy from free_list
    //Add parent to free_list
    //recurse with parent and parent's buddy
    //munmap b1 and its buddy... I was thinking we have garbage collection but this isn't java.
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

	if(in_use == NULL)
	{
		in_use = mmap(NULL, sizeof(linked_list), PROT_WRITE, 0, 0, 0);
		free_list = mmap(NULL, sizeof(linked_list), PROT_WRITE, 0, 0, 0);
	}
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
	node* index = in_use->head;
	while(index->block->front != ptr && index != NULL){
		index = index->next;
	}
	
	if (index == NULL && in_use->head == NULL){
		node *curr = free_list->head;
		node *next = curr->next;
		if(curr == NULL){
			return;
		}
		while(next != NULL){
			munmap(curr->block->front, curr->block->size);
			munmap(curr->block, sizeof(block));
			munmap(curr, sizeof(node));
			curr = next;
			next = next->next;
		}
		if(curr != NULL){
			munmap(curr->block->front, curr->block->size);
			munmap(curr->block, sizeof(block));
			munmap(curr, sizeof(node));
		}
		return;
	}

	index->block->free = true;
	remove_in_use(index->block);
	add_free(index->block);
	coalesce(index->block);
}

int remove_free(block *b1) {
	//simply take the block out of the free list
    return remove_from_ll(b1, free_list);
}

int remove_in_use(block *b1) { 
    //Remove a block from the in use list
    return remove_from_ll(b1, in_use);
}

//Generic Remove from Linked List method
int remove_from_ll(block* b1, linked_list *ll) { 
    node *curr_node = ll->head;
    node *prev_node = NULL;

    while(curr_node != NULL)
    {
        if(curr_node->block == b1)
        {
            if(curr_node == ll->head)
            {
                ll->head = NULL;
                return 1;
            }
            
            //Actually remove   
            prev_node->next = curr_node->next;
            curr_node->next = NULL;
            return 1; 
        }
        
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    return 0;
}

void add_in_use(block* b1) {
	return add_in_order(b1, in_use);
}

void add_free(block* b1) {
	return add_in_order(b1, free_list);
}

void add_in_order(block* b1, linked_list* list) {
	node *new_node;
	new_node->block = b1;
	node *prev = list->head;

	//if its the smallest block, make it the head
	if(list->head != NULL && b1->size <= prev->block->size)
	{
		new_node->next = list->head;
		list->head = new_node;
		return;
	}

	//search for the correct spot and add it
	node *curr = prev->next;
	while(curr != NULL)
	{
		if(b1->size <= prev->block->size)
		{
			new_node->next = prev->next;
			prev->next = new_node;
			return;
		}
	}

	//if b1 is the largest block, just append it to the end
	prev->next = new_node;
	return; 		
		
}
int main()
{
	int *stuff = (int *)(gtmalloc(69));
	*stuff = 5;
	printf("%d\n", *stuff);
	gtfree(stuff);
    return 0;
}

