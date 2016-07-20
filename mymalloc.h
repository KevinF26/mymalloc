#ifndef _mymalloc_h
#define _mymalloc_h

struct memUnit{
	unsigned int size;
	char isfree;
	struct memUnit * next;
};

struct memUnit * find_open_unit(unsigned int size);
void * mymalloc(unsigned int size);
void myfree(void *,char *, int);

#endif
