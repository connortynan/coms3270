#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "util/vector.h"
#include "util/heap.h"

/**
 * @brief Represents a node in the pathfinding algorithm.
 *
 * This structure stores information about a node, including its
 * cost, a pointer to user-defined data, and the index of the
 * previous node in the path.
 */
typedef struct pathing_node
{
    void *data;      /**< Pointer to user-defined data associated with this node. */
    uint64_t cost;   /**< The accumulated cost to reach this node. */
    size_t prev_idx; /**< Index of the previous node in the computed path. */
} pathing_node;

struct pathing_context;

/**
 * @brief Function type for evaluating the movement cost of a node.
 *
 * This function is responsible for computing the cost associated with moving
 * to a given node in the pathfinding algorithm.
 *
 * @param node_idx The index of the node being evaluated.
 * @param ctx The pathfinding context, which may contain additional state.
 * @return The computed movement cost for the given node.
 */
typedef uint64_t (*pathing_cost_eval_func)(size_t node_idx, struct pathing_context *ctx);

/**
 * @brief Function type for evaluating the neighbors of a node.
 *
 * This function should iterate through all neighbors of the given node
 * and call `pathing_eval_node` for each valid neighboring node.
 *
 * @param node_idx The index of the current node whose neighbors are being evaluated.
 * @param ctx The pathfinding context containing all nodes.
 */
typedef void (*pathing_evaluate_neighbors)(size_t node_idx, struct pathing_context *ctx);

/**
 * @brief Function type for determining if the end condition has been met.
 *
 * This function checks whether a given node satisfies the termination
 * criteria for the pathfinding algorithm, such as reaching the destination.
 *
 * @param node_idx The index of the node being checked.
 * @param ctx The pathfinding context.
 * @return Non-zero if the end condition is met, otherwise 0.
 */
typedef int (*pathing_end_condition)(size_t node_idx, struct pathing_context *ctx);

/**
 * @brief Structure for managing the pathfinding process.
 *
 * This structure contains all relevant information about the
 * pathfinding algorithm's state, including the list of nodes,
 * priority queue, and function pointers for evaluating costs
 * and neighbors.
 */
typedef struct pathing_context
{
    pathing_node *nodes;                        /**< Array of nodes in the pathfinding graph. */
    size_t num_nodes;                           /**< Total number of nodes in the graph. */
    heap *pq;                                   /**< Min-heap used for priority queue operations. */
    pathing_cost_eval_func cost_eval_func;      /**< Function for calculating movement cost. */
    pathing_evaluate_neighbors eval_nbors_func; /**< Function for evaluating neighboring nodes. */
    pathing_end_condition end_cond_func;        /**< Function to check if the goal node has been reached. */
    void *data;
} pathing_context;

/**
 * @brief Comparator function for heap operations in pathfinding.
 *
 * This function compares two nodes in the pathfinding context based on their cost.
 * It is used for maintaining the priority queue (min-heap) in Dijkstra's algorithm.
 *
 * @param a Pointer to the first node index.
 * @param b Pointer to the second node index.
 * @param context Pointer to the pathing context.
 * @return A negative value if the first node has a lower cost,
 *         a positive value if the second node has a lower cost,
 *         and 0 if they are equal.
 */
static int pathing_node_compare(const void *a, const void *b, void *context)
{
    pathing_context *p_ctx = (pathing_context *)context;

    size_t idx_a = *((const size_t *)a); // Index of node A
    size_t idx_b = *((const size_t *)b); // Index of node B
    const pathing_node *node_a = &p_ctx->nodes[idx_a];
    const pathing_node *node_b = &p_ctx->nodes[idx_b];

    return (node_a->cost > node_b->cost) - (node_a->cost < node_b->cost);
}

/**
 * @brief Evaluates a neighboring node in the pathfinding process.
 *
 * This function calculates the new cost of a neighboring node and updates it
 * if a shorter path is found. If the cost is updated, the neighbor is added
 * to the priority queue.
 *
 * @param current_idx Index of the current node.
 * @param neighbor_idx Index of the neighboring node being evaluated.
 * @param ctx Pointer to the pathing context.
 */
static inline void pathing_eval_node(
    size_t current_idx, size_t neighbor_idx,
    pathing_context *ctx)
{
    pathing_node *neighbor = &ctx->nodes[neighbor_idx];

    uint64_t new_cost = ctx->nodes[current_idx].cost + ctx->cost_eval_func(neighbor_idx, ctx);
    if (new_cost < neighbor->cost)
    {
        neighbor->cost = new_cost;
        neighbor->prev_idx = current_idx;
        heap_insert(ctx->pq, &neighbor_idx);
    }
}

/**
 * @brief Computes the shortest path using a priority queue (Dijkstra's algorithm).
 *
 * This function implements a generalized version of Dijkstra’s algorithm to find
 * the shortest path in a graph. It maintains a priority queue to process nodes
 * in order of their current known cost.
 *
 * @param start_idx Index of the starting node.
 * @param num_nodes Total number of nodes in the graph.
 * @param cost_eval Function that calculates the movement cost between nodes.
 * @param eval_neighbors Function that evaluates a node's neighbors.
 * @param end_cond Function that determines if the end condition is met.
 * @return A dynamically allocated vector containing the indices of nodes
 *         forming the shortest path, or NULL if no path is found.
 */
static inline vector *pathing_solve(
    size_t start_idx, size_t num_nodes,
    void *node_data, size_t node_data_elem_size,
    void *ctx_data,
    pathing_cost_eval_func cost_eval,
    pathing_evaluate_neighbors eval_neighbors,
    pathing_end_condition end_cond)
{
    pathing_context ctx = {
        .num_nodes = num_nodes,
        .cost_eval_func = cost_eval,
        .eval_nbors_func = eval_neighbors,
        .end_cond_func = end_cond,
        .data = ctx_data,
    };

    ctx.nodes = (pathing_node *)malloc(num_nodes * sizeof(pathing_node));
    if (!ctx.nodes)
        return NULL;

    for (size_t i = 0; i < num_nodes; i++)
    {
        if (node_data)
            ctx.nodes[i].data = (uint8_t *)node_data + (i * node_data_elem_size);
        else
            ctx.nodes[i].data = NULL;

        ctx.nodes[i].cost = UINT64_MAX;
        ctx.nodes[i].prev_idx = SIZE_MAX;
    }

    ctx.pq = heap_init(0, sizeof(size_t), pathing_node_compare, &ctx);
    if (!ctx.pq)
    {
        free(ctx.nodes);
        return NULL;
    }

    vector *path = vector_init(0, sizeof(size_t));
    if (!path)
    {
        free(ctx.nodes);
        heap_destroy(ctx.pq);
        return NULL;
    }

    ctx.nodes[start_idx].cost = 0;
    heap_insert(ctx.pq, &start_idx);

    size_t curr_idx;

    while (ctx.pq->_vec->size > 0)
    {
        heap_poll(ctx.pq, &curr_idx);

        if (ctx.end_cond_func(curr_idx, &ctx))
        {
            vector_push_back(path, &curr_idx);
            pathing_node *n = &ctx.nodes[curr_idx];
            while (n->prev_idx != SIZE_MAX)
            {
                vector_push_back(path, &n->prev_idx);
                n = &ctx.nodes[n->prev_idx];
            }
            heap_destroy(ctx.pq);
            free(ctx.nodes);
            return path;
        }

        ctx.eval_nbors_func(curr_idx, &ctx);
    }

    heap_destroy(ctx.pq);
    vector_destroy(path);
    free(ctx.nodes);
    return NULL;
}
