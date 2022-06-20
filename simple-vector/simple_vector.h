#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "array_ptr.h"

struct ReserveProxyObj {
    explicit ReserveProxyObj(size_t capacity_to_reserve): capacity_to_reserve_(capacity_to_reserve){}
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size): 
        array_(ArrayPtr<Type>(size))
        , size_(size)
        , capacity_(size) {
        std::fill(begin(), end(), Type());
    }

    explicit SimpleVector(ReserveProxyObj r):
        array_(ArrayPtr<Type>(r.capacity_to_reserve_))
        , size_(0)
        , capacity_(r.capacity_to_reserve_){}

    SimpleVector(size_t size, const Type& value):
        array_(ArrayPtr<Type>(size))
        , size_(size)
        , capacity_(size) {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init):
        array_(ArrayPtr<Type>(init.size()))
        , size_(init.size())
        , capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other):
        array_(ArrayPtr<Type>(other.GetCapacity()))
        , size_(other.GetSize())
        , capacity_(other.GetCapacity()){
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other)noexcept:
        array_(std::move(other.array_))
        , size_(std::exchange(other.size_, 0))
        , capacity_(std::exchange(other.capacity_, 0)) {}

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }

    void PushBack(Type item) {
        if (size_ == capacity_) {
            size_t new_capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_array_(new_capacity_);
            std::move(begin(), end(), new_array_.Get());
            array_.swap(new_array_);
            capacity_ = new_capacity_;
        }
        array_[size_++] = std::forward<Type>(item);
    }

    Iterator Insert(ConstIterator pos, Type value) {
        size_t index_pos = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_array_(new_capacity_);
            std::move(begin(), begin() + index_pos, new_array_.Get());
            new_array_[index_pos] = std::move(value);
            std::move(begin() + index_pos, end(), new_array_.Get() + index_pos + 1);
            array_.swap(new_array_);
            capacity_ = new_capacity_;
            ++size_;
        }
        else {
            ++size_;
            std::move_backward(begin() + index_pos, begin() + size_ - 1, end());
            array_[index_pos] = std::move(value);
        }
        return begin() + index_pos;
    }

    void PopBack() noexcept {
        assert(size_ > 0);
        --size_;
    }

    Iterator Erase(ConstIterator pos) {
        size_t index_pos = pos - begin();
        std::move(begin() + index_pos + 1, end(), begin() + index_pos);
        --size_;
        return begin() + index_pos;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_array_(new_capacity);
            std::move(begin(), end(), new_array_.Get());
            array_.swap(new_array_);
            capacity_ = new_capacity;
        }
    }

    void swap(SimpleVector& other) noexcept {
        array_.swap(other.array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(size_ > 0 && index < size_);
        return array_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(size_ > 0 && index < size_);
        return array_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return array_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("");
        }
        return array_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            ArrayPtr<Type> new_array_(new_size);
            std::move(begin(), end(), new_array_.Get());
            std::for_each(new_array_.Get() + size_, new_array_.Get() + new_size, [](Type& item) { item = Type(); });
            array_.swap(new_array_);
            size_ = new_size;
            capacity_ = std::max(new_size, capacity_*2);
        }
        else if(new_size > size_){
            std::for_each(begin() + size_, begin() + new_size, [](Type& item) {item = Type(); });
            size_ = new_size;
        }
        else {
            size_ = new_size;
        }
    }

    Iterator begin() noexcept {
        return array_.Get();
    }

    Iterator end() noexcept {
        return begin() + size_;
    }

    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    ConstIterator end() const noexcept {
        return begin() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs < rhs || lhs == rhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs > rhs || lhs == rhs;
}