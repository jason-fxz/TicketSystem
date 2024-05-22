/**
 * @file Vector.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief implement a container like std::vector
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <memory>

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
T max(const T &x, const T &y) {
    return x > y ? x : y;
}

template<typename T>
class vector {
  private:
    static constexpr double MULTIPLIER = 2; ///< The multiplier for capacity growth.
    T *_m_data; ///< The pointer to the data array.
    size_t _m_size; ///< The number of elements in the vector.
    size_t _m_cap;  ///< The capacity of the vector.
    std::allocator<T> _alloc; ///< The allocator for memory management.

  public:

    class const_iterator;
    /**
     * @brief iterator of vector
     * this iterator is random access iterator
     */
    class iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::output_iterator_tag;
        friend class vector;

      private:
        T *_m_ptr;
        const vector *const _vec;
        iterator(T *_ptr, const vector *__vec) : _m_ptr(_ptr), _vec(__vec) {}

      public:
        /**
         * return a new iterator which pointer n-next elements
         * as well as operator-
         */
        iterator operator+(const int &n) const {
            return iterator(_m_ptr + n, _vec);
        }
        iterator operator-(const int &n) const {
            return iterator(_m_ptr - n, _vec);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const iterator &rhs) const {
            if (_vec != rhs._vec) throw invalid_iterator();
            return _m_ptr - rhs._m_ptr;
        }
        iterator &operator+=(const int &n) {
            _m_ptr += n;
            return *this;
        }
        iterator &operator-=(const int &n) {
            _m_ptr -= n;
            return *this;
        }

        iterator operator++(int) {
            auto tmp = *this;
            ++_m_ptr;
            return tmp;
        }

        iterator &operator++() {
            ++_m_ptr;
            return *this;
        }

        iterator operator--(int) {
            auto tmp = *this;
            --_m_ptr;
            return tmp;
        }

        iterator &operator--() {
            --_m_ptr;
            return *this;
        }

        T &operator*() const {
            return *_m_ptr;
        }
        T *operator->() const {
            return _m_ptr;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory address).
         */
        bool operator==(const iterator &rhs) const {
            return _m_ptr == rhs._m_ptr;
        }
        bool operator==(const const_iterator &rhs) const {
            return _m_ptr == rhs._m_cptr;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return _m_ptr != rhs._m_ptr;
        }
        bool operator!=(const const_iterator &rhs) const {
            return _m_ptr != rhs._m_cptr;
        }
        bool operator<(const iterator &rhs) const {
            return _m_ptr < rhs._m_ptr;
        }
        bool operator<(const const_iterator &rhs) const {
            return _m_ptr < rhs._m_ptr;
        }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     */
    class const_iterator {
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::output_iterator_tag;

      private:
        /*TODO*/
        const T *_m_cptr;
        const vector *const _vec;
        const_iterator(const T *_ptr, const vector *__vec) : _m_cptr(_ptr), _vec(__vec) {}
        const_iterator(const iterator &other) : _m_cptr(other._m_ptr), _vec(other._vec) {}
        friend class vector;

      public:
        const_iterator operator+(const int &n) const {
            return const_iterator(_m_cptr + n, _vec);
        }
        const_iterator operator-(const int &n) const {
            return const_iterator(_m_cptr - n, _vec);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw invaild_iterator.
        int operator-(const const_iterator &rhs) const {
            if (_vec != rhs._vec) throw invalid_iterator();
            return _m_cptr - rhs._m_cptr;
        }
        const_iterator &operator+=(const int &n) {
            _m_cptr += n;
            return *this;
        }
        const_iterator &operator-=(const int &n) {
            _m_cptr -= n;
            return *this;
        }

        const_iterator operator++(int) {
            auto tmp = *this;
            ++_m_cptr;
            return tmp;
        }

        const_iterator &operator++() {
            ++_m_cptr;
            return *this;
        }

        const_iterator operator--(int) {
            auto tmp = *this;
            --_m_cptr;
            return tmp;
        }

        const_iterator &operator--() {
            --_m_cptr;
            return *this;
        }

        const T &operator*() const {
            return *_m_cptr;
        }
        const T *operator->() const {
            return _m_cptr;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory address).
         */
        bool operator==(const iterator &rhs) const {
            return _m_cptr == rhs._m_ptr;
        }
        bool operator==(const const_iterator &rhs) const {
            return _m_cptr == rhs._m_cptr;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return _m_cptr != rhs._m_ptr;
        }
        bool operator!=(const const_iterator &rhs) const {
            return _m_cptr != rhs._m_cptr;
        }
        bool operator<(const iterator &rhs) const {
            return _m_cptr < rhs._m_ptr;
        }
        bool operator<(const const_iterator &rhs) const {
            return _m_cptr < rhs._m_ptr;
        }

    };
    /**
     * @brief Construct a new vector object
     *
     */
    vector() : _m_data(nullptr), _m_cap(0), _m_size(0) {}
    /**
     * @brief Construct a new vector object by copying another vector
     *
     * @param other
     */
    vector(const vector &other) {
        _m_data = _alloc.allocate(other._m_cap);
        _m_size = other._m_size;
        _m_cap = other._m_cap;
        for (int i = 0; i < _m_size; ++i) {
            std::construct_at(_m_data + i, std::as_const(other._m_data[i]));
        }
    }

    /**
     * @brief Construct a new vector object by moving another vector
     *
     * @param other
     */
    vector(vector &&other) {
        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_cap = other._m_cap;
        other._m_data = nullptr;
        other._m_size = 0;
        other._m_cap = 0;
    }
    /**
     * @brief Destroy the vector object
     *
     */
    ~vector() {
        cclear();
    }
    /**
     * @brief assign operator
     * clear current vector and assign new vector by copying other
     *
     * @param other
     * @return vector&
     */
    vector &operator=(const vector &other) {
        if (this == &other) return *this;
        cclear();
        _m_data = _alloc.allocate(other._m_cap);
        _m_size = other._m_size;
        _m_cap = other._m_cap;
        for (int i = 0; i < _m_size; ++i) {
            std::construct_at(_m_data + i, std::as_const(other._m_data[i]));
        }
        return *this;
    }
    /**
     * @brief assign operator
     * clear current vector and assign new vector by moving other
     *
     * @param other
     * @return vector&
     */
    vector &operator=(vector &&other) {
        if (this == &other) return *this;
        cclear();
        _m_data = other._m_data;
        _m_size = other._m_size;
        _m_cap = other._m_cap;
        other._m_data = nullptr;
        other._m_size = 0;
        other._m_cap = 0;
        return *this;
    }

    /**
     * @brief assigns specified element with bounds checking
     * It will check whether pos is in [0, size) and throw index_out_of_bound if not.
     * @param pos the position of the element to be assigned
     * @throw index_out_of_bound if pos is not in [0, size)
     * @return T&
     */
    T &at(const size_t &pos) {
        if (pos < 0 || pos >= _m_size) throw index_out_of_bound();
        return _m_data[pos];
    }
    /**
     * @brief assigns specified element with bounds checking (const version)
     * It will check whether pos is in [0, size) and throw index_out_of_bound if not.
     * @param pos the position of the element to be assigned
     * @throw index_out_of_bound if pos is not in [0, size)
     * @return const T&
     */
    const T &at(const size_t &pos) const {
        if (pos < 0 || pos >= _m_size) throw index_out_of_bound();
        return _m_data[pos];
    }
    /**
     * @brief access specified element without bounds checking
     * It will not check whether pos is in [0, size), so it may cause segmentation fault.
     * @param pos
     * @return T&
     */
    T &operator[](const size_t &pos) {
        return at(pos);
    }
    /**
     * @brief access specified element without bounds checking (const version)
     * It will not check whether pos is in [0, size), so it may cause segmentation fault.
     * @param pos
     * @return const T&
     */
    const T &operator[](const size_t &pos) const {
        return at(pos);
    }
    /**
     * @brief access the first element.
     *
     * @throw container_is_empty if size == 0
     * @return const T& the first element
     */
    const T &front() const {
        if (_m_size == 0) throw container_is_empty();
        return _m_data[0];
    }
    /**
     * @brief access the last element.
     *
     * @throw container_is_empty if size == 0
     * @return const T& the last element
     */
    const T &back() const {
        if (_m_size == 0) throw container_is_empty();
        return _m_data[_m_size - 1];
    }
    /**
     * @brief returns an iterator to the beginning.
     *
     * @return iterator
     */
    iterator begin() {
        return iterator(_m_data, this);
    }
    /**
     * @brief returns an const_iterator to the beginning.
     *
     * @return const_iterator
     */
    const_iterator begin() const {
        return const_iterator(_m_data, this);
    }
    /**
     * @brief returns an const_iterator to the beginning.
     *
     * @return const_iterator
     */
    const_iterator cbegin() const {
        return const_iterator(_m_data, this);
    }
    /**
     * @brief returns an iterator to the end.
     *
     * @return iterator
     */
    iterator end() {
        return iterator(_m_data + _m_size, this);
    }
    /**
     * @brief returns an const_iterator to the end.
     *
     * @return const_iterator
     */
    const_iterator end() const {
        return const_iterator(_m_data + _m_size, this);
    }
    /**
     * @brief returns an const_iterator to the end.
     *
     * @return const_iterator
     */
    const_iterator cend() const {
        return const_iterator(_m_data + _m_size, this);
    }
    /**
     * checks whether the container is empty
     */
    bool empty() const {
        return _m_size == 0;
    }
    /**
     * returns the number of elements
     */
    size_t size() const {
        return _m_size;
    }
    /**
     * returns the number of elements that can be held in currently allocated storage.
     */
    size_t capacity() const {
        return _m_cap;
    }
    /**
     * clears the contents and capacity of the vector.
     */
    void cclear() {
        if (_m_data == nullptr) return;
        for (int i = 0; i < _m_size; ++i) {
            std::destroy_at(_m_data + i);
        }
        _alloc.deallocate(_m_data, _m_cap);
        _m_data = nullptr;
        _m_size = 0;
        _m_cap = 0;
    }
    /**
     *  Just clear the contents of the vector.
     */
    void clear() {
        if (_m_data == nullptr) return;
        for (int i = 0; i < _m_size; ++i) {
            std::destroy_at(_m_data + i);
        }
        _m_size = 0;
    }
    /**
     * @brief inserts value before pos
     * It will grow the capacity if the size is larger than the capacity, so it may cause memory reallocation.
     * It means that the previous iterators are invalid.
     * @param pos position to insert
     * @param value to insert
     * @returns iterator pointing to the inserted value.
     */
    iterator insert(iterator pos, const T &value) {
        return insert(pos - begin(), value);
    }
    /**
     * @brief inserts value at index
     * It will grow the capacity if the size is larger than the capacity, so it may cause memory reallocation.
     * It means that the previous iterators are invalid.
     * @throw index_out_of_bound if ind > size
     * @param ind the index to insert
     * @param value to insert
     * @return iterator pointing to the inserted value.
     */
    iterator insert(const size_t &ind, const T &value) {
        if (ind > _m_size) throw index_out_of_bound();
        _grow_capacity_until(_m_size + 1);
        for (size_t i = _m_size; i > ind; --i) {
            if (i == _m_size) {
                std::construct_at(_m_data + i, std::move_if_noexcept(_m_data[i - 1]));
            } else {
                _m_data[i] = std::move_if_noexcept(_m_data[i - 1]);
            }
        }
        if (ind == _m_size) {
            std::construct_at(_m_data + ind, value);
        } else {
            _m_data[ind] = value;
        }
        ++_m_size;
        return iterator(_m_data + ind, this);
    }
    /**
     * @brief removes the element at pos.
     * remove the element at pos and return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     * @param pos the position to remove
     * @return iterator pointing to the following element.
     */
    iterator erase(iterator pos) {
        return erase(pos - begin());
    }
    /**
     * @brief removes the element at ind.
     * remove the element at ind and return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is returned.
     * @throw index_out_of_bound if ind >= size
     * @param ind the index to remove
     * @return iterator pointing to the following element.
     */
    iterator erase(const size_t &ind) {
        if (ind >= _m_size) throw index_out_of_bound();
        --_m_size;
        for (size_t i = ind; i < _m_size; ++i) {
            _m_data[i] = std::move_if_noexcept(_m_data[i + 1]);
        }
        std::destroy_at(_m_data + _m_size);
        // _shrink_capacity();
        return iterator(_m_data + ind, this);
    }
    /**
     * adds an element to the end.
     */
    void push_back(const T &value) {
        _grow_capacity_until(_m_size + 1);
        std::construct_at(_m_data + _m_size, value);
        ++_m_size;
    }
    /**
     * remove the last element from the end.
     * @throw container_is_empty if size() == 0
     */
    void pop_back() {
        if (_m_size == 0) throw container_is_empty();
        std::destroy_at(_m_data + _m_size - 1);
        --_m_size;
    }

    /**
     * @brief Recapacity the container to contain n elements.
     * if n > capacity(), the capacity will be increased to n.
     */
    void reserve(size_t n) {
        _grow_capacity_until(n);
    }

    /**
     * @brief Resize the container to contain n elements.
     * if n > size(), the new elements will be initialized with value.
     * if n < size(), the container will be destroyed.
     */
    void resize(size_t n, const T &value) {
        if (n < _m_size) {
            for (size_t i = n; i < _m_size; ++i) {
                std::destroy_at(_m_data + i);
            }
            _m_size = n;
        } else if (n > _m_size) {
            _grow_capacity_until(n);
            for (size_t i = _m_size; i < n; ++i) {
                std::construct_at(_m_data + i, value);
            }
            _m_size = n;
        }
    }

    /**
     * @brief Resize the container to contain n elements.
     * if n > size(), the new elements will be default initialized.
     * if n < size(), the container will be destroyed.
     */
    void resize(size_t n) {
        if (n < _m_size) {
            for (size_t i = n; i < _m_size; ++i) {
                std::destroy_at(_m_data + i);
            }
            _m_size = n;
        } else if (n > _m_size) {
            _grow_capacity_until(n);
            for (size_t i = _m_size; i < n; ++i) {
                std::construct_at(_m_data + i, T());
            }
            _m_size = n;
        }
    }

    /**
     * Shrink the capacity to fit the size.
     */
    void shrink_to_fit() {
        _shrink_capacity_until(_m_size);
    }

    /**
     * @brief Get the data pointer. 
     */
    T *data() {
        return _m_data;
    }
    /**
     * @brief Get the data pointer. 
     */
    const T *data() const {
        return _m_data;
    }

  private:
    void _grow_capacity_until(size_t n) {
        if (n <= _m_cap) return;
        n = max(n, size_t(_m_cap * MULTIPLIER));
        auto old_data = _m_data;
        _m_data = _alloc.allocate(n);
        if (old_data) {
            for (size_t i = 0; i < _m_size; ++i) {
                std::construct_at(_m_data + i, std::move_if_noexcept(old_data[i]));
            }
            for (size_t i = 0; i < _m_size; ++i) {
                std::destroy_at(old_data + i);
            }
            _alloc.deallocate(old_data, _m_cap);
        }
        _m_cap = n;
    }
    void _shrink_capacity_until(size_t n) {
        if (n <= _m_size) return;
        auto old_data = _m_data;
        _m_data = _alloc.allocate(n);
        if (old_data) {
            for (size_t i = 0; i < _m_size; ++i) {
                std::construct_at(_m_data + i, std::move_if_noexcept(old_data[i]));
            }
            for (size_t i = 0; i < _m_size; ++i) {
                std::destroy_at(old_data + i);
            }
            _alloc.deallocate(old_data, _m_cap);
        }
        _m_cap = n;
    }

};


} // namespace sjtu

#endif // SJTU_VECTOR_HPP

