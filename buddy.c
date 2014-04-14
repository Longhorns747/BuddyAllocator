/*
 * A buddy allocator for user-space memory allocation
 * By Ethan Shernan, Steven Wojcio, Chris Gordon AKA "The Ballmers"
 */

#include "buddy.h"
#include <fcntl.h>
#include <sys/time.h>
#define FILEPATH "/tmp/mmapped.bin"
#define FILESIZE 2147483648

block_t* search(size_t size);
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
block_t* search(size_t size)
{
    //Iterate through list
    //At each block, check if size requested > block size
    //If not, check if needs to be split
    //Otherwise, we found the tightest fit block
	
	node *index = free_list->head;

	while(index != NULL && index->block->size < size)
		index = index->next;

	if (index == NULL) {
		return NULL;
	}
	
    block_t *smallest_fit = index->block;
	
	if(size == FILESIZE){
		return smallest_fit;
	}
	while (smallest_fit->size >= size<<1) {
		smallest_fit = split(smallest_fit);
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
	block_t* new_block1 = mmap(NULL, curr_block->size / 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	block_t* new_block2 = mmap(NULL, curr_block->size / 2, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	new_block1->size = curr_block->size>>1;
	new_block1->buddy = new_block2;
	new_block1->front = curr_block->front;
	new_block1->parent = curr_block;


	new_block2->size = new_block1->size;
	new_block2->buddy = new_block1;
	new_block2->front = new_block1->front + new_block1->size;
	new_block2->parent = curr_block;

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
	
    if (b1->free == false || b1->buddy == NULL || b1->buddy->free == false || b1->parent->free == true)
		return b1;
	remove_free(b1->buddy);
	remove_free(b1);

	add_free(b1->parent);
	block_t *parent = b1->parent;
	
	munmap(b1->buddy, sizeof(block_t));
        munmap(b1, sizeof(block_t));

	return coalesce(b1->parent);
}

void* gtmalloc(size_t size)
{
    //Search for block
    //Put block in_use list
    //return void pointer
    	if(size<1 || size>FILESIZE) return NULL;
	if(in_use == NULL && free_list == NULL){
		fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
		lseek(fd, FILESIZE-1, SEEK_SET);

        	write(fd, "", 1);
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
	}
	block_t* smallest_fit = search(size);
	if (smallest_fit == NULL) return NULL;
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
    	if (ptr == NULL) return;

	node* index = in_use->head;
	while(index->block->front != ptr && index != NULL){
		index = index->next;
	}

	add_free(index->block);
	coalesce(index->block);
	remove_in_use(index->block);

	if (in_use -> head == NULL) {
		//print_ll(free_list);
		//print_ll(in_use);
		munmap(free_list->head->block->front,FILESIZE);
		munmap(free_list->head->block, sizeof(block_t));
		munmap(free_list->head,sizeof(node));
		munmap(free_list, sizeof(linked_list));
		munmap(in_use, sizeof(linked_list));
		in_use = NULL;
		free_list = NULL;
	}
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
	    munmap(curr_node, sizeof(node));
            return 1; 
        }
        
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    return 0;
}

node* add_in_use(block_t* b1) {
        b1->free = false;
	return add_in_order(b1, in_use);
}

node* add_free(block_t* b1) {
	b1->free = true;
	return add_in_order(b1, free_list);
}

node* add_in_order(block_t* b1, linked_list* list) {
	
	node *new_node = mmap(NULL, sizeof(node), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
	new_node->block = b1;
	node *prev = list->head;
	
	if (list->head == NULL) {
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
        curr = curr->next;
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
        printf("Item %d: size %u, front %p, buddy: %p\n", count, curr_node->block->size, 
            curr_node->block->front, curr_node->block->buddy->front); 
        curr_node = curr_node->next;
    }
    printf("Total Size: %d\n", count);
}

int main()
{
	struct timeval start, end;
	double elapsedtime;

	printf("---------------------Test 1------------------------\n");
	printf("Here, we will initialize and free 'FILESIZE' sized chunks 100 times for both malloc and gtmalloc. \n");
	gettimeofday(&start, NULL);

	int *stuff;
	int i;
	for(i = 0; i < 100; i++){
		stuff = gtmalloc(FILESIZE);
		gtfree(stuff);
	}

	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("gt: %f\n", elapsedtime);


	gettimeofday(&start, NULL);

	for(i = 0; i < 100; i++){
		stuff = malloc(FILESIZE);
		free(stuff);
	}

	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("malloc: %f\n", elapsedtime);

	

	printf("---------------------Test 2------------------------\n");
	printf("This test is testing allocating memory of various sizes when some memory has already been allocated.\n");
	gettimeofday(&start, NULL);
	int *stuff2 = gtmalloc(1);
	for(i = 0; i < 100; i++){
		stuff = gtmalloc(i*8);
		gtfree(stuff);
	}
	gtfree(stuff2);
	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("gt: %f\n", elapsedtime);


	gettimeofday(&start, NULL);
	stuff2 = malloc(1);
	for(i = 0; i < 100; i++){
		stuff = malloc(i*8);
		free(stuff);
	}
	free(stuff2);
	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("malloc: %f\n", elapsedtime);


	printf("---------------------Test 3------------------------\n");
	printf("This will test allocating a lot of small blocks and then freeing them (all one by one).\n");
	gettimeofday(&start, NULL);

	for(i = 0; i < 100; i++){
		stuff = gtmalloc(1);
		gtfree(stuff);
	}

	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("gt: %f\n", elapsedtime);


	gettimeofday(&start, NULL);

	for(i = 0; i < 100; i++){
		stuff = malloc(1);
		free(stuff);
	}

	gettimeofday(&end, NULL);
	elapsedtime = (end.tv_sec - start.tv_sec) * 1000.0;
	elapsedtime += (end.tv_usec - start.tv_usec) / 1000.0;
	
	printf("malloc: %f\n", elapsedtime);
	return 0;
}
