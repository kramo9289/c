/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"


int correct_index(size_t splitBlockSize){
    int index = 0;

    if(splitBlockSize == 32)
        return 0;
    if(splitBlockSize <= 64 && splitBlockSize > 32)
       index = 1;
    if(splitBlockSize <= 128 && splitBlockSize > 64)
        index = 2;
    if(splitBlockSize <= 256 && splitBlockSize > 128)
        index = 3;
    if(splitBlockSize <= 512 && splitBlockSize > 256)
        index = 4;
    if(splitBlockSize <= 1024 && splitBlockSize > 512)
        index = 5;
    if(splitBlockSize <= 2048 && splitBlockSize > 1024)
        index = 6;
    if(splitBlockSize <= 4096 && splitBlockSize > 2048)
        index = 7;
    if(splitBlockSize > 4096)
        index = 8;

    return index;
}

void *sf_malloc(size_t size) {
    //Check if the size requested is 0, if so return NULL without setting sf_errno
    if((int)size <= 0)
        return NULL;

    //Check to see if this is the first call to malloc. If it is then initialize the heap with
    //the pro and epilogue
    if(sf_mem_start() == sf_mem_end()){
        //grow the heap by one page sz, if NULL then return NULL, sf_mem_grow() already sets erno
        if(sf_mem_grow() == NULL)
            return NULL;

        //create a pointer of type sf_prologue to point to the beginning of the heap
        sf_prologue *p = sf_mem_start();
        //set the block size to 32 and set the aloc and prev bits to 1
        p->header = 32 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;
        //set the footer for the block with the same contents as the header xored with sf_magic()
        p->footer = p->header ^ sf_magic();

        //create a pointer of type sf_epilogue and point it to the end of the heap
        sf_epilogue *e = (void*)sf_mem_end() - sizeof(sf_epilogue);
        //set the block size to 0 adn set the aloc bit to 1, prv is not set as there is no aloc
        e->header = 0 | THIS_BLOCK_ALLOCATED;

        //create a pointer to the free block and add it after prologue
        sf_block *b = sf_mem_start() + sizeof(sf_prologue) - sizeof(sf_footer);

        //set the next and prev sf_block pointer to the dummy sentinal at the range (64M, 128M]
        b->body.links.prev = &sf_free_list_heads[7];
        b->body.links.next = &sf_free_list_heads[7];
        //set the block size to page sz - the size fo the prologue and epilogue
        size_t blockSize = (PAGE_SZ - sizeof(sf_prologue)- sizeof(sf_epilogue));

        //sets the prv aloc bit to 1
        b->header = blockSize | PREV_BLOCK_ALLOCATED;
        //point to the next block which will be the epilogue. This will be casted into an sf_block where
        //prev_footer will contain the value of the free block xored with sf_magic
        sf_block * nextBlock = sf_mem_end() - sizeof(sf_epilogue) - sizeof(sf_footer);
        nextBlock->prev_footer = b->header ^ sf_magic();

        //set the sentinals to point to themsleves initially
        for(int i=0; i<NUM_FREE_LISTS; i++){
            sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
            sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        }
        //set the free list at the range 64M - 128M ie index 7 to the newly created free block
        sf_free_list_heads[7].body.links.next = b;
        sf_free_list_heads[7].body.links.prev = b;
    }
    //add 16 bytes to the size requested for the header and footer.
    size_t newSize = size + 16;
    //If the size is still less than the minimal amount of 32 bytes, then set the requested size to 32
    if(newSize < 32)
        newSize = 32;
    //The size is greater than 32 so the size needs to be a multiple of 16
    if(newSize % 16 != 0)
        //changes the size to become a multiple of 16
        newSize = newSize + (16 -(newSize%16));
    //now we will search the entire free list to find a free block
    for(int i = correct_index(newSize); i<NUM_FREE_LISTS; i++){
        //sets a block pointer to the dummy sentinel for each entry in the array
        sf_block* bPtr = sf_free_list_heads[i].body.links.next;
        //this while loop will go through the whole doubly linked list for each of the intervals, stops if the next pointer is the address of the original
        while(bPtr != &sf_free_list_heads[i]){
            //gets the blocksize of the current block
            size_t bSize = (bPtr -> header) & BLOCK_SIZE_MASK;
            //if the block size is greater or equal to the size needed then it is the block that will be allocated
            if(bSize >= newSize){
                //if the remainder leaves a splinter of size less than 32 then the block is not broken
                if(bSize-newSize < 32){

                    //change the size of the block that is allocated
                    if((bPtr->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED)
                        bPtr->header = newSize | 0x3;
                    else
                        bPtr->header = newSize | THIS_BLOCK_ALLOCATED;

                    //remove the block from the free list
                    bPtr->body.links.prev->body.links.next = bPtr->body.links.next;
                    bPtr->body.links.next->body.links.prev = bPtr->body.links.prev;

                    bPtr->body.links.prev = 0;
                    bPtr->body.links.next = 0;

                    //change the footer to match the header
                    sf_block * nBlock = (void*)bPtr + bSize;
                    //xor with sf_magic
                    nBlock->prev_footer = bPtr->header ^ sf_magic();

                    //change the next block to show that it is allocated
                    nBlock->header = nBlock->header | PREV_BLOCK_ALLOCATED;

                    if((nBlock->header & BLOCK_SIZE_MASK) != 0){
                        //get the block after the next block
                        sf_block * nextNextNextBlock = (void*)nBlock + (nBlock->header & BLOCK_SIZE_MASK);
                        nextNextNextBlock->prev_footer = nBlock->header ^ sf_magic();
                    }

                    //return a pointer to the payload
                    return (void*)bPtr->body.payload;
                }
                //there is a remainder that will leave a block size of the correct size
                else{
                    //get the size of the other split block
                    size_t splitBlockSize = bSize - newSize;
                    //change the size of the block that is allocated
                    if((bPtr->header & PREV_BLOCK_ALLOCATED)== PREV_BLOCK_ALLOCATED)
                        bPtr->header = newSize | 0x3;
                    else
                        bPtr->header = newSize | THIS_BLOCK_ALLOCATED;

                    //remove the block from the free list
                    bPtr->body.links.prev->body.links.next = bPtr->body.links.next;
                    bPtr->body.links.next->body.links.prev = bPtr->body.links.prev;

                    bPtr->body.links.prev = 0;
                    bPtr->body.links.next = 0;

                    sf_block * newFreeBlock = (void*)bPtr + newSize;
                    newFreeBlock->prev_footer = bPtr->header ^ sf_magic();


                    //get the index of the freelisthead arr where the block will be added to
                    int index = correct_index(splitBlockSize);

                    //set the header for the new free block
                    newFreeBlock->header = splitBlockSize | PREV_BLOCK_ALLOCATED;

                    //change the  footer of the new block
                    sf_block * blockAfterNewFreeBlock = (void*)newFreeBlock + splitBlockSize ;
                    blockAfterNewFreeBlock->prev_footer = newFreeBlock->header ^ sf_magic();

                    //change the prev alloc bit to 0 for the block after
                    blockAfterNewFreeBlock->header = blockAfterNewFreeBlock->header & 0xfffffffe;

                    //adds the new free block to the appropriate free list
                    sf_free_list_heads[index].body.links.next->body.links.prev = newFreeBlock;
                    newFreeBlock->body.links.next = sf_free_list_heads[index].body.links.next;
                    sf_free_list_heads[index].body.links.next = newFreeBlock;
                    newFreeBlock->body.links.prev = &sf_free_list_heads[index];

                    if((blockAfterNewFreeBlock->header & BLOCK_SIZE_MASK) != 0){
                        //get the block after the next block
                        sf_block * nextNextNextBlock = (void*)blockAfterNewFreeBlock + (blockAfterNewFreeBlock->header & BLOCK_SIZE_MASK);
                        nextNextNextBlock->prev_footer = blockAfterNewFreeBlock->header ^ sf_magic();
                    }


                    //return the payload of the new allocated block
                    return (void*)bPtr->body.payload;
                }
            }
            //goes on to the next free block
            bPtr = bPtr->body.links.next;
        }
    }

    //if the functions reaches this then there were no free blocks that can be allocated with size
    //get a pointer to the sf_block representing the epilogue and the previous header
    sf_block * prevEndBlock = sf_mem_end() - sizeof(sf_epilogue) - sizeof(sf_footer);
    //get the header from the footer before the epilogue
    sf_header prevHeader = prevEndBlock->prev_footer ^ sf_magic();
    //check to see if this block is allocated or not
    if((prevHeader & THIS_BLOCK_ALLOCATED)== THIS_BLOCK_ALLOCATED){
        //call sf mem grow to get an additional page of information
        if(sf_mem_grow() == NULL)
            return NULL;
        //change the header of the new freed block
        prevEndBlock->header = PAGE_SZ | PREV_BLOCK_ALLOCATED;
        //put a pointer to the the new end block
        void* newPtr = sf_mem_end();
        sf_block * newEndBlock = newPtr - sizeof(sf_epilogue) - sizeof(sf_footer);
        //set the prev footer of the epilogue to the header of the new freed block xord with sf magic
        newEndBlock-> prev_footer = prevEndBlock->header ^ sf_magic();
        //set the epilogue of the new end block
        newEndBlock->header = 0 | THIS_BLOCK_ALLOCATED | PREV_BLOCK_ALLOCATED;

        //add the new freed block to the appropriate free list
        int index = correct_index(PAGE_SZ);
        sf_free_list_heads[index].body.links.next->body.links.prev = prevEndBlock;
        prevEndBlock->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = prevEndBlock;
        prevEndBlock->body.links.prev = &sf_free_list_heads[index];

        //call malloc again
        return sf_malloc(size);
    }
    //this means that the prev block was not alloc. This will attempt to moira it to the previous block which is free
    else{
        //call sf mem grow to get an additional page of information
        void* newPtr = sf_mem_grow();
        if(newPtr == NULL)
            return NULL;
        //get a pointer to the block of the prev freed block
        sf_block * prevFreeBlock = newPtr - (prevHeader & BLOCK_SIZE_MASK) - sizeof(sf_footer) - sizeof(sf_epilogue);
        //change the header to the previous free block and add PAGE SZ to it
        prevFreeBlock->header = prevFreeBlock->header + PAGE_SZ;

        //remove the block from the free list
        prevFreeBlock->body.links.prev->body.links.next = prevFreeBlock->body.links.next;
        prevFreeBlock->body.links.next->body.links.prev = prevFreeBlock->body.links.prev;

        //add the new freed block to the appropriate free list
        int index = correct_index(prevFreeBlock->header & BLOCK_SIZE_MASK);
        sf_free_list_heads[index].body.links.next->body.links.prev = prevFreeBlock;
        prevFreeBlock->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = prevFreeBlock;
        prevFreeBlock->body.links.prev = &sf_free_list_heads[index];

        //put a pointer to the the new end block
        newPtr = sf_mem_end();
        sf_block * newEndBlock = newPtr - sizeof(sf_epilogue) - sizeof(sf_footer);
        //set the prev footer of the epilogue to the header of the new freed block xord with sf magic
        newEndBlock-> prev_footer = prevFreeBlock->header^sf_magic();
        //set the epilogue of the new end block
        if((prevFreeBlock->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED)
            newEndBlock->header = 0 | THIS_BLOCK_ALLOCATED| PREV_BLOCK_ALLOCATED;
        else
            newEndBlock->header = 0 | THIS_BLOCK_ALLOCATED;

        return sf_malloc(size);
    }

    //if it reaches here then there was an error
    sf_errno = ENOMEM;
    return NULL;
}

void sf_free(void *pp) {
    //set the blocktobefreed to 16 bytes behind the payload pointer
    sf_block * blockToBeFreed = pp - 16;
    //if the pointer is null abort
    if(pp == NULL)
        abort();
    //check to see if the pointer is less than the end of prologue
    if((void*)blockToBeFreed + 8 < ((void*)sf_mem_start() + sizeof(sf_prologue)))
        abort();
    //check to see if the pointer is greater then the start of epilogue
    if((void*)blockToBeFreed + 8 > ((void*)sf_mem_end() - sizeof(sf_epilogue)))
        abort();
    //check to see if allocted bit is not 1
    if((blockToBeFreed->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)
        abort();
    //check to see if block size is less than 32 bytes
    if((blockToBeFreed -> header & BLOCK_SIZE_MASK) < 32)
        abort();
    //check to see if prev alloc is 0 and compare it with the previous block header
    if((blockToBeFreed -> header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
        //if the previous header is not alloc then it is invalid
        if((((blockToBeFreed -> prev_footer) ^ sf_magic()) & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)
            abort();
    }
    //compare the prev footer with the actual header of the previous block
    sf_header prevFooter = ((blockToBeFreed -> prev_footer) ^ sf_magic());
    //goes to the previous block
    sf_block * previousBlock = (void*)blockToBeFreed - (prevFooter & BLOCK_SIZE_MASK);
    //gets the header
    sf_header prevHeader = previousBlock->header;
    //if they are not equal, then abort
    if(prevFooter != prevHeader)
        abort();

    //the pointer is a valid one, now attempts to free the curernt block
    //get the block after the current block to check for information
    sf_block * nextBlock = (void *)blockToBeFreed + (blockToBeFreed->header & BLOCK_SIZE_MASK);

    //if the nextBlock and the previous block are both alloc then there is no need to moira, just free the block and add it to the free list
    if(((previousBlock->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED) && ((nextBlock->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED)){
        //change the aloc bit of block to 0
        blockToBeFreed->header = blockToBeFreed->header & 0xfffffffd;
        //add the block to the free list
        int index = correct_index(blockToBeFreed->header & BLOCK_SIZE_MASK);
        sf_free_list_heads[index].body.links.next->body.links.prev = blockToBeFreed;
        blockToBeFreed->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = blockToBeFreed;
        blockToBeFreed->body.links.prev = &sf_free_list_heads[index];
        //change the footer to match the new header
        printf("print the previous footer of the block to be free %ld\n", blockToBeFreed->header);
        nextBlock->prev_footer = blockToBeFreed->header ^ sf_magic();
        //change the next block prev aloc bit to 0
        nextBlock->header = nextBlock->header & 0xfffffffe;
        printf("print the next block of the block to be free %ld\n", nextBlock->prev_footer ^ sf_magic());
        //get the block after the next block
        sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
        nextNextBlock->prev_footer = nextBlock->header ^ sf_magic();

        //if the nextnextblock is the epilogue ignore
        if((nextNextBlock->header & BLOCK_SIZE_MASK) != 0){
            //get the block after the next block
            sf_block * nextNextNextBlock = (void*)nextNextBlock + (nextNextBlock->header & BLOCK_SIZE_MASK);
            nextNextNextBlock->prev_footer = nextNextBlock->header ^ sf_magic();
        }

        return;
    }
    //if the prev is free but the next isnt the moira the prev and current block
    if(((previousBlock->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) && ((nextBlock->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED)){
        //add the size of the block to be freed to the previous block
        previousBlock->header = previousBlock->header + (blockToBeFreed->header & BLOCK_SIZE_MASK);
        //change the footer of the new block which is found in the prev footer of the next block
        nextBlock->prev_footer = previousBlock->header ^ sf_magic();
        //remove the prev block from free list
        previousBlock->body.links.prev->body.links.next = previousBlock->body.links.next;
        previousBlock->body.links.next->body.links.prev = previousBlock->body.links.prev;

        //readd the block to the list with the new size
        int index = correct_index(previousBlock->header & BLOCK_SIZE_MASK);
        sf_free_list_heads[index].body.links.next->body.links.prev = previousBlock;
        previousBlock->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = previousBlock;
        previousBlock->body.links.prev = &sf_free_list_heads[index];

        //set the values of the blocktobefreed to 0
        blockToBeFreed->prev_footer = 0;
        blockToBeFreed->header=0;

        //change the next block prev aloc bit to 0
        nextBlock->header = nextBlock->header & 0xfffffffe;
        //get the block after the next block
        sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
        nextNextBlock->prev_footer = nextBlock->header ^ sf_magic();

        //if the nextnextblock is the epilogue ignore
        if((nextNextBlock->header & BLOCK_SIZE_MASK) != 0){
            //get the block after the next block
            sf_block * nextNextNextBlock = (void*)nextNextBlock + (nextNextBlock->header & BLOCK_SIZE_MASK);
            nextNextNextBlock->prev_footer = nextNextBlock->header ^ sf_magic();
        }
        //return as everytihng is done
        return;
    }
    //if the next is free but prev isnt the moira the NEXt and current block
    if(((previousBlock->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED) && ((nextBlock->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)){
        //change the aloc bit of block to 0
        blockToBeFreed->header = blockToBeFreed->header & 0xfffffffd;
        //add the size of the next block  to the current block
        blockToBeFreed->header = blockToBeFreed->header + (nextBlock->header & BLOCK_SIZE_MASK);
        //get the block after the next block
        sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
        //change the footer of the new block which is found in the prev footer of the next block
        nextNextBlock->prev_footer = blockToBeFreed->header ^ sf_magic();
        //remove the next block from free list
        nextBlock->body.links.prev->body.links.next = nextBlock->body.links.next;
        nextBlock->body.links.next->body.links.prev = nextBlock->body.links.prev;

        //readd the block to the list with the new size
        int index = correct_index(blockToBeFreed->header & BLOCK_SIZE_MASK);
        sf_free_list_heads[index].body.links.next->body.links.prev = blockToBeFreed;
        blockToBeFreed->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = blockToBeFreed;
        blockToBeFreed->body.links.prev = &sf_free_list_heads[index];

        //set the values of the nextblock to 0
        nextBlock->prev_footer = 0;
        nextBlock->header=0;

        //change the next next block prev aloc bit to 0
        nextNextBlock->header = nextNextBlock->header & 0xfffffffe;

        //if the nextnextblock is the epilogue ignore
        if((nextNextBlock->header & BLOCK_SIZE_MASK) != 0){
            //get the block after the next block
            sf_block * nextNextNextBlock = (void*)nextNextBlock + (nextNextBlock->header & BLOCK_SIZE_MASK);
            nextNextNextBlock->prev_footer = nextNextBlock->header ^ sf_magic();
        }
        //return as everytihng is done
        return;
    }
    //if both of the blocks are free, then moira all 3 blocks
    if(((previousBlock->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED) && ((nextBlock->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED)){
        //add the size of all 3 blocks to prev block
        previousBlock->header = previousBlock->header + (blockToBeFreed->header & BLOCK_SIZE_MASK) + (nextBlock->header & BLOCK_SIZE_MASK);
        //get the block after the next block
        sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
        //change the footer of the new block which is found in the prev footer of the next block
        nextNextBlock->prev_footer = previousBlock->header ^ sf_magic();
        //remove both blocks from the free list
        nextBlock->body.links.prev->body.links.next = nextBlock->body.links.next;
        nextBlock->body.links.next->body.links.prev = nextBlock->body.links.prev;

        previousBlock->body.links.prev->body.links.next = previousBlock->body.links.next;
        previousBlock->body.links.next->body.links.prev = previousBlock->body.links.prev;

        //readd the block to the list with the new size
        int index = correct_index(previousBlock->header & BLOCK_SIZE_MASK);
        sf_free_list_heads[index].body.links.next->body.links.prev = previousBlock;
        previousBlock->body.links.next = sf_free_list_heads[index].body.links.next;
        sf_free_list_heads[index].body.links.next = previousBlock;
        previousBlock->body.links.prev = &sf_free_list_heads[index];

        //set the values of the nextblock to 0
        nextBlock->prev_footer = 0;
        nextBlock->header=0;
        //set the values of the blocktobefreed to 0
        blockToBeFreed->prev_footer = 0;
        blockToBeFreed->header=0;

        //change the next next block prev aloc bit to 0
        nextNextBlock->header = nextNextBlock->header & 0xfffffffe;

        //if the nextnextblock is the epilogue ignore
        if((nextNextBlock->header & BLOCK_SIZE_MASK) != 0){
            //get the block after the next block
            sf_block * nextNextNextBlock = (void*)nextNextBlock + (nextNextBlock->header & BLOCK_SIZE_MASK);
            nextNextNextBlock->prev_footer = nextNextBlock->header ^ sf_magic();
        }
        //return as everytihng is done
        return;
    }


    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    //set the blocktobefreed to 16 bytes behind the payload pointer
    sf_block * blockToResize = pp - 16;
    //if the pointer is null abort
    if(pp == NULL){
        sf_errno = EINVAL;
        abort();
    }
    //check to see if the pointer is less than the end of prologue
    if((void*)blockToResize + 8 < ((void*)sf_mem_start() + sizeof(sf_prologue))){
        sf_errno = EINVAL;
        abort();
    }
    //check to see if the pointer is greater then the start of epilogue
    if((void*)blockToResize + 8 > ((void*)sf_mem_end() - sizeof(sf_epilogue))){
        sf_errno = EINVAL;
        abort();
    }
    //check to see if allocted bit is not 1
    if((blockToResize->header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED){
        sf_errno = EINVAL;
        abort();
    }
    //check to see if block size is less than 32 bytes
    if((blockToResize -> header & BLOCK_SIZE_MASK) < 32){
        sf_errno = EINVAL;
        abort();
    }
    //check to see if prev alloc is 0 and compare it with the previous block header
    if((blockToResize -> header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
        //if the previous header is not alloc then it is invalid
        if((((blockToResize -> prev_footer) ^ sf_magic()) & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED){
            sf_errno = EINVAL;
            abort();
        }
    }
    //compare the prev footer with the actual header of the previous block
    sf_header prevFooter = ((blockToResize -> prev_footer) ^ sf_magic());
    //goes to the previous block
    sf_block * previousBlock = (void*)blockToResize - (prevFooter & BLOCK_SIZE_MASK);
    //gets the header
    sf_header prevHeader = previousBlock->header;
    //if they are not equal, then abort
    if(prevFooter != prevHeader){
        sf_errno = EINVAL;
        abort();
    }

    //the pointer is valid so now check size
    if(rsize == 0)
        return NULL;
    //get the block after the current block to check for information
    sf_block * nextBlock = (void *)blockToResize + (blockToResize->header & BLOCK_SIZE_MASK);
    //payload size is the subtraction of the two adddresses minus 16
    size_t payloadSize = (void *)nextBlock -(void *)blockToResize - 16;

    //the client requested the same size so return the same pointer
    if(rsize == payloadSize)
        return pp;
    //if client requests a larger size
    if(rsize > payloadSize){
        //get a pointer with the approriate size
        void* ptr = sf_malloc(rsize);
        if(ptr == NULL)
            return NULL;
        //copy the data from the client pp to the new ptr
        memcpy(ptr, pp, payloadSize);
        //free the block that was passed by the client
        sf_free(pp);
        return ptr;
    }
    //if the client requests a smaller size
    if(rsize < payloadSize){
        //the split would cause a splinter
        if(payloadSize - rsize < 32){
            //there is notihng to update so the ptr is returned to the client
            return pp;
        }
        //the split will not cause a splinter so it is split
        else{
            //create a new size for the current block to resize
            size_t newrsize = rsize + (16 -(rsize%16));
            //save the prevsize fo blocktoresize
            size_t prevSize = (blockToResize->header & BLOCK_SIZE_MASK);

            //used to check if the next block is free. If it is then moirad require
            sf_block * nextBlock = (void*)blockToResize + (blockToResize->header & BLOCK_SIZE_MASK);

            //keep the same information that is has about prev aloc and aloc
            blockToResize->header = (blockToResize->header & 0x3);
            //add 16 for the footer and the header
            blockToResize->header = blockToResize->header + newrsize + 16;
            //create a pointer to the next block which will now be free
            sf_block * splitFreeBlock = (void*)blockToResize + (blockToResize->header & BLOCK_SIZE_MASK);
            splitFreeBlock->header =  prevSize - (blockToResize->header & BLOCK_SIZE_MASK);

            //set the previous footer of the new block to the previous footer
            splitFreeBlock ->prev_footer = blockToResize->header ^ sf_magic();

            //check if the next block is free. If it is then moirad required
            if((nextBlock-> header & THIS_BLOCK_ALLOCATED) != THIS_BLOCK_ALLOCATED){
                //change the aloc bit of block to 0
                splitFreeBlock->header = splitFreeBlock->header & 0xfffffffd;
                //add the size of the next block  to the current block
                splitFreeBlock->header = splitFreeBlock->header + (nextBlock->header & BLOCK_SIZE_MASK);
                //make the prev aloc bit to 1
                splitFreeBlock->header = splitFreeBlock->header | PREV_BLOCK_ALLOCATED;
                //get the block after the next block
                sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
                //change the footer of the new block which is found in the prev footer of the next block
                nextNextBlock->prev_footer = splitFreeBlock->header ^ sf_magic();
                //remove the next block from free list
                nextBlock->body.links.prev->body.links.next = nextBlock->body.links.next;
                nextBlock->body.links.next->body.links.prev = nextBlock->body.links.prev;

                //readd the block to the list with the new size
                int index = correct_index(splitFreeBlock->header & BLOCK_SIZE_MASK);
                sf_free_list_heads[index].body.links.next->body.links.prev = splitFreeBlock;
                splitFreeBlock->body.links.next = sf_free_list_heads[index].body.links.next;
                sf_free_list_heads[index].body.links.next = splitFreeBlock;
                splitFreeBlock->body.links.prev = &sf_free_list_heads[index];

                //set the values of the nextblock to 0
                nextBlock->prev_footer = 0;
                nextBlock->header=0;

                //change the next next block prev aloc bit to 0
                nextNextBlock->header = nextNextBlock->header & 0xfffffffe;

                //if the nextnextblock is the epilogue ignore
                if((nextNextBlock->header & BLOCK_SIZE_MASK) != 0){
                    //get the block after the next block
                    sf_block * nextNextNextBlock = (void*)nextNextBlock + (nextNextBlock->header & BLOCK_SIZE_MASK);
                    nextNextNextBlock->prev_footer = nextNextBlock->header ^ sf_magic();
                }

                //everything is done so the pointer is returned
                return pp;
            }
            //the next block isnt free so not moira required
            else{
                //change the aloc bit of block to 0
                splitFreeBlock->header = splitFreeBlock->header & 0xfffffffd;
                //add the block to the free list
                int index = correct_index(splitFreeBlock->header & BLOCK_SIZE_MASK);
                sf_free_list_heads[index].body.links.next->body.links.prev = splitFreeBlock;
                splitFreeBlock->body.links.next = sf_free_list_heads[index].body.links.next;
                sf_free_list_heads[index].body.links.next = splitFreeBlock;
                splitFreeBlock->body.links.prev = &sf_free_list_heads[index];
                //change the footer to match the new header
                nextBlock->prev_footer = splitFreeBlock->header ^ sf_magic();
                //change the next block prev aloc bit to 0
                nextBlock->header = nextBlock->header & 0xfffffffe;

                //get the block after the next block
                sf_block * nextNextBlock = (void*)nextBlock + (nextBlock->header & BLOCK_SIZE_MASK);
                nextNextBlock->prev_footer = nextBlock->header ^ sf_magic();

                //everything is done so the pointer is returned
                return pp;
            }

        }
    }


    return NULL;
}