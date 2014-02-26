#include <stdbool.h>
#include <sys/mman.h>
#include <stdlib.h>

typedef struct block {
	int size;
	void *front;
	char free;
	struct block *buddy;
	struct block *parent;
} block;

typedef struct node {
	block* block;
	struct node *next;
} node;

typedef struct linked_list {
	node *head;
	node *tail;
} linked_list;
