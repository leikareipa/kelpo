/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A generic LIFO stack whose data is stored in a contiguous block of memory.
 * 
 * Usage:
 * 
 *   1. Call __create() to set up a new stack.
 * 
 *   2. Use the appropriate helper functions (__push_copy(), __pop(), etc.) to
 *      manipulate the stack.
 * 
 *   3. If the stack is full when pushing a new element onto it, the stack will
 *      be automatically grown to accommodate.
 * 
 *   4. To clear the stack without deallocating its capacity in memory, call
 *      __clear(). Subsequent new elements will be pushed onto the stack starting
 *      at index 0 and using the existing allocated capacity.
 * 
 *   5. To fully deallocate the cache, call __free(). The stack pointer obtained
 *      in (1) will no longer be valid.
 * 
 */

#ifndef KELPO_INTERFACE_GENERIC_STACK_H
#define KELPO_INTERFACE_GENERIC_STACK_H

#include <kelpo_interface/common/stdint.h>

struct kelpo_generic_stack_s
{
    /* A contiguous block of memory holding the stack's allocated elements.
     * Elements in the range [0,size) contain the stack's current data; while
     * elements in the range [size,allocatedSize) contain data that is either
     * stale or uninitialized and to which the stack's size will grow as new
     * data are pushed on.*/
    void *data;

    /* The size, in bytes, of an individual stack element.*/
    uint32_t elementByteSize;

    /* The number of stack elements in use.*/
    uint32_t count;

    /* The number of elements allocated for the stack.*/
    uint32_t capacity;
};

/* Creates a new stack.*/
struct kelpo_generic_stack_s* kelpo_generic_stack__create(const uint32_t initialElementCount,
                                                          const uint32_t elementByteSize);

/* Increases the stack's allocated size. To guarantee that the stack's data remain
 * contiguous in memory, the new data area will be obtained from an entirely new
 * allocation and the existing data will be migrated there (all previous pointers
 * to these data will become invalid). If the stack's current capacity is equal to
 * or larger than the given new element count, the function will return without
 * doing anything.*/
void kelpo_generic_stack__grow(struct kelpo_generic_stack_s *const stack,
                               uint32_t newElementCount);

/* Adds a shallow copy of the given element of data onto the stack. The stack will
 * be grown as needed to accommodate the addition.*/
void kelpo_generic_stack__push_copy(struct kelpo_generic_stack_s *const stack,
                                    const void *const newElement);

/* Removes the most recently added element from the stack and returns a pointer
 * to it. The removal is simply a decrementing of an index value; no memory is
 * deallocated. The data pointed to by the returned pointer remains valid until
 * a new element is pushed onto the stack (which overwrites the data), the stack
 * is asked to grow (which deallocates the original data), or the stack is freed.*/
const void* kelpo_generic_stack__pop(struct kelpo_generic_stack_s *const stack);

/* Returns a pointer to the most recently added element.*/
void* kelpo_generic_stack__front(struct kelpo_generic_stack_s *const stack);

/* Returns a pointer to the idx'th element.*/
void* kelpo_generic_stack__at(struct kelpo_generic_stack_s *const stack,
                              const uint32_t idx);

/* Removes all existing elements from the stack, but doesn't deallocate their
 * memory. The memory will be reused for new elements pushed onto the stack.*/
void kelpo_generic_stack__clear(struct kelpo_generic_stack_s *const stack);

/* Deallocates all memory allocated for the stack, including the stack pointer
 * itself. After this call, the stack pointer should be considered invalid, as
 * should any existing pointers to the stack's data.*/
void kelpo_generic_stack__free(struct kelpo_generic_stack_s *const stack);

#endif
