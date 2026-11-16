#include "memalloc.h"
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>

typedef struct BlockHeader{
	alignas(16) size_t size;
	bool isFree;
	struct BlockHeader *next;
}BlockHeader;

static BlockHeader *head=NULL;
static BlockHeader *tail=NULL;

static size_t align16(size_t size){
	return (size + 15) & ~((size_t)15);
}

static void splitBlock(BlockHeader *block, size_t size){
	if( block == NULL || (block->size < size + (sizeof(BlockHeader)+16)))
		return;

	BlockHeader *newBlock = (BlockHeader *)((char *)block + sizeof(BlockHeader) + size);
	newBlock->isFree = true;
	newBlock->size = (size_t)(block->size - size - sizeof(BlockHeader));
	newBlock->next = block->next;

	block->size = size;
	
	if ( tail == block){
		tail->next = newBlock;
		tail = newBlock;
	}
	else{
		block->next = newBlock;
	}
}

static void coalesceBlocks(void){
	BlockHeader *curr = head;
	if(curr == NULL)
		return;

	while( curr!=NULL && curr->next!=NULL ){
		if(curr->isFree && curr->next->isFree){
			if(curr->next == tail){
				tail = curr;
			}
			curr->size += (size_t)( sizeof(BlockHeader) + curr->next->size);
			curr->next = curr->next->next;
		}
		else{
			curr = curr->next;
		}
	}
}

static BlockHeader *findFreeBlock(size_t size) {
	BlockHeader *curr = head;
  while (curr != NULL) {
    if (curr->isFree && curr->size >= size) {
    	return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

void *myMalloc(size_t size){
	if( size == 0)
		return NULL;

	size_t alignedSize = align16(size);
	size_t totalSize = sizeof(BlockHeader) + alignedSize;
	
	BlockHeader *block = findFreeBlock(alignedSize);
	if(block == NULL){

		void *raw_mem = sbrk( (intptr_t)totalSize );
		if( raw_mem == (void *)-1)
			return NULL;

		block = (BlockHeader *)raw_mem;
		block->size = alignedSize;
		block->isFree = false;
		block->next = NULL;

		if( head == NULL )
			head = tail = block;
		else{
			tail->next = block;
			tail = block;
		}
	}else{
		splitBlock(block, alignedSize);
		block->isFree = false;
	}
	return (void *)(block+1);
}

void myFree(void *ptr){
	if( ptr == NULL)
		return;

	BlockHeader *block = ((BlockHeader *)ptr) -1;
	block->isFree = true;

	coalesceBlocks();
}

void *myCalloc(size_t nmemb, size_t size){
	if( nmemb == 0 || size == 0)
		return NULL;

	if ( nmemb > SIZE_MAX / size)
		return NULL;

	size_t totalBytes = nmemb * size;
	void *ptr = myMalloc(totalBytes);
	if(ptr != NULL){
		memset(ptr, 0, totalBytes);
	}
	return ptr;
}

void *myRealloc(void *ptr, size_t size){
	if( ptr == NULL)
		return myMalloc(size);
	if(size == 0){
		myFree(ptr);
		return NULL;
	}

	size_t alignedSize = align16(size);
	
	BlockHeader *block = ((BlockHeader *)ptr)-1;
	if( block->size >= alignedSize){
		splitBlock(block, alignedSize);
		coalesceBlocks();
		return ptr;
	}
	else{
		if( block->next!=NULL && block->next->isFree ){
			if( block->size + sizeof(BlockHeader) + block->next->size >= alignedSize ){
				block->size += (size_t)(sizeof(BlockHeader) + block->next->size);
				if(block->next == tail)
					tail = block;
				block->next = block->next->next;
				splitBlock(block, alignedSize);
				return ptr;
			}
		}
		void *newPtr = myMalloc(size);
		if(newPtr == NULL)
			return NULL;
		memcpy(newPtr, ptr, block->size);
		myFree(ptr);
		return newPtr;
	}
}
