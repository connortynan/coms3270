#pragma once

#include <vector>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

template <typename T>
class Grid
{
public:
    Grid(std::size_t width, std::size_t height, const T &default_value = T())
        : width_(width), height_(height), data_(width * height, default_value) {}

    Grid(std::size_t width, std::size_t height, std::initializer_list<T> values)
        : width_(width), height_(height), data_(values)
    {
        if (values.size() != width * height)
            throw std::invalid_argument("Initializer list size does not match grid dimensions");
    }

    bool in_bounds(std::size_t x, std::size_t y) const { return (x < width) && (y < height); }

    T &at(std::size_t x, std::size_t y)
    {
#ifdef GRID_EXTRA_CHECKING
        if (!in_bounds(x, y))
            throw std::out_of_range("Grid::at() - index out of bounds");
#endif
        return data_[y * width_ + x];
    }

    const T &at(std::size_t x, std::size_t y) const
    {
#ifdef GRID_EXTRA_CHECKING
        if (!in_bounds(x, y))
            throw std::out_of_range("Grid::at() const - index out of bounds");
#endif
        return data_[y * width_ + x];
    }

    T &operator()(std::size_t x, std::size_t y) { return at(x, y); }
    const T &operator()(std::size_t x, std::size_t y) const { return at(x, y); }

    void swap(std::size_t x1, std::size_t y1, std::size_t x2, std::size_t y2)
    {
#ifdef GRID_EXTRA_CHECKING
        if (!in_bounds(x1, y1) || !in_bounds(x2, y2))
            throw std::out_of_range("Grid::swap_items() - index out of bounds");
#endif
        std::swap(at(x1, y1), at(x2, y2));
    }

    void fill(T val) { std::fill(data_.begin(), data_.end(), val); }

    T *data() { return data_.data(); }
    const T *data() const { return data_.data(); }

    std::size_t width() const { return width_; }
    std::size_t height() const { return height_; }

private:
    std::size_t width_, height_;
    std::vector<T> data_;
};
