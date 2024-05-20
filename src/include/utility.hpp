#ifndef __UTILITY_HPP
#define __UTILITY_HPP

#include <utility>
#include <iostream>

namespace sjtu {

using std::swap;


/**
 * @brief Compares two values and returns -1 if the first value is less than the second,
 * 0 if they are equal, and 1 if the first value is greater than the second.
 *
 * @tparam Tp The type of the values to compare.
 * @param x The first value.
 * @param y The second value.
 * @return int -1 if x < y, 0 if x == y, 1 if x > y.
 */
template <class Tp>
int Camp(const Tp &x, const Tp &y) {
    return x < y ? -1 : (x == y ? 0 : 1);
}

/**
 * @brief A class template that represents a pair of values.
 *
 * @tparam T1 The type of the first value.
 * @tparam T2 The type of the second value.
 */
template<class T1, class T2>
class pair {
  public:
    T1 first; /**< The first value of the pair. */
    T2 second; /**< The second value of the pair. */

    /**
     * @brief Default constructor.
     */
    constexpr pair() = default;

    /**
     * @brief Copy constructor.
     *
     * @param other The pair to copy from.
     */
    constexpr pair(const pair &other) = default;

    /**
     * @brief Move constructor.
     *
     * @param other The pair to move from.
     */
    constexpr pair(pair &&other) = default;

    /**
     * @brief Constructs a pair from two values.
     *
     * @tparam U1 The type of the first value.
     * @tparam U2 The type of the second value.
     * @param x The first value.
     * @param y The second value.
     */
    template<class U1 = T1, class U2 = T2>
    constexpr pair(U1 && x, U2 && y)
        : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

    /**
     * @brief Constructs a pair from another pair with different types.
     *
     * @tparam U1 The type of the first value of the other pair.
     * @tparam U2 The type of the second value of the other pair.
     * @param other The other pair to copy from.
     */
    template<class U1, class U2>
    constexpr pair(const pair<U1, U2> &other)
        : first(other.first), second(other.second) {}

    /**
     * @brief Constructs a pair from another pair with different types using move semantics.
     *
     * @tparam U1 The type of the first value of the other pair.
     * @tparam U2 The type of the second value of the other pair.
     * @param other The other pair to move from.
     */
    template<class U1, class U2>
    constexpr pair(pair<U1, U2> &&other)
        : first(std::move(other.first))
        , second(std::move(other.second)) {}

    /**
     * @brief Copy assignment operator.
     *
     * @param other The pair to copy from.
     * @return pair& Reference to the pair after assignment.
     */
    pair &operator=(const pair &other) = default;

    /**
     * @brief Move assignment operator.
     *
     * @param other The pair to move from.
     * @return pair& Reference to the pair after assignment.
     */
    pair &operator=(pair &&other) = default;

    /**
     * @brief Equality comparison operator.
     *
     * @param rhs The pair to compare with.
     * @return bool True if the pairs are equal, false otherwise.
     */
    bool operator==(const pair &rhs) const {
        return first == rhs.first && second == rhs.second;
    }

    /**
     * @brief Inequality comparison operator.
     *
     * @param rhs The pair to compare with.
     * @return bool True if the pairs are not equal, false otherwise.
     */
    bool operator!=(const pair &rhs) const {
        return first != rhs.first || second != rhs.second;
    }

    /**
     * @brief Less than comparison operator.
     *
     * @param rhs The pair to compare with.
     * @return bool True if this pair is less than the rhs pair, false otherwise.
     */
    bool operator<(const pair &rhs) const {
        return first < rhs.first || (first == rhs.first && second < rhs.second);
    }
};

/**
 * @brief Deduction guide for pair class template.
 *
 * @tparam T1 The type of the first value.
 * @tparam T2 The type of the second value.
 * @param T1 The first value.
 * @param T2 The second value.
 * @return pair<T1, T2> The deduced pair type.
 */
template<class T1, class T2>
pair(T1, T2) -> pair<T1, T2>;

/**
 * @brief Stream insertion operator for pair class template.
 *
 * @tparam Tp1 The type of the first value.
 * @tparam Tp2 The type of the second value.
 * @param os The output stream.
 * @param p The pair to insert into the stream.
 * @return std::ostream& Reference to the output stream after insertion.
 */
template<class Tp1, class Tp2>
std::ostream &operator<<(std::ostream &os, const pair<Tp1, Tp2> &p) {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

template <typename Tp>
inline void quickcopy(Tp *dest, const Tp *src, size_t n) {
    // memmove(dest, src, n * sizeof(Tp));
    if (dest < src) {
        for (int i = 0; i < n; ++i) {
            dest[i] = src[i];
        }
    } else {
        for (int i = (int)n - 1; i >= 0; --i) {
            dest[i] = src[i];
        }
    }
}

template <typename RandomAccessIterator, typename Compare>
void sort(RandomAccessIterator first, RandomAccessIterator last,
           Compare camp) {
    if (!(first < last)) return;
    RandomAccessIterator l = first, r = last - 1;
    auto mid = *(l + (r - l) / 2);
    while (!(r < l)) {  // l <= r <=> !(r < l)
        while (camp(*l, mid)) l++;
        while (camp(mid, *r)) r--;
        if (!(r < l)) {
            swap(*l, *r);
            l++, r--;
        }
    }
    sort(first, r + 1, camp);
    sort(l, last, camp);
}

template <typename RandomAccessIterator>
void sort(RandomAccessIterator first, RandomAccessIterator last) {
    if (!(first < last)) return;
    RandomAccessIterator l = first, r = last - 1;
    auto mid = *(l + (r - l) / 2);
    while (!(r < l)) {
        while (*l < mid) l++;
        while (mid < *r) r--;
        if (!(r < l)) {
            swap(*l, *r);
            
            l++, r--;
        }
    }
    sort(first, r + 1);
    sort(l, last);
}

template<typename ForwardIterator, typename Tp>
ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const Tp &val) {
    ForwardIterator l = first, r = last;
    while (l < r) {
        ForwardIterator mid = l + (r - l) / 2;
        if (*mid < val) l = mid + 1;
        else r = mid;
    }
    return l;
}
template<typename ForwardIterator, typename Tp>
ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const Tp &val) {
    ForwardIterator l = first, r = last;
    while (l < r) {
        ForwardIterator mid = l + (r - l) / 2;
        if (val < *mid) r = mid;
        else l = mid + 1;
    }
    return l;
}

template <typename RandomAccessIterator>
RandomAccessIterator unique(RandomAccessIterator first, RandomAccessIterator last) {
    if (first == last) return last;
    RandomAccessIterator l = first, r = first + 1;
    while (r != last) {
        if (*l != *r) *(++l) = *r;
        r++;
    }
    return ++l;
}

template <typename RandomAccessIterator>
void reverse(RandomAccessIterator first, RandomAccessIterator last) {
    if (first == last) return;
    --last;
    while (first < last) {
        swap(*first, *last);
        first++, last--;
    }

}



}

#endif // __UTILITY_HPP