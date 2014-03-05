/*
 * A buddy allocator for user-space memory allocation
 * By Ethan Shernan, Steven Wojcio, Chris Gordon AKA "The Ballmers"
 */

#include "buddy.h"
#include <fcntl.h>
#define FILEPATH "/tmp/mmapped.bin"
#define FILESIZE 2147483648

block_t* search(int size);
block_t* split(block_t *curr_block);
block_t* coalesce(block_t *b1);
void* gtmalloc(size_t size);
void gtfree(void *ptr);
int remove_free(block_t *b1);
node* add_free(block_t* b1);
int remove_in_use(block_t *b1);
node* add_in_use(block_t* b1);
int remove_from_ll(block_t* b1, linked_list *ll); 
node* add_in_order(block_t* b1, linked_list *list);
void print_ll(linked_list *ll);

static int fd;
linked_list* free_list;
linked_list* in_use;

//Search through the free_list for a block that fulfills the size request
block_t* search(int size)
{
    //Iterate through list
    //At each block, check if size requested > block size
    //If not, check if needs to be split
    //Otherwise, we found the tightest fit block
	
	node *index = free_list->head;
	printf("created index...");
	if(index != NULL) printf("and it wasnt null.\n");
	else printf("and it was null\n");

	while(index != NULL && index->block->size < size)
		index = index->next;

	printf("finished while. index set\n");

	if (index == NULL) {
		//printf("index was null\n");
		//block_t *new_block = mmap(NULL, sizeof(block_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		//not sure why this line was here...
		//printf("between mmaps\n");
		//new_block->front = mmap(NULL, FILESIZE / 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	       	//printf("NEW BLOCK ADDR -> %x",new_block->front); 
		//printf("just mmapped again\n");
		//if(free_list -> head == NULL) printf("The free list is null now... what happens?\n"); 
		//index = add_free(new_block);
	
		//printf("added to in use and THIS IS WHAT THE ADDR IS %x\n",index->block->front);	
		return NULL;
	}
	block_t *smallest_fit = index->block;

	printf("smallest fit set\n");
	
	while (smallest_fit->size >= size<<1) {
		printf("had to split\n"); 
		smallest_fit = split(smallest_fit);
		printf("split it\n");	
	}
	return smallest_fit;	
}

