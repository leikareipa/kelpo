/*
 * 2020 Tarpeeksi Hyvae Soft
 * 
 * Provides a contiguously-stored LIFO stack for holding triangles of the shiet
 * renderer.
 * 
 */

#ifndef SHIET_INTERFACE_POLYGON_TRIANGLE_TRIANGLE_STACK_H
#define SHIET_INTERFACE_POLYGON_TRIANGLE_TRIANGLE_STACK_H

#include <shiet_interface/common/stdint.h>

struct shiet_polygon_triangle_s;

struct shiet_polygon_triangle_stack_s
{
    /* A contiguous block of memory holding the stack's allocated elements.
     * Elements in the range [0,size) contain the stack's current triangle data;
     * while elements in the range [size,allocatedSize) contain data that is
     * either stale or uninitialized and to which the stack's size will grow
     * as new triangles are pushed on.*/
    struct shiet_polygon_triangle_s *data;

    /* The number of stack elements in use.*/
    uint32_t count;

    /* The number of elements allocated for the stack.*/
    uint32_t capacity;
};

/* Creates a new stack.*/
struct shiet_polygon_triangle_stack_s* shiet_tristack_create(const uint32_t initialElementCount);

/* Increases the stack's allocated size. To guarantee that the stack's data remain
 * contiguous in memory, the new data area will be obtained from an entirely new
 * allocation and the existing data will be migrated there (all previous pointers
 * to these data will become invalid).*/
void shiet_tristack_grow(struct shiet_polygon_triangle_stack_s *const stack,
                         uint32_t newElementCount);

/* Adds a copy of the given triangle onto the stack. The stack will be grown as
 * needed to accommodate the addition.*/
void shiet_tristack_push_copy(struct shiet_polygon_triangle_stack_s *const stack,
                              struct shiet_polygon_triangle_s *const triangle);

/* Removes the most recently added element from the stack and returns a pointer
 * to it. The removal is simply a decrementing of an index value; no memory is
 * deallocated. The data pointed to by the returned pointer remains valid until
 * a new element is pushed onto the stack (which overwrites the data), the stack
 * is asked to grow (which deallocates the original data), or the stack is freed.*/
const struct shiet_polygon_triangle_s* shiet_tristack_pop(struct shiet_polygon_triangle_stack_s *const stack);

/* Returns a pointer to the most recently added element.*/
struct shiet_polygon_triangle_s* shiet_tristack_front(struct shiet_polygon_triangle_stack_s *const stack);

/* Returns a pointer to the idx'th element.*/
struct shiet_polygon_triangle_s* shiet_tristack_at(struct shiet_polygon_triangle_stack_s *const stack,
                                                   const uint32_t idx);

/* Removes all existing elements from the stack, but doesn't deallocate their
 * memory. The memory will be reused for new elements pushed onto the stack.*/
void shiet_tristack_clear(struct shiet_polygon_triangle_stack_s *const stack);

/* Deallocates all memory allocated for the stack, including the stack pointer
 * itself. After this call, the stack pointer should be considered invalid, as
 * should any existing pointers to the stack's data.*/
void shiet_tristack_free(struct shiet_polygon_triangle_stack_s *const stack);

#endif
