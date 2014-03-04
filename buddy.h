#include <stdbool.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct t_block {
	unsigned int size;
	void *front;
	char free;
	struct t_block *buddy;
	struct t_block *parent;
} block_t;

typedef struct node {
	block_t* block;
	struct node *next;
} node;

typedef struct linked_list {
	node *head;
	node *tail;
} linked_list;
