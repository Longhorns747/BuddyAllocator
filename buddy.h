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

typedef struct free_list {
	node *head;
	node *tail;
} free_list;

typedef struct in_use {
	node *head;
	node *tail;
} in_use;