//Splits a block into two blocks of equal size and makes them buddies
block_t* split(block_t *curr_block)
{
    //Create new block
    //Adjust block sizes
    //Make beginning ptr of new block mid of curr_block
    //Connect Buddy pointers
    //Create parent ptr
    //Sort free_list
	printf("CURR BLOCK SIZE>>> SPLITTING: %x",curr_block->size);
	block_t* new_block1 = mmap(NULL, curr_block->size / 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	block_t* new_block2 = mmap(NULL, curr_block->size / 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	new_block1->size = curr_block->size>>1;
	new_block1->buddy = new_block2;
	new_block1->front = curr_block->front;
	new_block1->parent = curr_block;

	printf("new block one built\n");

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
block_t* coalesce(block_t *b1)
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
	if(in_use == NULL && free_list == NULL){
    		printf("INITIALIZING!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    //printf("Just did fd thing.\n");
		lseek(fd, FILESIZE-1, SEEK_SET);
    //printf("lseek done.\n");

    		write(fd, "", 1);
	//printf("write fd done\n");
	//if(in_use == NULL)
	//{
		//printf("about to mmap\n");
		in_use = mmap(NULL, sizeof(linked_list), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		free_list = mmap(NULL, sizeof(linked_list), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		
		block_t* new_block = mmap(NULL, sizeof(block_t),PROT_READ| PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		new_block->size = FILESIZE;
		new_block->front = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		new_block->parent = NULL;
		new_block->buddy = NULL;
		new_block->free = true;	
		
		node* new_node = mmap(NULL, sizeof(node),PROT_READ| PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		new_node->block = new_block;	
	
		in_use -> head = NULL;
		in_use -> tail = NULL;
		free_list -> head = new_node;
		free_list -> tail = NULL;
		//printf("done mmap\n");
	}
	block_t* smallest_fit = search(size);
	//printf("search done\n");
	add_in_use(smallest_fit);
	//printf("added to in use\n");
	remove_free(smallest_fit);
	//printf("removed from free\n");
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
	
	//if (index == NULL && in_use->head == NULL){
		//node *curr = free_list->head;
		//node *next = curr->next;
		//if(curr == NULL){
		//	return;
		//}
		//while(next != NULL){
		//	munmap(curr->block->front, curr->block->size);
		//	munmap(curr->block, sizeof(block_t));
		//	munmap(curr, sizeof(node));
		//	curr = next;
		//	next = next->next;
		//}
		//if(curr != NULL){
		//	munmap(curr->block->front, curr->block->size);
		//	munmap(curr->block, sizeof(block_t));
		//	munmap(curr, sizeof(node));
		//}
		//return;
	//}

	index->block->free = true;
	remove_in_use(index->block);
	add_free(index->block);
	coalesce(index->block);
}

int remove_free(block_t *b1) {
	//simply take the block out of the free list
    return remove_from_ll(b1, free_list);
}

int remove_in_use(block_t *b1) { 
    //Remove a block from the in use list
    return remove_from_ll(b1, in_use);
}

//Generic Remove from Linked List method
int remove_from_ll(block_t *b1, linked_list *ll) { 
    node *curr_node = ll->head;
    node *prev_node = NULL;

    while(curr_node != NULL)
    {
	if(curr_node->block == b1)
        {
           if(curr_node == ll->head)
           {
               ll->head = curr_node->next;
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

node* add_in_use(block_t* b1) {
	return add_in_order(b1, in_use);
}

node* add_free(block_t* b1) {
//	if (free_list -> head == NULL) printf("still null.... wtf\n");
	return add_in_order(b1, free_list);
}

node* add_in_order(block_t* b1, linked_list* list) {
	
	//if (list -> head == NULL) printf("1 still null.... wtf\n");
	node *new_node = mmap(NULL, sizeof(node), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		
	//if (list -> head == NULL) printf("2 still null.... wtf\n");
	new_node->block = b1;
	//printf("addr of b1 = %x",b1->front);	
	//if (list -> head == NULL) printf("3 still null.... wtf\n");
	node *prev = list->head;
	
	if (list->head == NULL) {
		//printf("Should go here.. list head should be null\n");
		//new_node->block = b1;
		list->head = new_node;
		return new_node;
	}
	//new_node->block = b1;
	//if its the smallest block, make it the head
	if(b1->size <= prev->block->size)
	{
		new_node->next = prev;
		list->head = new_node;
		return new_node;
	}

	//search for the correct spot and add it
	node *curr = prev->next;
	while(curr != NULL)
	{
		if(b1->size <= curr->block->size)
		{
			new_node->next = curr;
			prev->next = new_node;
			return new_node;
		}
	}

	//if b1 is the largest block, just append it to the end
	prev->next = new_node;
	new_node -> next = NULL;
	return new_node;		
}

void print_ll(linked_list *ll)
{
    node *curr_node = ll->head;
    int count = 0;
    
    while(curr_node != NULL)
    {
        count++;
        printf("Item %d: size %d, front %p, buddy: %p\n", count, curr_node->block->size, 
            curr_node->block->front, curr_node->block->buddy); 
        curr_node = curr_node->next;
    }
    printf("Total Size: %d\n", count);
}

int main()
{
	printf("Starting Test.\n");
	int *stuff = (int *)(gtmalloc(69));
	int *stuff2 = (int *)(gtmalloc(16));
	int *stuff3 = (int *)(gtmalloc(16));
	printf("Created 'stuff' printing in-use list\n");
	print_ll(in_use);
    
    if (in_use == NULL) printf("In Use is null\n");
	else printf("before seg used%x\n",in_use->head->block->front);
	if (free_list == NULL) printf("Free is null\n");
	//else printf("before seg free %x\n",free_list->head->block->front);
	*stuff = 5;
	*stuff2 = 6;
	*stuff3 = 7;
	printf("yo yo yo%d %d %d\n", *stuff,*stuff2,*stuff3);
	gtfree(stuff);
	if (in_use == NULL && free_list == NULL) printf("good\n");    
return 0;
}

