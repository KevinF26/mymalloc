#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<mymalloc.h>

//global head of list
struct memUnit * head=NULL;

//iterate over list to find open memory of at least size size. returns either pointer to memunit or NULL
struct memUnit * find_open_unit(unsigned int size)
{
	struct memUnit * curr=head;
	while(curr!=NULL && (!curr->isfree || curr->size < size))
	{
		curr=curr->next;
	}
	return curr;
}

//returns void pointer to adequate size memory chunk
//this implementation handles both saturation and fragmentation, therefore there is no reason to print errors from it
void * mymalloc(unsigned int size)
{	
	//handle calls to mymalloc of size<=0
	if(size<=0)
	{
		return NULL;
	}

	struct memUnit * unit;
	struct memUnit * tmp;

	if(head==NULL)//first malloc call
	{
		//create initial chunk of size 5000
		unit=sbrk(5000);
		struct memUnit * next;
                next=(char*)unit+size+sizeof(struct memUnit);
                next->next=NULL;
                next->isfree=1;
                next->size=5000-(size+sizeof(struct memUnit));
                unit->next=next;
                unit->size=size;
		unit->isfree=0;
		head=unit;
		return unit+1;
	}
	else
	{
		//attempt to find preexisting free memory chunk of adequate size
		unit=find_open_unit(size);
		if(unit==NULL) //find_open_unit didnt return any preexisting chunks
		{
			//allocate new memory chunk and throw it in the list
			tmp=head;
			unit=sbrk(5000);
			struct memUnit * next;
                	next=(char*)unit+size+sizeof(struct memUnit);
			next->next=tmp;
			next->isfree=1;
			next->size=5000-(size+sizeof(struct memUnit));
			unit->next=next;
			unit->size=size;
			unit->isfree=0;
			head=unit;
			return unit+1;
		}
		else
		{
			//determine size of returned chunk and break up if necessary, then send to caller	
			if(unit->size > (size+2*sizeof(struct memUnit))) //big enough to be broken up
			{
				//breaks unit into smallest chunk needed plus everything else in next
				struct memUnit * next;
				next=(char*)unit+size+sizeof(struct memUnit);
				next->next=unit->next;
				next->isfree=1;
				next->size=unit->size-(size+sizeof(struct memUnit));
				unit->next=next;
				unit->size=size;
			}
			unit->isfree=0;
			return unit+1;
		}
	}
}

//will return allocated memory to the list, attempting to concatenate contiguous free chunks where possible to handle fragmentation
//returns error messages when invalid pointers are sent to myfree and when redundant free calls are made
void myfree(void * ptr, char * f, int l)
{
	//handles calls with NULL pointer
	if(ptr==NULL)
        {
                return;
        }

	//walk through list to see if the ptr argument corresponds to something we previously mymalloced
	struct memUnit * tofree=(struct memUnit*)ptr - 1;
	struct memUnit * curr=head;
	while(curr!=NULL)
	{
		if(curr==tofree)
		{
			break;
		}
		curr=curr->next;
	}
	//handle invalid pointer
	if(curr==NULL)
	{
		printf("error: call on line %d in file %s was to a pointer never allocated by mymalloc\n",l,f);
		return;
	}
	//handle redundant freeing
	if(tofree->isfree)
	{
		printf("error: call on line %d in file %s points to a block that is already free\n",l,f);
		return;
	}
	//free the memory chunk
	tofree->isfree=1;
	//check for contiguous free memory to concatenate
	while(tofree->next && tofree->next->isfree)
	{
		tofree->size+=tofree->next->size+sizeof(struct memUnit);
		tofree->next=tofree->next->next;
	}
	return;
}
