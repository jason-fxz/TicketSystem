/**
 * @file Map.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief implement a container like std::map
 * @version 0.1
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>`
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#include <iterator>

namespace sjtu {

/**
 * @brief A red-black tree implementation.
 *
 * This class provides functionality for inserting, removing, and finding elements in a red-black tree.
 * @tparam Key
 * @tparam Val
 * @tparam Compare
 */
template <class Key, class Val, class Compare = std::less<Key>> class RBTree {
  public:
    typedef pair<const Key, Val>
    data_type;     ///< The type of data stored in the tree.
    typedef Key                  key_type;      ///< The type of the key.
    typedef Val                  value_type;    ///< The type of the value.

    enum ColorT { RED, BLACK };
    struct Node {
        typedef Node *NodePtr;
        NodePtr   left;       ///< Pointer to the left child.
        NodePtr   right;      ///< Pointer to the right child.
        NodePtr   parent;     ///< Pointer to the parent node.
        ColorT    color;      ///< The color of the node.
        data_type data;       ///< The data stored in the node.


        /**
         * @brief Construct a new Node object
         *
         * @param _data
         * @param _color
         */
        Node(const data_type &_data, ColorT _color)
            : data(_data), left(nullptr), right(nullptr), parent(nullptr), color(_color) {}

        /**
         * @brief Get the key object
         *
         * @return const key_type&
         */
        const key_type &key() const { return data.first; }

        /**
         * @brief Get the minimum in the subtree rooted at this node.
         *
         * @return NodePtr
         */
        NodePtr minimum() {
            NodePtr x = this;
            while (x->left) x = x->left;
            return x;
        }

        /**
         * @brief Get the maximum in the subtree rooted at this node.
         *
         * @return NodePtr
         */
        NodePtr maximum() {
            NodePtr x = this;
            while (x->right) x = x->right;
            return x;
        }

        /**
         * @brief Get the successor of this node.
         *
         * @return NodePtr
         */
        NodePtr successor() {
            if (right) return right->minimum();
            NodePtr x = this;
            NodePtr y = parent;
            while (y && x == y->right) {
                x = y;
                y = y->parent;
            }
            return y;
        }

        /**
         * @brief Get the predecessor of this node.
         *
         * @return NodePtr
         */
        NodePtr predecessor() {
            if (left) return left->maximum();
            NodePtr x = this;
            NodePtr y = parent;
            while (y && x == y->left) {
                x = y;
                y = y->parent;
            }
            return y;
        }

    };

    typedef Node   *NodePtr;

    NodePtr m_root;  ///< The root of the tree.
    size_t  m_size;  ///< The number of nodes in the tree.

  public:
    /**
     * @brief Construct a new RBTree object
     */
    RBTree() : m_root(nullptr), m_size(0) {}

    /**
     * @brief Destroy the RBTree object
     */
    ~RBTree() { tree_delete(m_root); }

    /**
     * @brief Construct a new RBTree object by copying another tree.
     *
     * @param other
     */
    RBTree(const RBTree &other) {
        tree_copy(m_root, other.m_root);
        m_size = other.m_size;
    }

    /**
     * @brief Get the number of nodes in the tree.
     *
     * @return size_t
     */
    size_t size() const { return m_size; }

    /**
     * @brief Check if the tree is empty.
     *
     * @return true if the tree is empty, otherwise false.
     */
    bool empty() const { return m_root == nullptr; }

    /**
     * @brief Clear the tree.
     *
     */
    void clear() {
        tree_delete(m_root);
        m_root = nullptr;
        m_size = 0;
    }

    /**
     * @brief assignment operator for the tree.
     * delete the current tree, copy the tree from another tree.
     *
     * @param other
     * @return RBTree&
     */
    RBTree &operator=(const RBTree &other) {
        if (this == &other) return *this;
        tree_delete(m_root);
        tree_copy(m_root, other.m_root);
        m_size = other.m_size;
        return *this;
    }

    /**
     * @brief Insert a new node into the tree.
     *
     * @param data key-value pair to insert.
     * @return pair<NodePtr, bool>
     */
    pair<NodePtr, bool> insert(const data_type &data) {
        NodePtr t, p, gp;
        if (m_root == nullptr) {
            m_root = new Node(data, BLACK);
            m_size = 1;
            return pair<NodePtr, bool>(m_root, true);
        }
        p = gp = t = m_root;
        while (true) {
            if (t) {
                if (isRed(t->left) && isRed(t->right)) {
                    t->left->color = t->right->color = BLACK;
                    t->color = RED;
                    insertAdjust(gp, p, t);
                }
                gp = p;
                p = t;
                if (Compare()(data.first, t->key())) t = t->left;
                else if (Compare()(t->key(), data.first)) t = t->right;
                else return pair<NodePtr, bool>(t, false);
            } else {
                t = new Node(data, RED);
                if (Compare()(data.first, p->key())) p->left = t;
                else p->right = t;
                t->parent = p;
                insertAdjust(gp, p, t);
                m_root->color = BLACK;
                ++m_size;
                return pair<NodePtr, bool>(t, true);
            }
        }
    }

    /**
     * @brief Remove a node from the tree by given key.
     * if the key is not found, do nothing.
     * @param key
     * @return true if the node is removed, otherwise false.
     */
    bool remove(const key_type &key) {
        NodePtr c, p, t; // c: current node, p: parent node, t: brother node
        key_type del = key;
        if (m_root == nullptr) return false;
        if (key_equal(m_root->key(), key) && m_root->left == nullptr
            && m_root->right == nullptr) {
            delete m_root;
            m_size = 0;
            m_root = nullptr;
            return true;
        }
        p = c = t = m_root;
        while (c != nullptr) {
            removeAdjust(p, c, t, del);
            if (key_equal(c->key(), del) && c->left && c->right) {
                NodePtr tmp = c->right;
                while (tmp->left) tmp = tmp->left;
                swap(tmp->color, c->color);
                tmp->left = c->left;
                c->left = nullptr;
                if (tmp->left) tmp->left->parent = tmp;
                if (c->parent) {
                    if (c->parent->left == c) c->parent->left = tmp;
                    else c->parent->right = tmp;
                } else {
                    m_root = tmp;
                }
                NodePtr pc = c->parent;
                if (tmp->parent != c) {
                    tmp->parent->left = c;
                    c->parent = tmp->parent;
                    swap(c->right, tmp->right);
                    if (c->right) c->right->parent = c;
                    if (tmp->right) tmp->right->parent = tmp;
                } else {
                    c->right = tmp->right;
                    if (c->right) c->right->parent = c;
                    c->parent = tmp;
                    tmp->right = c;
                }
                tmp->parent = pc;
                p = tmp;
                c = p->right;
                t = p->left;
                continue;
            }
            if (key_equal(c->key(), del)) {
                delete c;
                --m_size;
                if (p->left == c) p->left = nullptr;
                else p->right = nullptr;
                m_root->color = BLACK;
                return true;
            }
            p = c;
            c = (Compare()(del, c->key())) ? c->left : c->right;
            t = (c == p->left) ? p->right : p->left;
        }
        return false;
    }

    /**
     * @brief Find a node by given key.
     * if the key is not found, return nullptr.
     *
     * @param key
     * @return NodePtr
     */
    NodePtr find(const key_type &key) const {
        NodePtr t = m_root;
        while (t) {
            if (Compare()(key, t->key())) t = t->left;
            else if (Compare()(t->key(), key)) t = t->right;
            else break;
        }
        return t;
    }

  private:

    /**
     * @brief Check if two keys are equal.
     *
     * @param lhs
     * @param rhs
     * @return true if the keys are equal, otherwise false.
     */
    bool key_equal(const key_type &lhs, const key_type &rhs) const {
        return !(Compare()(lhs, rhs) || Compare()(rhs, lhs));
    }

    /**
     * @brief Delete a subtree rooted at x.
     *
     * @param x
     */
    void tree_delete(NodePtr x) {
        if (x == nullptr) return;
        tree_delete(x->left);
        tree_delete(x->right);
        delete x;
        x = nullptr;
    }

    /**
     * @brief Copy a tree from src to dest.
     *
     * @param dest
     * @param src
     */
    void tree_copy(NodePtr &dest, NodePtr src) {
        if (src == nullptr) {
            dest = nullptr;
            return;
        }
        dest = new Node(src->data, src->color);
        tree_copy(dest->left, src->left);
        tree_copy(dest->right, src->right);
        if (dest->left) dest->left->parent = dest;
        if (dest->right) dest->right->parent = dest;
    }

    /**
     * @brief LL rotation.
     *
     * @param gp
     */
    void LL(NodePtr gp) {
        NodePtr p = gp->left;
        gp->left = p->right;
        if (p->right) p->right->parent = gp;
        p->right = gp;
        p->parent = gp->parent;
        if (gp->parent) {
            if (gp->parent->left == gp) gp->parent->left = p;
            else gp->parent->right = p;
        } else {
            m_root = p;
        }
        gp->parent = p;
    }

    /**
     * @brief RR rotation.
     *
     * @param gp
     */
    void RR(NodePtr gp) {
        NodePtr p = gp->right;
        gp->right = p->left;
        if (p->left) p->left->parent = gp;
        p->left = gp;
        p->parent = gp->parent;
        if (gp->parent) {
            if (gp->parent->left == gp) gp->parent->left = p;
            else gp->parent->right = p;
        } else {
            m_root = p;
        }
        gp->parent = p;
    }

    /**
     * @brief LR rotation.
     *
     * @param gp
     */
    void LR(NodePtr gp) {
        RR(gp->left);
        LL(gp);
    }

    /**
     * @brief RL rotation.
     *
     * @param gp
     */
    void RL(NodePtr gp) {
        LL(gp->right);
        RR(gp);
    }

    /**
     * @brief Check if a node is black.
     *
     * @param x
     * @return true if the node is black, otherwise false.
     */
    inline bool isBlack(NodePtr x) {
        return x && x->color == BLACK;
    }

    /**
     * @brief Check if a node is red.
     *
     * @param x
     * @return true if the node is red, otherwise false.
     */
    inline bool isRed(NodePtr x) {
        return x && x->color == RED;
    }

    /**
    * @brief Adjust the tree after inserting a node.
    *
    * @param gp grandparent
    * @param p  parent
    * @param t  current node
    */
    void insertAdjust(NodePtr gp, NodePtr p, NodePtr t) {
        if (p->color == BLACK) return;
        if (p == m_root) {
            p->color = BLACK;
            return;
        }
        if (gp->left == p) {
            if (p->left == t) LL(gp), swap(p->color, gp->color);
            else LR(gp), swap(t->color, gp->color);
        } else {
            if (p->right == t) RR(gp), swap(p->color, gp->color);
            else RL(gp), swap(t->color, gp->color);
        }
    }

    /**
     * @brief Adjust the tree after removing a node.
     *
     * @param p  parent
     * @param c  current node
     * @param t  brother node
     * @param del key to delete
     */
    void removeAdjust(NodePtr &p, NodePtr &c, NodePtr &t, key_type del) {
        if (isRed(c)) return;
        if (c == m_root) {
            if (c->left && c->right && c->left->color == c->right->color) {
                c->color = RED;
                c->left->color = c->right->color = BLACK;
                return ;
            }
        }
        if (!isRed(c->left) && !isRed(c->right)) {
            if (!isRed(t->left) && !isRed(t->right)) {
                p->color = BLACK;
                c->color = t->color = RED;
            } else {
                if (p->left == t) {
                    if (isRed(t->left)) {
                        t->left->color = BLACK;
                        p->color = BLACK;
                        t->color = RED;
                        LL(p);
                    } else {
                        p->color = BLACK;
                        LR(p);
                    }
                } else {
                    if (isRed(t->right)) {
                        t->right->color = BLACK;
                        p->color = BLACK;
                        t->color = RED;
                        RR(p);
                    } else {
                        p->color = BLACK;
                        RL(p);
                    }
                }
                c->color = RED;
            }
        } else {
            if (key_equal(c->key(), del)) {
                if (c->left && c->right) {
                    if (c->right->color == BLACK) {
                        c->left->color = BLACK;
                        c->color = RED;
                        p = c->left;
                        LL(c);
                    }
                    return ;
                }
                if (c->left) {
                    c->left->color = BLACK;
                    c->color = RED;
                    p = c->left;
                    LL(c);
                } else {
                    c->right->color = BLACK;
                    c->color = RED;
                    p = c->right;
                    RR(c);
                }
            } else {
                p = c;
                c = (Compare()(del, p->key())) ? p->left : p->right;
                t = (c == p->left) ? p->right : p->left;
                if (c->color == BLACK) {
                    t->color = BLACK, p->color = RED;
                    if (t == p->right) RR(p);
                    else LL(p);
                    t = (c == p->left) ? p->right : p->left;
                    removeAdjust(p, c, t, del);
                }
            }
        }
    }
};

