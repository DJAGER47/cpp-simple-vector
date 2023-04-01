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

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size), capacity_(size), items_(size)
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value)
        : size_(size), capacity_(size), items_(size)
    {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_(init.size()), capacity_(init.size()), items_(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector &other)
        : size_(other.size_), capacity_(other.capacity_), items_(other.size_)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector &&other)
        : size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)), items_(std::move(other.items_))
    {
    }

    SimpleVector(ReserveProxyObj r)
        : size_(0), capacity_(r.new_capacity), items_(size_)
    {
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (this != &rhs)
        {
            auto tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector &operator=(SimpleVector &&rhs)
    {
        if (this != &rhs)
        {
            auto tmp = std::move(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
        return *(items_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
        return *(items_.Get() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
        if (!(index < size_))
            throw std::out_of_range("Index must be less than vector size");
        return (*this)[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        if (new_size < size_)
        {
            size_ = new_size;
        }
        else if (new_size <= capacity_)
        {
            std::fill(end(), &items_[capacity_], Type());
            size_ = new_size;
        }
        else
        {
            Reserve(std::max(new_size, 2 * capacity_));
            std::fill(&items_[size_], &items_[new_size], Type());
            size_ = new_size;
        }
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity >= capacity_)
        {
            ArrayPtr<Type> new_items(new_capacity);
            std::move(begin(), end(), new_items.Get());
            items_.swap(new_items);
            capacity_ = new_capacity;
        }
    }

    void PushBack(const Type &item)
    {
        size_t number = 1;
        if (size_ < capacity_)
        {
            items_[size_] = item;
        }
        else
        {
            Reserve(std::max(number, 2 * capacity_));
            items_[size_] = item;
        }
        ++size_;
    }

    void PushBack(Type &&item)
    {
        size_t number = 1;
        if (size_ < capacity_)
        {
            items_[size_] = std::move(item);
        }
        else
        {
            Reserve(std::max(number, 2 * capacity_));
            items_[size_] = std::move(item);
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type &value)
    {
        assert(begin() <= pos && pos <= end());

        size_t n = std::distance(begin(), Iterator(pos));
        if (capacity_ == 0)
        {
            PushBack(value);
            return begin();
        }
        if (size_ < capacity_)
        {
            for (size_t i = size_; i > n; --i)
            {
                items_[i] = std::move(items_[i - 1]);
            }
            items_[n] = value;
            ++size_;
            return begin() + n;
        }
        else
        {
            SimpleVector<Type> new_vec(capacity_ * 2);

            std::copy(begin(), begin() + n, new_vec.begin());
            std::copy(begin() + n, end(), new_vec.begin() + n + 1);
            *(new_vec.begin() + n) = value;
            auto old_size = size_;
            swap(new_vec);
            size_ = old_size + 1;
            return begin() + n;
        }
    }

    Iterator Insert(ConstIterator pos, Type &&value)
    {
        assert(begin() <= pos && pos <= end());

        size_t n = std::distance(begin(), Iterator(pos));
        if (capacity_ == 0)
        {
            PushBack(std::move(value));
            return begin();
        }
        if (size_ < capacity_)
        {
            for (size_t i = size_; i > n; --i)
            {
                items_[i] = std::move(items_[i - 1]);
            }
            items_[n] = std::move(value);

            ++size_;
            return begin() + n;
        }
        else
        {
            SimpleVector<Type> new_vec(capacity_ * 2);

            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + n), new_vec.begin());
            std::copy(std::make_move_iterator(begin() + n), std::make_move_iterator(end()), new_vec.begin() + n + 1);
            *(new_vec.begin() + n) = std::move(value);
            auto old_size = size_;
            swap(new_vec);
            size_ = old_size + 1;
            return begin() + n;
        }
    }

    void PopBack() noexcept
    {
        assert(!IsEmpty());
        --size_;
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

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
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
    return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
    return !(lhs < rhs);
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
