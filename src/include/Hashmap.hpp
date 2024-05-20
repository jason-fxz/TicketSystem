/**
 * @file Hashmap.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief Hashmap and LRUHashmap class
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
  public:
    typedef std::pair<const Key, Tp> Data_t;
  private:
    /**
     * @brief Node structure for storing key-value pairs.
     */
    struct node {
        Data_t data;       ///< The key-value pair.
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
    bool insert(const Data_t &data) {
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



template <class Key, class Tp, size_t MOD, class Hash = std::hash<Key>>
class LRUHashmap {
  private:
    typedef std::pair<const Key, Tp> data_t;
    /**
     * @brief Node structure for storing key-value pairs.
     */
    struct node {
        data_t data;       ///< The key-value pair.
        node  *next;       ///< Pointer to the next node in the chain.
        node  *list_next;
        node  *list_prev;
    };
    node *m_data[MOD];     ///< Array of pointers to the head of each chain.
    node *m_list_head, *m_list_tail;
    size_t m_size;         ///< The number of key-value pairs in the hash map.


    void list_remove(node *p) {
        if (p == m_list_head) {
            m_list_head = p->list_next;
        }
        if (p->list_prev) p->list_prev->list_next = p->list_next;
        if (p->list_next) p->list_next->list_prev = p->list_prev;
    }

    void list_move_to_head(node *p) {
        if (p == m_list_head) return;
        list_remove(p);
        p->list_next = m_list_head;
        p->list_prev = nullptr;
        m_list_head->list_prev = p;
        m_list_head = p;
    }


  public:
    /**
     * @brief Constructs an empty hash map.
     */
    LRUHashmap() {
        m_size = 0;
        for (size_t i = 0; i < MOD; ++i) {
            m_data[i] = nullptr;
        }
        m_list_head = m_list_tail = new node{data_t{}, nullptr, nullptr, nullptr};
    }

    /**
     * @brief Destroys the hash map and frees the memory.
     */
    ~LRUHashmap() {
        clear();
        delete m_list_tail;
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
        m_list_head = m_list_tail;
    }

    bool check(const Key &key) {
        size_t h = Hash()(key) % MOD;
        for (node *pp = m_data[h], *p = m_data[h]; p; pp = p, p = p->next) {
            if (p->data.first == key) {
                list_move_to_head(p);
                return true;
            }
        }
        return false;
    }


    Tp &at(const Key &key) {
        size_t h = Hash()(key) % MOD;
        for (node *pp = m_data[h], *p = m_data[h]; p; pp = p, p = p->next) {
            if (p->data.first == key) {
                list_move_to_head(p);
                return p->data.second;
            }
        }
        node *tmp = new node{{key, Tp()}, m_data[h], m_list_head, nullptr};
        m_list_head->list_prev = tmp;
        m_list_head = tmp;
        m_data[h] = tmp;
        ++m_size;
        return tmp->data.second;
    }


    void insert(const Key &key, const Tp &value) {
        size_t h = Hash()(key) % MOD;
        node *tmp = new node{{key, value}, m_data[h], m_list_head, nullptr};
        m_list_head->list_prev = tmp;
        m_list_head = tmp;
        m_data[h] = tmp;
        ++m_size;
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
                list_remove(p);
                delete p;
                --m_size;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Removes the last key-value pair from the hash map.
     */
    void pop_back() {
        if (m_list_tail == m_list_head) return;
        node *p = m_list_tail->list_prev;
        erase(p->data.first);
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

