/**
 * @file Hashmap.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief Hashmap class
 * @version 0.1
 * @date 2024-05-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"
#include "utility.hpp"

namespace sjtu {


/**
 * @brief A hash map implementation using separate chaining collision resolution.
 *
 * @tparam Key The key type.
 * @tparam Tp The value type.
 * @tparam MOD The size of the hash head table.
 * @tparam Hash The hash function type.
 */
template <class Key, class Tp, size_t MOD, class Hash = std::hash<Key>>
class Hashmap {
  private:
    typedef std::pair<const Key, Tp> data_t;
    /**
     * @brief Node structure for storing key-value pairs.
     */
    struct node {
        data_t data;       ///< The key-value pair.
        node  *next;       ///< Pointer to the next node in the chain.
    };
    node *m_data[MOD];     ///< Array of pointers to the head of each chain.
    size_t m_size;         ///< The number of key-value pairs in the hash map.

  public:
    /**
     * @brief Constructs an empty hash map.
     */
    Hashmap() {
        m_size = 0;
        for (size_t i = 0; i < MOD; ++i) {
            m_data[i] = nullptr;
        }
    }

    /**
     * @brief Destroys the hash map and frees the memory.
     */
    ~Hashmap() {
        clear();
    }

    /**
     * @brief Removes all elements from the hash map.
     */
    void clear() {
        m_size = 0;
        for (size_t i = 0; i < MOD; ++i) {
            while (m_data[i]) {
                node *tmp = m_data[i];
                m_data[i] = m_data[i]->next;
                delete tmp;
            }
        }
    }

    /**
     * @brief Accesses the value associated with the given key.
     * If the key does not exist, a new key-value pair is created.
     *
     * @param key The key to access.
     * @return Tp& Reference to the value associated with the key.
     */
    Tp &operator[](const Key &key) {
        size_t h = Hash()(key) % MOD;
        for (node *p = m_data[h]; p; p = p->next) {
            if (p->data.first == key) {
                return p->data.second;
            }
        }
        node *tmp = new node{{key, Tp()}, m_data[h]};
        m_data[h] = tmp;
        ++m_size;
        return tmp->data.second;
    }

    /**
     * @brief Accesses the value associated with the given key.
     * If the key does not exist, throws an exception.
     *
     * @throw index_out_of_bound If the key does not exist.
     * @param key The key to access.
     * @return Tp& Reference to the value associated with the key.
     */
    Tp &at(const Key &key) {
        size_t h = Hash()(key) % MOD;
        for (node *p = m_data[h]; p; p = p->next) {
            if (p->data.first == key) {
                return p->value;
            }
        }
        throw sjtu::index_out_of_bound();
    }

    /**
     * @brief Removes the key-value pair with the given key from the hash map.
     *
     * @param key The key to erase.
     * @return true if the key-value pair was found and erased, false otherwise.
     */
    bool erase(const Key &key) {
        size_t h = Hash()(key) % MOD;
        node *p = m_data[h], *last = nullptr;
        for (; p; last = p, p = p->next) {
            if (p->data.first == key) {
                if (last) {
                    last->next = p->next;
                } else {
                    m_data[h] = p->next;
                }
                delete p;
                --m_size;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Returns the number of key-value pairs with the given key.
     * since the key is unique, the return value is either 0 or 1.
     * @param key The key to count.
     * @return size_t The number of key-value pairs with the given key.
     */
    size_t count(const Key &key) {
        size_t h = Hash()(key) % MOD;
        for (node *p = m_data[h]; p; p = p->next) {
            if (p->data.first == key) {
                return 1;
            }
        }
        return 0;
    }

    
    /**
     * @brief Inserts a new key-value pair into the hash map.
     * If the key already exists, the function does nothing.
     *
     * @param data The key-value pair to insert.
     * @return true if the key-value pair was inserted, false otherwise.
     */
    bool insert(const data_t &data) {
        size_t h = Hash()(data.first) % MOD;
        for (node *p = m_data[h]; p; p = p->next) {
            if (p->data.first == data.first) {
                return false;
            }
        }
        node *tmp = new node{data, m_data[h]};
        m_data[h] = tmp;
        ++m_size;
        return true;
    }

    /**
     * @brief Returns the number of key-value pairs in the hash map.
     *
     * @return size_t The number of key-value pairs.
     */
    size_t size() const {
        return m_size;
    };

    /**
     * @brief Checks if the hash map is empty.
     *
     * @return true if the hash map is empty, false otherwise.
     */
    bool empty() const {
        return m_size == 0;
    }
};


} // namespace sjtu


#endif // HASHMAP_HPP

