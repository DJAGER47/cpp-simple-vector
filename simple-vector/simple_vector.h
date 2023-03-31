#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include "array_ptr.h"

class ReserveProxyObj
{
public:
    size_t new_capacity;
    ReserveProxyObj(size_t capacity_to_reserve)
        : new_capacity{capacity_to_reserve}
    {
    }
    size_t GetNewCapacity()
    {
        return new_capacity;
    }
};

template <typename Type>
class SimpleVector
{

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;

public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    // ������ ������ �� size ���������, ������������������ ��������� �� ���������
    explicit SimpleVector(size_t size)
    {
        SimpleVector new_vector(size, Type{});
        swap(new_vector);
    }

    // ������ ������ �� size ���������, ������������������ ��������� value
    SimpleVector(size_t size, const Type &value)
    {
        ArrayPtr<Type> new_vector(size);
        items_.swap(new_vector);
        capacity_ = size;
        size_ = capacity_;
        std::fill(begin(), end(), value);
    }

    // ������ ������ �� std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
    {
        SimpleVector<Type> new_vector(init.size());
        std::copy(init.begin(), init.end(), new_vector.begin());
        swap(new_vector);
    }

    // Конструктор копирования
    SimpleVector(const SimpleVector &other)
    {
        SimpleVector<Type> new_vector(other.GetSize());
        new_vector.capacity_ = other.capacity_;
        std::copy(other.begin(), other.end(), new_vector.begin());
        swap(new_vector);
    }

    SimpleVector(SimpleVector &&other)
    {
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    // создает вектор с необходимой емкостью
    SimpleVector(ReserveProxyObj r)
    {
        auto capacity_to_reserve = r.GetNewCapacity();
        ArrayPtr<Type> new_vector(capacity_to_reserve);
        items_.swap(new_vector);
        capacity_ = capacity_to_reserve;
        size_ = 0;
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (this != &rhs)
        {
            // using copy constructor
            auto rhs_copy(rhs);
            // swapping content
            swap(rhs_copy);
        }
        return *this;
    }

    // ���������� ���������� ��������� � �������
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // ���������� ����������� �������
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // ��������, ������ �� ������
    bool IsEmpty() const noexcept
    {
        return (size_ == 0);
    }

    // ���������� ������ �� ������� � �������� index
    Type &operator[](size_t index) noexcept
    {
        return *(items_.Get() + index);
    }

    // ���������� ����������� ������ �� ������� � �������� index
    const Type &operator[](size_t index) const noexcept
    {
        return *(items_.Get() + index);
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    Type &At(size_t index)
    {
        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // ���������� ����������� ������ �� ������� � �������� index
    // ����������� ���������� std::out_of_range, ���� index >= size
    const Type &At(size_t index) const
    {
        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // �������� ������ �������, �� ������� ��� �����������
    void Clear() noexcept
    {
        Resize(0);
    }

    // �������� ������ �������.
    // ��� ���������� ������� ����� �������� �������� �������� �� ��������� ��� ���� Type
    void Resize(size_t new_size)
    {
        if (new_size < size_)
        {
            size_ = new_size;
        }
        else if (new_size < capacity_)
        {
            std::for_each(items_.Get() + size_, items_.Get() + new_size, [](auto &x)
                          { x = Type{}; });
            size_ = new_size;
        }
        else
        {
            ArrayPtr<Type> new_vector(new_size);
            std::move(items_.Get(), items_.Get() + size_, new_vector.Get());
            std::for_each(new_vector.Get() + size_, new_vector.Get() + new_size, [](auto &x)
                          { x = Type{}; });
            items_.swap(new_vector);
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    void Reserve(size_t new_capacity)
    {
        if (capacity_ < new_capacity)
        {
            SimpleVector new_vector(new_capacity);
            new_vector.size_ = size_;
            std::copy(begin(), end(), new_vector.begin());
            swap(new_vector);
        }
    }

    void PushBack(const Type &item)
    {
        Resize(size_ + 1);
        (*this)[size_ - 1] = item;
    }

    void PushBack(Type &&item)
    {
        if (size_ != capacity_)
        {
            items_[size_] = std::move(item);
            ++size_;
        }
        else
        {
            auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, new_vector.Get());
            new_vector[size_] = std::move(item);
            items_.swap(new_vector);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    Iterator Insert(ConstIterator position, const Type &value)
    {
        assert(position >= begin() && position < end());
        if (size_ != capacity_)
        {
            std::copy_backward(const_cast<Iterator>(position), end(), end() + 1);
            items_[position - items_.Get()] = value;
            ++size_;
            return const_cast<Iterator>(position);
        }
        else
        {
            auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            const auto insert_pos = position - items_.Get();
            ArrayPtr<Type> new_vector(new_capacity);
            std::copy(begin(), const_cast<Iterator>(position), new_vector.Get());
            new_vector[insert_pos] = value;
            std::copy(const_cast<Iterator>(position), end(), new_vector.Get() + insert_pos + 1);
            items_.swap(new_vector);
            ++size_;
            capacity_ = new_capacity;
            return items_.Get() + insert_pos;
        }
    }
    Iterator Insert(ConstIterator position, Type &&value)
    {
        if (size_ != capacity_)
        {
            std::move_backward(const_cast<Iterator>(position), end(), end() + 1);
            items_[position - items_.Get()] = std::move(value);
            ++size_;
            return const_cast<Iterator>(position);
        }
        else
        {
            auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            const auto insert_pos = position - items_.Get();
            ArrayPtr<Type> new_vector(new_capacity);
            std::move(begin(), const_cast<Iterator>(position), new_vector.Get());
            new_vector[insert_pos] = std::move(value);
            std::move(const_cast<Iterator>(position), end(), new_vector.Get() + insert_pos + 1);
            items_.swap(new_vector);
            ++size_;
            capacity_ = new_capacity;
            return items_.Get() + insert_pos;
        }
    }

    void PopBack() noexcept
    {
        Resize(size_ - 1);
    }

    Iterator Erase(ConstIterator position)
    {
        assert(position >= begin() && position < end());
        std::move(const_cast<Iterator>(position) + 1, end(), const_cast<Iterator>(position));
        --size_;
        return const_cast<Iterator>(position);
    }

    void swap(SimpleVector &other) noexcept
    {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // ���������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator begin() noexcept
    {
        return items_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    Iterator end() noexcept
    {
        return items_.Get() + size_;
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator begin() const noexcept
    {
        return items_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator end() const noexcept
    {
        return items_.Get() + size_;
    }

    // ���������� ����������� �������� �� ������ �������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cbegin() const noexcept
    {
        return items_.Get();
    }

    // ���������� �������� �� �������, ��������� �� ���������
    // ��� ������� ������� ����� ���� ����� (��� �� �����) nullptr
    ConstIterator cend() const noexcept
    {
        return items_.Get() + size_;
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    ReserveProxyObj r(capacity_to_reserve);
    return r;
}

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs) && !(rhs < lhs);
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return lhs == rhs || lhs < rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs);
}
