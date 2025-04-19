#pragma once

#include <iterator>
#include <functional>

template <typename T, typename BaseIter>
class FilteredIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T *;
    using reference = T *&;
    using pointer = T **;

    FilteredIterator(BaseIter current, BaseIter end,
                     std::function<bool(const T &)> predicate)
        : current_(current), end_(end), pred_(std::move(predicate))
    {
        advance();
    }

    T *operator*() const
    {
        return dynamic_cast<T *>(current_->get());
    }

    FilteredIterator &operator++()
    {
        ++current_;
        advance();
        return *this;
    }

    bool operator!=(const FilteredIterator &other) const
    {
        return current_ != other.current_;
    }

private:
    void advance()
    {
        while (current_ != end_)
        {
            if (auto *ptr = dynamic_cast<T *>(current_->get()))
            {
                if (pred_(*ptr))
                    return;
            }
            ++current_;
        }
    }

    BaseIter current_, end_;
    std::function<bool(const T &)> pred_;
};

template <typename T, typename Container>
class FilteredView
{
public:
    using BaseIter = typename Container::const_iterator;

    FilteredView(const Container &container, std::function<bool(const T &)> predicate = [](const T &)
                                             { return true; })
        : begin_it_(container.begin(), container.end(), predicate), end_it_(container.end(), container.end(), predicate) {}

    auto begin() const { return begin_it_; }
    auto end() const { return end_it_; }

private:
    FilteredIterator<T, BaseIter> begin_it_;
    FilteredIterator<T, BaseIter> end_it_;
};
