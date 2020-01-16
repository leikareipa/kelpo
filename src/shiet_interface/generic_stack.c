/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A generic LIFO stack whose data is stored in a contiguous block of memory.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <shiet_interface/generic_stack.h>

/* When growing a stack, its new size will be its current allocated size multiplied
 * by this value and floored to an integer.*/
static const float STACK_GROWTH_MULTIPLIER = 1.5;

/* The smallest allowed number of elements in a stack. If the user requests a
 * stack whose initial size is smaller than this, a stack of this size will be
 * given.*/
static const uint32_t MINIMUM_STACK_SIZE = 4;

struct shiet_generic_stack_s* shiet_generic_stack__create(const uint32_t initialElementCount,
                                                          const uint32_t elementByteSize)
{
    struct shiet_generic_stack_s *newStack = (struct shiet_generic_stack_s*)calloc(1, sizeof(struct shiet_generic_stack_s));
    assert(newStack && "Failed to allocate memory for a new stack.");

    newStack->count = 0;
    newStack->elementByteSize = elementByteSize;
    shiet_generic_stack__grow(newStack, initialElementCount);

    return newStack;
}

void shiet_generic_stack__grow(struct shiet_generic_stack_s *const stack,
                               uint32_t newElementCount)
{
    void *newStackBuffer = NULL;

    if (newElementCount < MINIMUM_STACK_SIZE)
    {
        newElementCount = MINIMUM_STACK_SIZE;
    }

    assert(stack && "Attempting to operate on a NULL stack.");
    
    assert((newElementCount > stack->capacity) &&
           (newElementCount > stack->count) &&
           "Trying to grow the stack to a smaller size than what it is now.");

    assert((stack->count <= stack->capacity) &&
           "Attempting to grow a malformed stack.");

    newStackBuffer = calloc(newElementCount, stack->elementByteSize);
    assert(newStackBuffer && "Failed to allocate memory to grow the stack.");

    if (stack->data)
    {
        memcpy(newStackBuffer, stack->data, (stack->count * stack->elementByteSize));
        free(stack->data);
    }

    stack->data = newStackBuffer;
    stack->capacity = newElementCount;

    return;
}

void shiet_generic_stack__push_copy(struct shiet_generic_stack_s *const stack,
                                    const void *const newElement)
{
    if ((stack->count >= stack->capacity) ||
        (stack->capacity < MINIMUM_STACK_SIZE))
    {
        shiet_generic_stack__grow(stack, (stack->capacity * STACK_GROWTH_MULTIPLIER));
    }

    assert((stack->count < stack->capacity) && "Failed to properly grow the stack.");

    memcpy(((uint8_t*)stack->data + (stack->count * stack->elementByteSize)), newElement, stack->elementByteSize);

    stack->count++;

    return;
}

const void* shiet_generic_stack__pop(struct shiet_generic_stack_s *const stack)
{
    assert((stack->count > 0) && "Attempting to pop an empty stack.");

    stack->count--;

    return ((uint8_t*)stack->data + (stack->count * stack->elementByteSize));
}

void* shiet_generic_stack__front(struct shiet_generic_stack_s *const stack)
{
    return ((uint8_t*)stack->data + ((stack->count - 1) * stack->elementByteSize));
}

void* shiet_generic_stack__at(struct shiet_generic_stack_s *const stack,
                              const uint32_t idx)
{
    assert((idx < stack->count) && "Attempting to access the stack out of bounds.");

    return ((uint8_t*)stack->data + (idx * stack->elementByteSize));
}

void shiet_generic_stack__clear(struct shiet_generic_stack_s *const stack)
{
    stack->count = 0;

    return;
}

void shiet_generic_stack__free(struct shiet_generic_stack_s *const stack)
{
    free(stack->data);
    free(stack);

    return;
}