/**
 * @brief Iterator tags for map.
 */
struct map_iterator_tag : std::bidirectional_iterator_tag {};

/**
 * @brief A map implementation based on red-black tree.
 *
 * @tparam Key key type
 * @tparam T value type
 * @tparam Compare comparison function
 */
template <
    class Key,
    class T,
    class Compare = std::less<Key>
    > class map : public RBTree<Key, T, Compare> {
  private:
    using Node = typename RBTree<Key, T, Compare>::Node;
    using NodePtr = Node*;
    using ConstNodePtr = const Node*;
  public:

    using value_type = typename
                       RBTree<Key, T, Compare>::data_type; ///< The type of data stored in the map.

    class const_iterator;
    /**
     * @brief Iterator for map.
     * this iterator is bidirectional.
     * you can use it++, ++it to move to the next element, and it--, --it to move to the previous element.
     */
    class iterator {
        friend map;
      private:
        const map *mp;
        NodePtr ptr;

      public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename RBTree<Key, T, Compare>::data_type;
        using iterator_category = map_iterator_tag;
        using pointer = value_type *;
        using reference = value_type &;

        iterator() : ptr(nullptr), mp(nullptr) {}
        iterator(NodePtr _ptr, const map *_mp) : ptr(_ptr), mp(_mp) {}
        iterator(const iterator &other) : ptr(other.ptr), mp(other.mp) {}

        iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator &operator++() {
            if (ptr == nullptr) throw invalid_iterator();
            ptr = ptr->successor();
            return *this;
        }

        iterator operator--(int) {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        iterator &operator--() {
            if (ptr == nullptr) {
                if (mp->m_root == nullptr) throw invalid_iterator();
                ptr = mp->m_root->maximum();
            } else {
                auto tmp = ptr->predecessor();
                if (tmp == nullptr) throw invalid_iterator();
                ptr = tmp;
            }
            return *this;
        }
        /**
         * a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {
            return mp == rhs.mp && ptr == rhs.ptr;
        }
        bool operator==(const const_iterator &rhs) const {
            return mp == rhs.mp && ptr == rhs.ptr;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }

        reference operator*() const {
            return ptr->data;
        }
        pointer operator->() const noexcept {
            return &(ptr->data);
        }
    };

    class const_iterator {
        friend map;
      private:
        NodePtr ptr;
        const map *mp;
      public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename RBTree<Key, T, Compare>::data_type;
        using iterator_category = map_iterator_tag;
        using pointer = const value_type *;
        using reference = const value_type &;

        const_iterator() : ptr(nullptr), mp(nullptr) {}
        const_iterator(NodePtr _ptr, const map *_mp) : ptr(_ptr), mp(_mp) {}
        const_iterator(const const_iterator &other) : ptr(other.ptr), mp(other.mp) {}
        const_iterator(const iterator &other) : ptr(other.ptr), mp(other.mp) {}

        const_iterator operator++(int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const_iterator &operator++() {
            if (ptr == nullptr) throw invalid_iterator();
            ptr = ptr->successor();
            return *this;
        }

        const_iterator operator--(int) {
            auto tmp = *this;
            --(*this);
            return tmp;
        }

        const_iterator &operator--() {
            if (ptr == nullptr) {
                if (mp->m_root == nullptr) throw invalid_iterator();
                ptr = mp->m_root->maximum();
            } else {
                auto tmp = ptr->predecessor();
                if (tmp == nullptr) throw invalid_iterator();
                ptr = tmp;
            }
            return *this;
        }
        /**
         * @brief a operator to check whether two iterators are same (pointing to the same memory).
         */
        bool operator==(const iterator &rhs) const {
            return mp == rhs.mp && ptr == rhs.ptr;
        }
        bool operator==(const const_iterator &rhs) const {
            return mp == rhs.mp && ptr == rhs.ptr;
        }
        /**
         * @brief some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
        reference operator*() const {
            return ptr->data;
        }
        pointer operator->() const noexcept {
            return &(ptr->data);
        }
    };
    /**
     * @brief Construct a new map object
     *
     */
    map() : RBTree<Key, T, Compare>() {}
    /**
     * Construct a new map object by copying another map.
     *
     * @param other another map
     */
    map(const map &other) : RBTree<Key, T, Compare>(other) {}
    /**
     * @brief Assignment operator for map.
     *
     * @param other another map
     * @return map&
     */
    map &operator=(const map &other) {
        if (this == &other) return *this;
        RBTree<Key, T, Compare>::operator=(other);
        return *this;
    }
    /**
     * @brief Destroy the map object
     *
     */
    ~map() {}

    /**
     * @brief access specified element with bounds checking
     *  Returns a reference to the mapped value of the element with key equivalent to key.
     *  If no such element exists, an exception of type `index_out_of_bound' is thrown.
     * @throw index_out_of_bound
     * @param key
     * @return T&
     */
    T &at(const Key &key) {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }

    /**
     * @brief access specified element with bounds checking, const version
     *  Returns a reference to the mapped value of the element with key equivalent to key.
     *  If no such element exists, an exception of type `index_out_of_bound' is thrown.
     * @throw index_out_of_bound
     * @param key
     * @return const T&
     */
    const T &at(const Key &key) const {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }
    /**
     * @brief access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     * performing an insertion if such key does not already exist.
     * @param key
     * @return T&
     */
    T &operator[](const Key &key) {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) {
            return RBTree<Key, T, Compare>::insert({key, T()}).first->data.second;
        }
        return tmp->data.second;
    }
    /**
     * @brief access specified element, const version
     * Returns a reference to the value that is mapped to a key equivalent to key,
     * If no such element exists, an exception of type `index_out_of_bound' is thrown.
     * @throw index_out_of_bound
     * @param key
     * @return const T&
     */
    const T &operator[](const Key &key) const {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }
    /**
     * return an iterator to the beginning
     * @return iterator
     */
    iterator begin() {
        return iterator(RBTree<Key, T, Compare>::m_root ?
                        RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    /**
     * return an const_iterator to the beginning
     * @return const_iterator
     */
    const_iterator begin() const {
        return const_iterator(RBTree<Key, T, Compare>::m_root ?
                              RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    /**
     * return a const_iterator to the beginning
     * @return const_iterator
     */
    const_iterator cbegin() const {
        return const_iterator(RBTree<Key, T, Compare>::m_root ?
                              RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    /**
     * @brief return a iterator to the end
     * in fact, it returns past-the-end.
     * @return iterator
     */
    iterator end() {
        return iterator(nullptr, this);
    }
    /**
     * @brief return a const_iterator to the end
     * in fact, it returns past-the-end.
     * @return const_iterator
     */
    const_iterator end() const {
        return const_iterator(nullptr, this);
    }
    /**
     * @brief return a const_iterator to the end
     * in fact, it returns past-the-end.
     * @return const_iterator
     */
    const_iterator cend() const {
        return const_iterator(nullptr, this);
    }
    /**
     * checks whether the container is empty
     * @return true if empty, otherwise false.
     */
    bool empty() const {
        return RBTree<Key, T, Compare>::empty();
    }
    /**
     * returns the number of elements.
     * @return size_t
     */
    size_t size() const {
        return RBTree<Key, T, Compare>::size();
    }
    /**
     * clears the contents
     */
    void clear() {
        RBTree<Key, T, Compare>::clear();
    }
    /**
     * @brief insert an element.
     * If there is already an element with the key in the container, does nothing.
     *
     * @param x key-value pair to insert.
     * @return pair<iterator, bool> iterator to the new element, and a bool value to indicate whether the insertion is successful.
     */
    pair<iterator, bool> insert(const value_type &x) {
        auto tmp = RBTree<Key, T, Compare>::insert(x);
        return pair<iterator, bool>(iterator(tmp.first, this), tmp.second);
    }
    /**
     * @brief erase an element at pos.
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     * @param pos the iterator to the element to erase.
     * @throw invalid_iterator 
     */
    void erase(iterator pos) {
        if (pos.mp != this || pos == end()) throw invalid_iterator();
        RBTree<Key, T, Compare>::remove(pos.ptr->key());
    }
    /**
     * @brief erase an element by key.
     *  If there is no such element, do nothing.
     * @param key the key of the element to erase.
     */
    void erase(const Key &key) {
        RBTree<Key, T, Compare>::remove(key);
    }

    /**
     * @brief Returns the number of elements by key
     * It should be either 1 or 0 since this container does not allow duplicates.
     * @param key key of the element to count.
     * @return size_t 1 if the element is found, otherwise 0.
     */
    size_t count(const Key &key) const {
        return RBTree<Key, T, Compare>::find(key) == nullptr ? 0 : 1;
    }
    /**
     * @brief Finds an element with key equivalent to key.
     * Return the iterator to the element if found. 
     * If no such element is found, past-the-end (see end()) iterator is returned.
     * @param key key of the element to search for.
     * @return iterator to an element with key equivalent to key.
     */
    iterator find(const Key &key) {
        return iterator(RBTree<Key, T, Compare>::find(key), this);
    }
    /**
     * @brief Finds an element with key equivalent to key. const version
     * Return the iterator to the element if found. 
     * If no such element is found, past-the-end (see end()) iterator is returned.
     * @param key key of the element to search for.
     * @return const_iterator to an element with key equivalent to key.
     */
    const_iterator find(const Key &key) const {
        return const_iterator(RBTree<Key, T, Compare>::find(key), this);
    }

};

}

#endif
