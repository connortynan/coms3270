#pragma once

#include <memory>
#include <vector>
#include <limits>

namespace Pathing
{
    class Node
    {
    public:
        virtual ~Node() = default;

        unsigned long cost = std::numeric_limits<unsigned long>::max(); /**< Cost to reach this node */
        std::size_t prev = SIZE_MAX;                                    /**< Index of previous node in path */

        /** Return a list of neighbor indices for this node. */
        virtual std::vector<std::size_t> get_neighbors() const = 0;

        /** Return the cost to move from this node to the given neighbor. */
        virtual unsigned long movement_cost_to(const Node &neighbor) const = 0;

        /** Return true if this node is the goal. */
        virtual bool is_goal() const = 0;
    };

    std::vector<size_t> solve(std::vector<std::unique_ptr<Node>> &nodes, size_t start_idx);
} // namespace Pathing
