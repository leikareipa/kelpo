/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a LIFO stack for storing the shiet renderer's triangles.
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <shiet_interface/polygon/triangle/triangle.h>
#include "triangle_stack.h"

/* When growing a stack, its new size will be its current allocated size multiplied
 * by this value and floored to an integer.*/
static const float STACK_GROWTH_MULTIPLIER = 1.5;

/* The smallest allowed number of elements in a stack. If the user requests a
 * stack whose initial size is smaller than this, a stack of this size will be
 * given.*/
static const uint32_t MINIMUM_STACK_SIZE = 4;

struct shiet_polygon_triangle_stack_s* shiet_tristack_create(const uint32_t initialElementCount)
{
    struct shiet_polygon_triangle_stack_s *newStack = calloc(1, sizeof(struct shiet_polygon_triangle_stack_s));
    assert(newStack && "Failed to allocate memory for a new triangle stack.");

    newStack->count = 0;
    shiet_tristack_grow(newStack, initialElementCount);

    return newStack;
}

void shiet_tristack_grow(struct shiet_polygon_triangle_stack_s *const stack,
                         uint32_t newElementCount)
{
    struct shiet_polygon_triangle_s *newTriangleBuffer;

    if (newElementCount < MINIMUM_STACK_SIZE)
    {
        newElementCount = MINIMUM_STACK_SIZE;
    }

    assert(stack && "Attempting to operate on a NULL triangle stack.");
    
    assert((newElementCount > stack->capacity) &&
           (newElementCount > stack->count) &&
           "Trying to grow the triangle stack to a smaller size than what it is now.");

    assert((stack->count <= stack->capacity) &&
           "Attempting to grow a malformed triangle stack.");

    newTriangleBuffer = calloc(newElementCount, sizeof(stack->data[0]));
    assert(newTriangleBuffer && "Failed to allocate memory to grow the triangle stack.");

    if (stack->data)
    {
        memcpy(newTriangleBuffer, stack->data, (sizeof(stack->data[0]) * stack->count));
        free(stack->data);
    }

    stack->data = newTriangleBuffer;
    stack->capacity = newElementCount;

    return;
}

void shiet_tristack_push_copy(struct shiet_polygon_triangle_stack_s *const stack,
                              struct shiet_polygon_triangle_s *const triangle)
{
    if ((stack->count >= stack->capacity) ||
        (stack->capacity < MINIMUM_STACK_SIZE))
    {
        shiet_tristack_grow(stack, (stack->capacity * STACK_GROWTH_MULTIPLIER));
    }

    stack->data[stack->count++] = *triangle;

    return;
}

const struct shiet_polygon_triangle_s* shiet_tristack_pop(struct shiet_polygon_triangle_stack_s *const stack)
{
    assert((stack->count > 0) && "Attempting to pop an empty stack.");

    stack->count--;

    return &stack->data[stack->count];
}

struct shiet_polygon_triangle_s* shiet_tristack_front(struct shiet_polygon_triangle_stack_s *const stack)
{
    return &stack->data[stack->count - 1];
}

struct shiet_polygon_triangle_s* shiet_tristack_at(struct shiet_polygon_triangle_stack_s *const stack,
                                                   const uint32_t idx)
{
    assert((idx < stack->count) && "Attempting to access the triangle stack out of bounds.");

    return &stack->data[idx];
}

void shiet_tristack_clear(struct shiet_polygon_triangle_stack_s *const stack)
{
    stack->count = 0;

    return;
}

void shiet_tristack_free(struct shiet_polygon_triangle_stack_s *const stack)
{
    stack->count = 0;
    stack->capacity = 0;

    free(stack->data);
    stack->data = NULL;

    free(stack);

    return;
}
