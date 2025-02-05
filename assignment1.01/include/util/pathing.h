#pragma once

#include <stdint.h>

#include "util/vector.h"
#include "util/heap.h"

#define WITHIN_BOUNDS(x, y, w, h) (((x) >= 0) && ((x) < w) && ((y) >= 0) && ((y) < h))

/**
 * @brief 2D vector structure with unsigned 16-bit integer components.
 */
typedef struct vec2_u16
{
    uint16_t x; /**< @brief X component of the vector */
    uint16_t y; /**< @brief Y component of the vector */
} vec2_u16;

/**
 * @brief A structure to represent a node in the grid for Dijkstra's algorithm.
 */
typedef struct pathing_node
{
    vec2_u16 pos;              /**< (x, y) coordinate of the node in the grid */
    uint64_t cost;             /**< tentative distance of the node */
    struct pathing_node *prev; /** previous noe in the current path */
} pathing_node;

/**
 * @brief A comparison function for heap elements (used for Dijkstra's algorithm).
 * This function compares two nodes based on their tentative distances.
 *
 * @param a Pointer to the first node
 * @param b Pointer to the second node
 *
 * @return Negative if a < b, 0 if a == b, and positive if a > b
 */
static int node_compare(const void *a, const void *b)
{
    pathing_node *node_a = (pathing_node *)a;
    pathing_node *node_b = (pathing_node *)b;

    if (node_a->cost < node_b->cost)
        return -1;
    else if (node_a->cost > node_b->cost)
        return 1;
    else
        return 0;
}

/**
 * @brief Solves the pathfinding problem using the provided weight matrix.
 *
 * Given a weight matrix (in a row-major format) representing a grid, this
 * function calculates the shortest path between the start point and the
 * end point.
 *
 * @note The `weights` array is expected to be in **row-major** format, where
 *       each row is contiguous in memory, and each element represents the
 *       weight of a grid cell.
 *
 * @param weights Pointer to the matrix of weights (row-major).
 * @param width The width of the grid (number of columns).
 * @param height The height of the grid (number of rows).
 * @param start_x The x-coordinate (column) of the start position.
 * @param start_y The y-coordinate (row) of the start position.
 * @param end_x The x-coordinate (column) of the end position.
 * @param end_y The y-coordinate (row) of the end position.
 *
 * @return A vector containing the path from start to end, or NULL if no
 *         path is found. The path is returned as a sequence of `vec2_u16`
 *         coordinates.
 */
static inline vector *
pathing_solve(
    uint8_t *weights, /**< @brief Pointer to the matrix of weights (row-major) */
    uint16_t width,   /**< @brief The width (number of columns) of the grid */
    uint16_t height,  /**< @brief The height (number of rows) of the grid */
    uint16_t start_x, /**< @brief The x-coordinate (column) of the start position */
    uint16_t start_y, /**< @brief The y-coordinate (row) of the start position */
    uint16_t end_x,   /**< @brief The x-coordinate (column) of the end position */
    uint16_t end_y    /**< @brief The y-coordinate (row) of the end position */
)
{
    vector *path = vector_init(0, sizeof(vec2_u16));
    if (!path)
        return NULL;

    pathing_node *nodes = (pathing_node *)malloc(width * height * sizeof(*nodes));
    if (!nodes)
    {
        vector_destroy(path);
        return NULL;
    }

    uint16_t x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pathing_node *n = &nodes[x + y * width];
            n->pos.x = x;
            n->pos.y = y;
            n->cost = UINT32_MAX;
            n->prev = NULL;
        }
    }

    pathing_node *start_node = &nodes[start_x + start_y * width];
    start_node->cost = 0;

    heap *q = heap_init(0, sizeof(pathing_node), node_compare);
    if (!q)
    {
        free(nodes);
        vector_destroy(path);
        return NULL;
    }
    heap_insert(q, start_node);

    static const int moves[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    while (q->_vec->size > 0)
    {
        // Get the node with the smallest tentative distance
        pathing_node *curr = (pathing_node *)heap_poll(q);

        // If we reached the end, reconstruct the path
        if (curr->pos.x == end_x && curr->pos.y == end_y)
        {
            pathing_node *n = curr;
            while (n != NULL)
            {
                vec2_u16 v = {n->pos.x, n->pos.y};
                vector_push_back(path, &v);
                n = n->prev;
            }

            // Reverse the path since we built it backwards
            for (size_t i = 0; i < path->size / 2; ++i)
            {
                vec2_u16 *a = (vec2_u16 *)vector_at(path, i);
                vec2_u16 *b = (vec2_u16 *)vector_at(path, path->size - i - 1);
                vec2_u16 temp = *a;
                *a = *b;
                *b = temp;
            }

            free(curr);
            heap_destroy(q);
            free(nodes);

            return path;
        }

        // Explore neighbors
        for (int i = 0; i < 4; ++i)
        {
            uint16_t nx = curr->pos.x + moves[i][0];
            uint16_t ny = curr->pos.y + moves[i][1];

            if (WITHIN_BOUNDS(nx, ny, width, height))
            {
                uint32_t new_cost = curr->cost + weights[ny * width + nx];
                pathing_node *nbor = &nodes[ny * width + nx];
                if (new_cost < nbor->cost)
                {
                    nbor->cost = new_cost;
                    nbor->prev = curr;
                    heap_insert(q, nbor);
                }
            }
        }
        free(curr);
    }

    // No path found
    free(nodes);
    heap_destroy(q);
    vector_destroy(path);
    return NULL;
}
