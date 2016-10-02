#ifndef PMAP_H
#define PMAP_H

struct pmap_node
{
	void* left;
	void* right;
};

/* Danger, Will Robinson! The type and the macro must be compatible. */

struct pmap
{
	struct pmap_node* item;
	int count;
	int max;
};

#define PMAPOF(LEFT, RIGHT) \
	struct { \
		struct { LEFT* left; RIGHT* right; }* item; \
		int count; \
		int max; \
	}

extern void pmap_put(void* map, void* left, void* right);
extern void pmap_add(void* map, void* left, void* right);
extern void* pmap_get(void* map, void* left);

#endif
