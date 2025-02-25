#pragma once

#include <stdint.h>
#include "util/vector.h"
#include "util/heap.h"

typedef struct pathing_node
{
    void *data;      /**< Pointer to user-defined data associated with this node. */
    uint64_t cost;   /**< The accumulated cost to reach this node. */
    size_t prev_idx; /**< Index of the previous node in the computed path. */
} pathing_node;

struct pathing_context;

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
 *
 * @note The user must call this function for each node they want to check within the `pathing_evaluate_neighbors` function
 */
void pathing_eval_node(size_t current_idx, size_t neighbor_idx, struct pathing_context *ctx);

typedef uint64_t (*pathing_cost_eval_func)(size_t node_idx, struct pathing_context *ctx);
typedef void (*pathing_evaluate_neighbors)(size_t node_idx, struct pathing_context *ctx);
typedef int (*pathing_end_condition)(size_t node_idx, struct pathing_context *ctx);

typedef struct pathing_context
{
    pathing_node *nodes;                        /**< Array of nodes in the pathfinding graph. */
    size_t num_nodes;                           /**< Total number of nodes in the graph. */
    heap *pq;                                   /**< Min-heap used for priority queue operations. */
    pathing_cost_eval_func cost_eval_func;      /**< Function for calculating movement cost. */
    pathing_evaluate_neighbors eval_nbors_func; /**< Function for evaluating neighboring nodes. (Must call pathing_eval_nodefor each neighbor you want to check) */
    pathing_end_condition end_cond_func;        /**< Function to check if the goal node has been reached. */
    void *data;
} pathing_context;

/**
 * @brief Function to solve the shortest path problem using a priority queue (Dijkstra's algorithm).
 *
 * @param start_idx Index of the starting node.
 * @param num_nodes Total number of nodes in the graph.
 * @param node_data Pointer to node data (optional).
 * @param node_data_elem_size Size of the data per node element.
 * @param ctx_data Pointer to user-defined context data (optional).
 * @param cost_eval Function to calculate node movement cost.
 * @param eval_neighbors Function to evaluate a node's neighbors. (Must call pathing_eval_node for each neighbor you want to check)
 * @param end_cond Function to check if the goal has been reached.
 * @return A dynamically allocated vector containing the indices of nodes forming the shortest path.
 */
vector *pathing_solve(
    size_t start_idx, size_t num_nodes,
    void *node_data, size_t node_data_elem_size,
    void *ctx_data,
    pathing_cost_eval_func cost_eval,
    pathing_evaluate_neighbors eval_neighbors,
    pathing_end_condition end_cond);
