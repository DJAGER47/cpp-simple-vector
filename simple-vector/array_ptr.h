#pragma once

#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr
{
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size)
    {
        raw_ptr_ = size ? new Type[size]{} : nullptr;
    }

    explicit ArrayPtr(Type *raw_ptr) noexcept
        : raw_ptr_(raw_ptr)
    {
    }

    ArrayPtr(const ArrayPtr &) = delete;

    ArrayPtr(ArrayPtr &&other)
        : raw_ptr_(other.Release())
    {
    }

    ~ArrayPtr()
    {
        delete[] raw_ptr_;
    }

    ArrayPtr &operator=(const ArrayPtr &) = delete;

    ArrayPtr &operator=(ArrayPtr &&other)
    {
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);

        return *this;
    }

    Type &operator[](size_t index) noexcept
    {
        return raw_ptr_[index];
    }

    const Type &operator[](size_t index) const noexcept
    {
        return raw_ptr_[index];
    }

    explicit operator bool() const
    {
        return raw_ptr_ ? true : false;
    }

    [[nodiscard]] Type *Release() noexcept
    {
        return std::exchange(raw_ptr_, nullptr);
    }

    Type *Get() const noexcept
    {
        return raw_ptr_;
    }

    void swap(ArrayPtr &other) noexcept
    {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type *raw_ptr_ = nullptr;
};