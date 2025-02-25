#include "pathing.h"
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "util/heap.h"
#include "util/vector.h"

static int pathing_node_compare(const void *a, const void *b, void *context)
{
    pathing_context *p_ctx = (pathing_context *)context;

    size_t idx_a = *((const size_t *)a); // Index of node A
    size_t idx_b = *((const size_t *)b); // Index of node B
    const pathing_node *node_a = &p_ctx->nodes[idx_a];
    const pathing_node *node_b = &p_ctx->nodes[idx_b];

    return (node_a->cost > node_b->cost) - (node_a->cost < node_b->cost);
}

void pathing_eval_node(
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

vector *pathing_solve(
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

    while (heap_size(ctx.pq) > 0)
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
