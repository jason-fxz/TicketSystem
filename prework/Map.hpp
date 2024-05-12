/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>`
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

#include <iostream>
#include <iterator>

namespace sjtu {

template<typename Tp>
void swap(Tp &a, Tp &b) {
    Tp tmp = a;
    a = b;
    b = tmp;
}


template <class Key, class Val, class Compare = std::less<Key>> class RBTree {
  public:
    typedef pair<const Key, Val> data_type;
    typedef Key                  key_type;
    typedef Val                  value_type;

    enum ColorT { RED, BLACK };
    struct Node {
        typedef Node *NodePtr;
        NodePtr   left;
        NodePtr   right;
        NodePtr   parent;
        ColorT    color;
        data_type data;

        Node(const data_type &_data, ColorT _color)
            : data(_data), left(nullptr), right(nullptr), parent(nullptr), color(_color) {}

        const key_type &key() const { return data.first; }

        NodePtr minimum() {
            NodePtr x = this;
            while (x->left) x = x->left;
            return x;
        }

        NodePtr maximum() {
            NodePtr x = this;
            while (x->right) x = x->right;
            return x;
        }

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

    NodePtr m_root;
    size_t  m_size;

  public:
    RBTree() : m_root(nullptr), m_size(0) {}
    ~RBTree() { tree_delete(m_root); }

    RBTree(const RBTree &other) {
        tree_copy(m_root, other.m_root);
        m_size = other.m_size;
    }

    size_t size() const { return m_size; }

    bool empty() const { return m_root == nullptr; }

    void clear() {
        tree_delete(m_root);
        m_root = nullptr;
        m_size = 0;
    }

    RBTree &operator=(const RBTree &other) {
        if (this == &other) return *this;
        tree_delete(m_root);
        tree_copy(m_root, other.m_root);
        m_size = other.m_size;
        return *this;
    }

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

    NodePtr find(const key_type &key) const {
        NodePtr t = m_root;
        while (t) {
            if (Compare()(key, t->key())) t = t->left;
            else if (Compare()(t->key(), key)) t = t->right;
            else break;
        }
        return t;
    }

    void debug(NodePtr x) {
        if (x == nullptr) return;
        if (x->left) {
            if (x->left->parent != x) {
                std::cerr << "ERROR: parent child?" << std::endl;
                throw;
            }
            debug(x->left);
        }
        // std::cout << "DEBUG: (" << x->key() << ", " << x->data.second << ") " <<
        //   (x->color ? "BLACK" : "RED") << " :";
        if (x->left) {
            if (isRed(x)) {
                if (isRed(x->left)) {
                    std::cerr << "ERROR: red red" << std::endl;
                    throw;
                }
            }
            // std::cout << "l: " <<  x->left->key() << " ";
        }
        if (x->right) {
            if (isRed(x)) {
                if (isRed(x->right)) {
                    std::cerr << "ERROR: red red" << std::endl;
                    throw;
                }
            }
            // std::cout << "r: " << x->right->key() << " ";
        }
        // std::cout << std::endl;
        if (x->right) {
            if (x->right->parent != x) {
                std::cerr << "ERROR: child parent?" << std::endl;
                throw;
            }
            debug(x->right);
        }
    }
    void print() {
        std::cerr << "======= Print =======" << std::endl;
        // std::cerr << "Root = " << m_root->key() << std::endl;
        if (m_root->parent != nullptr) {
            std::cerr << "ERROR: root has parent!" << std::endl;
            throw;
        }
        if (m_root->color != BLACK) {
            std::cerr << "ERROR: root is not black!" << std::endl;
            throw;
        }
        debug(m_root);
        std::cerr << "===== End Print =====" << std::endl;
    }

  private:

    bool key_equal(const key_type &lhs, const key_type &rhs) const {
        return !(Compare()(lhs, rhs) || Compare()(rhs, lhs));
    }

    void tree_delete(NodePtr x) {
        if (x == nullptr) return;
        tree_delete(x->left);
        tree_delete(x->right);
        delete x;
        x = nullptr;
    }

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

    void LR(NodePtr gp) {
        RR(gp->left);
        LL(gp);
    }

    void RL(NodePtr gp) {
        LL(gp->right);
        RR(gp);
    }

    inline bool isBlack(NodePtr x) {
        return x && x->color == BLACK;
    }

    inline bool isRed(NodePtr x) {
        return x && x->color == RED;
    }

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

struct map_iterator_tag : std::bidirectional_iterator_tag {};


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
    /**
     * the internal type of data.
     * it should have a default constructor, a copy constructor.
     * You can use sjtu::map as value_type by typedef.
     */
    using value_type = typename RBTree<Key, T, Compare>::data_type;


    /**
     * see BidirectionalIterator at CppReference for help.
     *
     * if there is anything wrong throw invalid_iterator.
     *     like it = map.begin(); --it;
     *       or it = map.end(); ++end();
     */
    class const_iterator;
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
    /**
     * constructors
     */
    map() : RBTree<Key, T, Compare>() {}
    map(const map &other) : RBTree<Key, T, Compare>(other) {}
    /**
     * assignment operator
     */
    map &operator=(const map &other) {
        if (this == &other) return *this;
        RBTree<Key, T, Compare>::operator=(other);
        return *this;
    }
    /**
     * Destructors
     */
    ~map() {}
    /**
     * access specified element with bounds checking
     * Returns a reference to the mapped value of the element with key equivalent to key.
     * If no such element exists, an exception of type `index_out_of_bound'
     */
    T &at(const Key &key) {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }
    const T &at(const Key &key) const {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }
    /**
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     * performing an insertion if such key does not already exist.
     */
    T &operator[](const Key &key) {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) {
            return RBTree<Key, T, Compare>::insert({key, T()}).first->data.second;
        }
        return tmp->data.second;
    }
    /**
     * behave like at() throw index_out_of_bound if such key does not exist.
     */
    const T &operator[](const Key &key) const {
        NodePtr tmp = RBTree<Key, T, Compare>::find(key);
        if (tmp == nullptr) throw index_out_of_bound();
        return tmp->data.second;
    }
    /**
     * return a iterator to the beginning
     */
    iterator begin() {
        return iterator(RBTree<Key, T, Compare>::m_root ?
                        RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    const_iterator begin() const {
        return const_iterator(RBTree<Key, T, Compare>::m_root ?
                              RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    const_iterator cbegin() const {
        return const_iterator(RBTree<Key, T, Compare>::m_root ?
                              RBTree<Key, T, Compare>::m_root->minimum() : nullptr, this);
    }
    /**
     * return a iterator to the end
     * in fact, it returns past-the-end.
     */
    iterator end() {
        return iterator(nullptr, this);
    }
    const_iterator end() const {
        return const_iterator(nullptr, this);
    }
    const_iterator cend() const {
        return const_iterator(nullptr, this);
    }
    /**
     * checks whether the container is empty
     * return true if empty, otherwise false.
     */
    bool empty() const {
        return RBTree<Key, T, Compare>::empty();
    }
    /**
     * returns the number of elements.
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
     * insert an element.
     * return a pair, the first of the pair is
     *   the iterator to the new element (or the element that prevented the insertion),
     *   the second one is true if insert successfully, or false.
     */
    pair<iterator, bool> insert(const value_type &value) {
        auto tmp = RBTree<Key, T, Compare>::insert(value);
        return pair<iterator, bool>(iterator(tmp.first, this), tmp.second);
    }
    /**
     * erase the element at pos.
     *
     * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
     */
    void erase(iterator pos) {
        if (pos.mp != this || pos == end()) throw invalid_iterator();
        RBTree<Key, T, Compare>::remove(pos.ptr->key());
    }
    /**
     * erase the element with key.
     * it will do nothing if the key doesn't exist.
     */
    void erase(const Key &key) {
        RBTree<Key, T, Compare>::remove(key);
    }

    /**
     * Returns the number of elements with key
     *   that compares equivalent to the specified argument,
     *   which is either 1 or 0
     *     since this container does not allow duplicates.
     * The default method of check the equivalence is !(a < b || b > a)
     */
    size_t count(const Key &key) const {
        return RBTree<Key, T, Compare>::find(key) == nullptr ? 0 : 1;
    }
    /**
     * Finds an element with key equivalent to key.
     * key value of the element to search for.
     * Iterator to an element with key equivalent to key.
     *   If no such element is found, past-the-end (see end()) iterator is returned.
     */
    iterator find(const Key &key) {
        return iterator(RBTree<Key, T, Compare>::find(key), this);
    }
    const_iterator find(const Key &key) const {
        return const_iterator(RBTree<Key, T, Compare>::find(key), this);
    }

    // void debug() {
    //     RBTree<Key, T, Compare>::print();
    // }
};

}

#endif
