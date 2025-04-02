#include "util/pathing.h"

#include <algorithm>
#include <queue>

namespace Pathing
{
    std::vector<std::size_t> solve(std::vector<std::unique_ptr<Node>> &nodes, std::size_t start_idx)
    {
        using QueueEntry = std::pair<unsigned long, std::size_t>; // (cost, index)

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
                std::vector<std::size_t> path;
                for (std::size_t i = curr_idx; i != SIZE_MAX; i = nodes[i]->prev)
                    path.push_back(i);
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (std::size_t neighbor_idx : current.get_neighbors())
            {
                Node &neighbor = *nodes[neighbor_idx];
                unsigned long new_cost = current.cost + current.movement_cost_to(neighbor);

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
