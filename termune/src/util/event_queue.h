#pragma once

#include <queue>
#include <vector>

template <typename queued_t>
class EventQueue
{
private:
    struct Event
    {
        queued_t data;
        tick_t target_tick;

        bool operator>(const Event &other) const
        {
            if (target_tick != other.target_tick)
                return target_tick > other.target_tick;
            return data > other.data;
        }
    };

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> event_queue_;
    tick_t current_tick_ = 0;

public:
    EventQueue() = default;

    void add(const queued_t &data, tick_t delay)
    {
        event_queue_.push({data, current_tick_ + delay});
    }

    void advance_tick(tick_t delta = 1)
    {
        current_tick_ += delta;
    }

    tick_t current_tick() const
    {
        return current_tick_;
    }

    std::vector<queued_t> resolve_past_events()
    {
        std::vector<queued_t> resolved;
        while (!event_queue_.empty() && event_queue_.top().target_tick <= current_tick_)
        {
            resolved.push_back(event_queue_.top().data);
            event_queue_.pop();
        }
        return resolved;
    }

    std::vector<queued_t> resolve_events_until(const queued_t &target)
    {
        std::vector<queued_t> resolved;

        while (!event_queue_.empty())
        {
            const auto &event = event_queue_.top();
            event_queue_.pop();
            if (event.target_tick > current_tick_)
                current_tick_ = event.target_tick;

            resolved.push_back(event.data);

            if (event.data == target)
                break;
        }

        return resolved;
    }
};
