/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * A generic LIFO stack whose data is stored in a contiguous block of memory.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <kelpo_auxiliary/generic_stack.h>

/* When growing a stack, its new size will be its current allocated size multiplied
 * by this value and floored to an integer.*/
static const float STACK_GROWTH_MULTIPLIER = 1.5;

/* The smallest allowed number of elements in a stack. If the user requests a
 * stack whose initial size is smaller than this, a stack of this size will be
 * given.*/
static const uint32_t MINIMUM_STACK_SIZE = 4;

struct kelpoa_generic_stack_s* kelpoa_generic_stack__create(const uint32_t initialElementCount,
                                                            const uint32_t elementByteSize)
{
    struct kelpoa_generic_stack_s *newStack = (struct kelpoa_generic_stack_s*)calloc(1, sizeof(struct kelpoa_generic_stack_s));
    assert(newStack && "Failed to allocate memory for a new stack.");

    newStack->count = 0;
    newStack->elementByteSize = elementByteSize;
    kelpoa_generic_stack__grow(newStack, initialElementCount);

    return newStack;
}

void kelpoa_generic_stack__grow(struct kelpoa_generic_stack_s *const stack,
                                uint32_t newElementCount)
{
    if (newElementCount < MINIMUM_STACK_SIZE)
    {
        newElementCount = MINIMUM_STACK_SIZE;
    }

    if (stack->capacity >= newElementCount)
    {
        return;
    }

    assert(stack && "Attempting to operate on a NULL stack.");

    assert((stack->count <= stack->capacity) &&
           "Attempting to grow a malformed stack.");

    stack->data = realloc(stack->data, (newElementCount * stack->elementByteSize));
    assert(stack->data && "Failed to allocate memory to grow the stack.");

    stack->capacity = newElementCount;

    return;
}

void kelpoa_generic_stack__push_copy(struct kelpoa_generic_stack_s *const stack,
                                     const void *const newElement)
{
    if ((stack->count >= stack->capacity) ||
        (stack->capacity < MINIMUM_STACK_SIZE))
    {
        kelpoa_generic_stack__grow(stack, (stack->capacity * STACK_GROWTH_MULTIPLIER));
    }

    assert((stack->count < stack->capacity) && "Failed to properly grow the stack.");

    memcpy(((uint8_t*)stack->data + (stack->count * stack->elementByteSize)), newElement, stack->elementByteSize);

    stack->count++;

    return;
}

const void* kelpoa_generic_stack__pop(struct kelpoa_generic_stack_s *const stack)
{
    assert((stack->count > 0) && "Attempting to pop an empty stack.");

    stack->count--;

    return ((uint8_t*)stack->data + (stack->count * stack->elementByteSize));
}

void* kelpoa_generic_stack__front(struct kelpoa_generic_stack_s *const stack)
{
    return ((uint8_t*)stack->data + ((stack->count - 1) * stack->elementByteSize));
}

void* kelpoa_generic_stack__at(struct kelpoa_generic_stack_s *const stack,
                               const uint32_t idx)
{
    assert((idx < stack->count) && "Attempting to access the stack out of bounds.");

    return ((uint8_t*)stack->data + (idx * stack->elementByteSize));
}

void kelpoa_generic_stack__clear(struct kelpoa_generic_stack_s *const stack)
{
    stack->count = 0;

    return;
}

void kelpoa_generic_stack__free(struct kelpoa_generic_stack_s *const stack)
{
    free(stack->data);
    free(stack);

    return;
}
