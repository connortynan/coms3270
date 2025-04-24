#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <algorithm>

class EventQueue
{
public:
    // Function signature for the callback, which returns true if the event should halt the queue
    using Callback = std::function<bool()>;

    void add(Callback cb, tick_t delay);
    void process();
    bool process_one();
    void flush();
    tick_t current_tick() const;

private:
    struct Event
    {
        Callback callback;
        tick_t target_tick;
        bool operator>(const Event &other) const
        {
            return target_tick > other.target_tick;
        }
    };

    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> queue_;
    tick_t current_tick_ = 0;
};

inline void EventQueue::add(Callback cb, tick_t delay)
{
    queue_.push({std::move(cb), current_tick_ + delay});
}

inline void EventQueue::process()
{
    while (!queue_.empty() && !process_one())
        ;
}

inline bool EventQueue::process_one()
{
    if (!queue_.empty())
    {
        auto event = queue_.top();
        queue_.pop();

        current_tick_ = std::max(current_tick_, event.target_tick);

        return event.callback();
    }
    return true;
}

inline void EventQueue::flush()
{
    while (!queue_.empty())
        queue_.pop();
}

inline tick_t EventQueue::current_tick() const
{
    return current_tick_;
}
