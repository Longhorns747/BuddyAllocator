#include <stdbool.h>
#include <sys/mman.h>
#include <stdlib.h>

typedef struct t_block {
	int size;
	void *front;
	char free;
	struct t_block *buddy;
	struct t_block *parent;
} t_block;

typedef struct node {
	t_block* block;
	struct node *next;
} node;

typedef struct linked_list {
	node *head;
	node *tail;
} linked_list;
