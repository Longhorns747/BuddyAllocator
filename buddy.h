typedef struct block {
	int size;
	void *front;
	struct block *buddy;
	char is_split;
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
