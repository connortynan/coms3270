#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <memory>

namespace Pathing
{
    /**
     * Abstract base class for pathfinding nodes.
     * Users should inherit from this and implement the required virtual functions.
     */
    class Node
    {
    public:
        virtual ~Node() = default;

        uint64_t cost = std::numeric_limits<uint64_t>::max(); /**< Cost to reach this node */
        size_t prev = SIZE_MAX;                               /**< Index of previous node in path */

        /** Return a list of neighbor indices for this node. */
        virtual std::vector<size_t> get_neighbors() const = 0;

        /** Return the cost to move from this node to the given neighbor. */
        virtual uint64_t movement_cost_to(const Node &neighbor) const = 0;

        /** Return true if this node is the goal. */
        virtual bool is_goal() const = 0;
    };

    /**
     * Solves the shortest path from start node using Dijkstra's algorithm.
     * @param nodes Vector of Node instances (owned via unique_ptr).
     * @param start_idx Index of the starting node.
     * @return A list of node indices forming the shortest path.
     */
    std::vector<size_t> solve(std::vector<std::unique_ptr<Node>> &nodes, size_t start_idx)
    {
        using QueueEntry = std::pair<uint64_t, size_t>; // (cost, index)

        auto cmp = [](const QueueEntry &a, const QueueEntry &b)
        {
            return a.first > b.first; // min-heap
        };

        std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(cmp)> pq(cmp);

        nodes[start_idx]->cost = 0;
        pq.emplace(0, start_idx);

        while (!pq.empty())
        {
            auto [curr_cost, curr_idx] = pq.top();
            pq.pop();

            Node &current = *nodes[curr_idx];

            if (current.is_goal())
            {
                std::vector<size_t> path;
                for (size_t i = curr_idx; i != SIZE_MAX; i = nodes[i]->prev)
                    path.push_back(i);
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (size_t neighbor_idx : current.get_neighbors())
            {
                Node &neighbor = *nodes[neighbor_idx];
                uint64_t new_cost = current.cost + current.movement_cost_to(neighbor);

                if (new_cost < neighbor.cost)
                {
                    neighbor.cost = new_cost;
                    neighbor.prev = curr_idx;
                    pq.emplace(new_cost, neighbor_idx);
                }
            }
        }
        return {}; // No path found
    }
} // namespace Pathing
