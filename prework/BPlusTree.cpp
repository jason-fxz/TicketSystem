#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
// #include <map>

template <class Tp>
int Camp(const Tp &x, const Tp &y) {
    return x < y ? -1 : (x == y ? 0 : 1);
}

template<class T1, class T2>
class pair {
  public:
    T1 first;
    T2 second;

    constexpr pair() = default;
    constexpr pair(const pair &other) = default;
    constexpr pair(pair &&other) = default;

    template<class U1 = T1, class U2 = T2>
    constexpr pair(U1 && x, U2 && y)
        : first(std::forward<U1>(x)), second(std::forward<U2>(y)) {}

    template<class U1, class U2>
    constexpr pair(const pair<U1, U2> &other)
        : first(other.first), second(other.second) {}

    template<class U1, class U2>
    constexpr pair(pair<U1, U2> &&other)
        : first(std::move(other.first))
        , second(std::move(other.second)) {}

    pair &operator=(const pair &other) = default;
    pair &operator=(pair &&other) = default;


    bool operator==(const pair &rhs) const {
        return first == rhs.first && second == rhs.second;
    }

    bool operator!=(const pair &rhs) const {
        return first != rhs.first || second != rhs.second;
    }

    bool operator<(const pair &rhs) const {
        return first < rhs.first || (first == rhs.first && second < rhs.second);
    }
};

template<class T1, class T2>
pair(T1, T2) -> pair<T1, T2>;


template<class Tp1, class Tp2>
std::ostream &operator<<(std::ostream &os, const pair<Tp1, Tp2> &p) {
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

template <size_t len>
class String {
  private:
    char data[len];
  public:

    String() { memset(data, 0, sizeof(data)); }
    String(const char *str) {
        strcpy(data, str);
    }
    String(const String &rhs) {
        memcpy(data, rhs.data, sizeof(data));
    }
    ~String() = default;

    String &operator=(const String &rhs) {
        if (this == &rhs) return *this;
        memcpy(data, rhs.data, sizeof(data));
        return *this;
    }

    String &operator=(const char *str) {
        memset(data, 0, sizeof(data));
        strcpy(data, str);
        return *this;
    }

    bool operator==(const String &rhs) const {
        return strcmp(data, rhs.data) == 0;
    }

    bool operator<(const String &rhs) const {
        return strcmp(data, rhs.data) < 0;
    }

    bool empty() const {
        return data[0] == '\0';
    }

    char &operator[](const int &k) {
        return data[k];
    }

    char operator[](const int &k) const {
        return data[k];
    }

    char *c_str() const {
        return data;
    }

    friend std::ostream &operator<<(std::ostream &os, const String &str) {
        os << str.data;
        return os;
    }

    friend std::istream &operator>>(std::istream &is, String &str) {
        is >> str.data;
        return is;
    }
};


template<int info_len, size_t BLOCK_SIZE = 4096>
class File {
  private:
    std::fstream file;
    std::string file_name;
    char buffer[BLOCK_SIZE];
    char infobuffer[info_len * sizeof(int)];
  public:
    File() {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    File(const std::string &file_name) : file_name(file_name) {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    ~File() {
        file.seekp(0);
        file.write(infobuffer, info_len * sizeof(int));
        file.close();
    }

    // init the file: create file if not exist or clear the file
    void init(std::string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out | std::ios::binary);
        file.write(buffer, BLOCK_SIZE);
        file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        memset(infobuffer, 0, sizeof(infobuffer));
    }

    // check if the file exist
    bool exist() {
        file.open(file_name, std::ios::in);
        bool res = file.is_open();
        file.close();
        return res;
    }

    // just open the file
    void open(std::string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(0);
        file.read(infobuffer, info_len * sizeof(int));
    }

    //读出第 n 个 int 的值赋给 tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        char *p = infobuffer + (n - 1) * sizeof(int);
        memcpy(&tmp, p, sizeof(int));
    }

    //将 tmp 写入第 n 个 int 的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        char *p = infobuffer + (n - 1) * sizeof(int);
        memcpy(p, &tmp, sizeof(int));
    }

    // 得到要写入的位置索引, 开辟空间
    int write() {
        file.seekp(0, std::ios::end);
        int index = file.tellp() / BLOCK_SIZE;
        file.write(buffer, BLOCK_SIZE);
        return index;
    }

    //用 t 的值更新位置索引 index 对应的对象，保证调用的 index 都是由 write 函数产生
    template<typename T>
    void update(T &t, const int index) {
        file.seekp(index * BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(&t), sizeof(T));
    }

    //读出位置索引 index 对应的 T 对象的值并赋值给 t，保证调用的 index 都是由 write 函数产生
    template<typename T>
    void read(T &t, const int index) {
        file.seekg(index * BLOCK_SIZE);
        file.read(reinterpret_cast<char *>(&t), sizeof(T));
    }

};

template <class Tp>
class stack {
  private:
    struct node {
        Tp value;
        node *next;
    };
    size_t m_size;
    node *m_top;
  public:

    stack() : m_size(0), m_top(nullptr) {}
    ~stack() {
        while (m_top) {
            node *tmp = m_top;
            m_top = m_top->next;
            delete tmp;
        }
    }

    struct iterator {
        node *ptr;
        iterator(node *_ptr) : ptr(_ptr) {}
        Tp operator*() const {
            return ptr->value;
        }
        Tp *operator->() const {
            return &(ptr->value);
        }
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }
        bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }

        iterator &operator++() {
            ptr = ptr->next;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ptr = ptr->next;
            return tmp;
        }
    };

    iterator begin() {
        return iterator(m_top);
    }

    iterator end() {
        return iterator(nullptr);
    }

    Tp top() const {
        return m_top->value;
    }

    void push(const Tp &value) {
        node *tmp = new node{value, m_top};
        m_top = tmp;
        ++m_size;
    }
    void pop() {
        node *tmp = m_top;
        m_top = m_top->next;
        delete tmp;
        --m_size;
    }
    size_t size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
};


template <class Tp, size_t MAX_SIZE>
class queue {
  private:
    Tp data[MAX_SIZE];
    size_t head, tail, m_size;
  public:
    queue() : head(0), tail(0), m_size(0) {}
    ~queue() = default;

    void clear() {
        head = tail = m_size = 0;
    }

    void push(const Tp &value) {
        data[tail] = value;
        tail = (tail + 1) % MAX_SIZE;
        ++m_size;
    }

    void pop() {
        head = (head + 1) % MAX_SIZE;
        --m_size;
    }

    Tp front() const {
        return data[head];
    }

    Tp back() const {
        return data[(tail - 1 + MAX_SIZE) % MAX_SIZE];
    }

    size_t size() const {
        return m_size;
    }

    bool empty() const {
        return m_size == 0;
    }
};


// only for std::less<T>
// #include <functional>
// #include <cstddef>
// #include "utility.hpp"
// #include "exceptions.hpp"

// #include <iostream>
// #include <cassert>
// #include <iterator>

namespace sjtu {
class exception {
  protected:
    const std::string variant = "";
    std::string detail = "";
  public:
    exception() {}
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {
    /* __________________________ */
};

class runtime_error : public exception {
    /* __________________________ */
};

class invalid_iterator : public exception {
    /* __________________________ */
};

class container_is_empty : public exception {
    /* __________________________ */
};


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
            : left(nullptr), right(nullptr), parent(nullptr), color(_color), data(_data) {}

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
            // assert(m_size == 1);
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
                // assert(c->left == nullptr && c->right == nullptr);
                // assert(c->parent == p);
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
        // assert(p->parent == gp);
        // assert(t->parent == p);
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
                    // assert(isRed(c->left));
                    c->left->color = BLACK;
                    c->color = RED;
                    p = c->left;
                    LL(c);
                } else {
                    // assert(isRed(c->right));
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
                    // assert(isRed(t));
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
        NodePtr ptr;
        const map *mp;

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
     * TODO two constructors
     */
    map() : RBTree<Key, T, Compare>() {}
    map(const map &other) : RBTree<Key, T, Compare>(other) {}
    /**
     * TODO assignment operator
     */
    map &operator=(const map &other) {
        if (this == &other) return *this;
        RBTree<Key, T, Compare>::operator=(other);
        return *this;
    }
    /**
     * TODO Destructors
     */
    ~map() {}
    /**
     * TODO
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
     * TODO
     * access specified element
     * Returns a reference to the value that is mapped to a key equivalent to key,
     *   performing an insertion if such key does not already exist.
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

    void debug() {
        RBTree<Key, T, Compare>::print();
    }
};

}


template <typename Tp> 
inline void quickcopy(Tp *dest, const Tp *src, size_t n) {
    memmove(dest, src, n * sizeof(Tp));
}

// B+ Tree database, Every Key should be unique!!
template < typename Key, typename Tp, size_t M, size_t L,
           size_t FILE_BLOCK_SIZE = 4096,
           size_t MAX_CACHE_SIZE = 8000,
           size_t MAX_NODE_SIZE = M,
           size_t MIN_NODE_SIZE = (M + 1) / 2,
           size_t MAX_LEAF_SIZE = L,
           size_t MIN_LEAF_SIZE = (L + 1) / 2 >
class BPlusTree {
    using Data_t = Tp;
    using Key_t = Key;
    struct node {
        int count; // count of children
        int index; // index in disk , >0 is inner_node, <0 is leaf_node, =0 is empty
        int get_index() const { // return the index in disk
            return index < 0 ? -index : index;
        }
        bool is_inner() const {
            return index > 0;
        }
        void set_index(int _index, bool is_inner) { // _index is the index in disk
            index = is_inner ? _index : -_index;
        }
    };

    struct inner_node : node {
        Key_t key[MAX_NODE_SIZE - 1];
        int child[MAX_NODE_SIZE];
    };

    struct leaf_node : node {
        Key_t key[MAX_LEAF_SIZE];
        Data_t data[MAX_LEAF_SIZE];
        int next;
    };

    class BNodePtr {
        File<2, FILE_BLOCK_SIZE> *file;
        size_t *refCount;
        node *ptr;

      public:
        BNodePtr() : file(nullptr), refCount(nullptr), ptr(nullptr) {}
        BNodePtr(File<2, FILE_BLOCK_SIZE> *dbfile, node *_ptr) : file(dbfile),
            refCount(new size_t(1)), ptr(_ptr) {}
        BNodePtr(File<2, FILE_BLOCK_SIZE> *dbfile, int index) : file(dbfile),
            refCount(new size_t(1)) {
            if (index > 0) {
                ptr = new inner_node;
                file->template read<inner_node>(*static_cast<inner_node *>(ptr), index);
            } else {
                ptr = new leaf_node;
                file->template read<leaf_node>(*static_cast<leaf_node *>(ptr), -index);
            }
        }
        BNodePtr(const BNodePtr &other) : file(other.file), refCount(other.refCount),
            ptr(other.ptr) {
            if (refCount) ++(*refCount);
        }
        BNodePtr(BNodePtr &&other) : file(other.file), refCount(other.refCount),
            ptr(other.ptr) {
            other.refCount = nullptr;
            other.ptr = nullptr;
            other.file = nullptr;
        }
        BNodePtr &operator=(const BNodePtr &other) {
            if (this == &other) return *this;
            if (refCount == other.refCount) return *this; // pointer to the same object
            clear();
            file = other.file;
            refCount = other.refCount;
            ptr = other.ptr;
            if (refCount) ++(*refCount);
            return *this;
        }
        void clear() {
            if (refCount) {
                --(*refCount);
                if (*refCount == 0) {
                    if (ptr->is_inner()) {
                        file->template update<inner_node>(*static_cast<inner_node *>(ptr),
                                                          ptr->get_index());
                        delete static_cast<inner_node *>(ptr);
                    } else {
                        file->template update<leaf_node>(*static_cast<leaf_node *>(ptr),
                                                         ptr->get_index());
                        delete static_cast<leaf_node *>(ptr);
                    }
                    delete refCount;
                }
            }
        }
        ~BNodePtr() {
            clear();
        }

        inner_node *as_inner() {
            return static_cast<inner_node *>(ptr);
        }

        leaf_node *as_leaf() {
            return static_cast<leaf_node *>(ptr);
        }

        node *operator->() {
            return ptr;
        }

    };


    int m_size; // size of the tree (number of the data)
    int m_root; // index of the root node
    File<2, FILE_BLOCK_SIZE> data_file;

    queue < int, MAX_CACHE_SIZE + 2 > cache;
    sjtu::map<int, BNodePtr> cache_map;

    pair<BNodePtr, int>
    path[100]; // path from root to leaf <index, pos>, 100 is enough
    int path_top;

  public:

    void init_cache() {
        cache.clear();
        cache_map.clear();
    }

    void clear_cache() {
    }

    BPlusTree(std::string data_file_name) : data_file(data_file_name) {
        static_assert(sizeof(inner_node) <= FILE_BLOCK_SIZE,
                      "inner_node is too large, please use smaller M");
        static_assert(sizeof(leaf_node) <= FILE_BLOCK_SIZE,
                      "leaf_node is too large, please use smaller L");
        if (!data_file.exist()) {
            data_file.init();
            // cerr << "init" << endl;
        } else {
            data_file.open();
            // cerr << "open" << endl;
        }
        data_file.get_info(m_root, 1);
        data_file.get_info(m_size, 2);
        // cerr << "root " << root << " size " << size << endl;
        init_cache();
    }

    ~BPlusTree() {
        data_file.write_info(m_root, 1);
        data_file.write_info(m_size, 2);
        clear_cache();
    }

  private:
    void shrink_cache() {
        while (cache.size() >= MAX_CACHE_SIZE) {
            int i = cache.front();
            cache.pop();
            if (cache_map.count(i)) {
                cache_map.erase(i);
            }
        }
    }

    BNodePtr get_node(int index) {
        if (cache_map.count(index)) {
            return cache_map.at(index);
        }
        shrink_cache();
        BNodePtr p(&data_file, index);
        cache.push(index);
        cache_map.insert({index, p});
        return p;
    }

    BNodePtr new_node(bool is_inner) {
        shrink_cache();
        BNodePtr p(&data_file, is_inner ? static_cast<node *>(new inner_node) :
                   static_cast<node *>(new leaf_node));
        p->set_index(data_file.write(), is_inner);
        cache.push(p->index);
        cache_map.insert({p->index, p});
        return p;
    }

    void remove_node(node *p) {
        // TODO: recycle the node
        cache_map.erase(p->index);
    }

    void insert_valchild(Key_t *key_list, int *child_list, int pos, int &count,
                         const Key_t &key,
                         const int &child) {
        for (int i = count - 1; i > pos; --i) {
            child_list[i + 1] = child_list[i];
            if (i != 0) key_list[i] = key_list[i - 1];
        }
        child_list[pos + 1] = child;
        if (pos >= 0) key_list[pos] = key;
        ++count;
    }

    void insert_valdata(Key_t *key_list, Data_t *data_list, int &count,
                        const Key_t &key,
                        const Data_t &data) {
        int i = count;
        while (i > 0 && Camp(key, key_list[i - 1]) < 0) {  // key_list[i - 1] > key
            key_list[i] = key_list[i - 1];
            data_list[i] = data_list[i - 1];
            --i;
        }
        key_list[i] = key;
        data_list[i] = data;
        ++count;
    }

    // insert into leaf node, if no split index = 0, if split index = new_leaf, upload_key is the minkey of new_leaf
    void leaf_node_insert(leaf_node *leaf, const Key_t &key, const Data_t &data,
                          int &index, Key_t &upload_key) {
        if (leaf->count < MAX_LEAF_SIZE) {
            // insert key and data (there's no case that key inserted in key[0])
            insert_valdata(leaf->key, leaf->data, leaf->count, key, data);
            index = 0;
        } else {
            // leaf is full, split it
            leaf_node *new_leaf = new_node(false).as_leaf();
            new_leaf->next = leaf->next;
            leaf->next = new_leaf->index;
            if (Camp(key, leaf->key[MIN_LEAF_SIZE - 1]) <=
                0) { // key <= leaf->key[MIN_LEAF_SIZE - 1]
                // insert key to the left node (leaf)
                    // for (int i = 0; i < MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE; ++i) {
                    //     new_leaf->key[i] = leaf->key[i + MIN_LEAF_SIZE - 1];
                    //     new_leaf->data[i] = leaf->data[i + MIN_LEAF_SIZE - 1];
                    // }
                quickcopy(new_leaf->key, leaf->key + MIN_LEAF_SIZE - 1, MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE);
                quickcopy(new_leaf->data, leaf->data + MIN_LEAF_SIZE - 1, MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE);

                leaf->count = MIN_LEAF_SIZE - 1;
                new_leaf->count = MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE;
                insert_valdata(leaf->key, leaf->data, leaf->count, key, data);
            } else {
                // insert key to the right node (new_leaf)
                    // for (int i = 0; i < MAX_LEAF_SIZE - MIN_LEAF_SIZE; ++i) {
                    //     new_leaf->key[i] = leaf->key[i + MIN_LEAF_SIZE];
                    //     new_leaf->data[i] = leaf->data[i + MIN_LEAF_SIZE];
                    // }
                quickcopy(new_leaf->key, leaf->key + MIN_LEAF_SIZE, MAX_LEAF_SIZE - MIN_LEAF_SIZE);
                quickcopy(new_leaf->data, leaf->data + MIN_LEAF_SIZE, MAX_LEAF_SIZE - MIN_LEAF_SIZE);
                leaf->count = MIN_LEAF_SIZE;
                new_leaf->count = MAX_LEAF_SIZE  - MIN_LEAF_SIZE;
                insert_valdata(new_leaf->key, new_leaf->data, new_leaf->count, key, data);
            }
            index = new_leaf->index;
            upload_key = new_leaf->key[0];
        }
    }

    // insert into inner node, if no split index = 0, if split index = new_node, upload_key is the minkey of new_node
    void inner_node_insert(inner_node *inner, int pos, const Key_t &key,
                           const int &child,
                           int &index, Key_t &upload_key) {
        if (inner->count < MAX_NODE_SIZE) {
            insert_valchild(inner->key, inner->child, pos, inner->count, key, child);
            index = 0;
        } else {
            inner_node *new_inner = new_node(true).as_inner();
            if (pos < MIN_NODE_SIZE - 1) {
                // insert key to the left node (inner)
                for (int i = 0; i < MAX_NODE_SIZE + 1 - MIN_NODE_SIZE; ++i) {
                    if (i != 0) new_inner->key[i - 1] = inner->key[i + MIN_NODE_SIZE - 2];
                    new_inner->child[i] = inner->child[i + MIN_NODE_SIZE - 1];
                }
                upload_key = inner->key[MIN_NODE_SIZE - 2];
                inner->count = MIN_NODE_SIZE - 1;
                new_inner->count = MAX_NODE_SIZE + 1 - MIN_NODE_SIZE;
                insert_valchild(inner->key, inner->child, pos, inner->count, key, child);
            } else if (pos == MIN_NODE_SIZE - 1)  {
                // insert key to the right node (new_inner)
                for (int i = 0; i < MAX_NODE_SIZE - MIN_NODE_SIZE; ++i) {
                    new_inner->key[i] = inner->key[i + MIN_NODE_SIZE - 1];
                    new_inner->child[i + 1] = inner->child[i + MIN_NODE_SIZE];
                }
                new_inner->child[0] = child;
                upload_key = key;
                inner->count = MIN_NODE_SIZE;
                new_inner->count = MAX_NODE_SIZE + 1 - MIN_NODE_SIZE;
            } else {
                // insert key to the right node (new_inner)
                for (int i = 0; i < MAX_NODE_SIZE - MIN_NODE_SIZE; ++i) {
                    if (i != 0) new_inner->key[i - 1] = inner->key[i + MIN_NODE_SIZE - 1];
                    new_inner->child[i] = inner->child[i + MIN_NODE_SIZE];
                }
                upload_key = inner->key[MIN_NODE_SIZE - 1];
                inner->count = MIN_NODE_SIZE;
                new_inner->count = MAX_NODE_SIZE - MIN_NODE_SIZE;
                insert_valchild(new_inner->key, new_inner->child, pos - MIN_NODE_SIZE,
                                new_inner->count, key, child);
            }
            index = new_inner->index;
            // upload_key = new_inner->key[0];
        }
    }

    bool leaf_merge_or_borrow(leaf_node *leaf, inner_node *father, int pos) {
        bool borrow = false;
        leaf_node *left_bro = nullptr, *right_bro = nullptr;
        if (pos > 0) { // try left
            left_bro = get_node(father->child[pos - 1]).as_leaf();
            if (left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            right_bro = get_node(father->child[pos + 1]).as_leaf();
            if (right_bro->count > MIN_LEAF_SIZE) {
                left_bro = nullptr;
                borrow = true;
            }
        }
        if (borrow) { // borrow
            if (left_bro) { // borrow from left bother
                for (int i = leaf->count; i > 0; --i) {
                    leaf->key[i] = leaf->key[i - 1];
                    leaf->data[i] = leaf->data[i - 1];
                }
                leaf->key[0] = left_bro->key[left_bro->count - 1];
                leaf->data[0] = left_bro->data[left_bro->count - 1];
                ++leaf->count;
                --left_bro->count;
                father->key[pos - 1] = leaf->key[0];
            } else { // borrow from right bother
                leaf->key[leaf->count] = right_bro->key[0];
                leaf->data[leaf->count] = right_bro->data[0];
                ++leaf->count;
                --right_bro->count;
                for (int i = 0; i < right_bro->count; ++i) {
                    right_bro->key[i] = right_bro->key[i + 1];
                    right_bro->data[i] = right_bro->data[i + 1];
                }
                father->key[pos] = right_bro->key[0];
            }
        } else { // merge
            if (left_bro) { // merge with left brother
                for (int i = 0; i < leaf->count; ++i) {
                    left_bro->key[left_bro->count + i] = leaf->key[i];
                    left_bro->data[left_bro->count + i] = leaf->data[i];
                }
                left_bro->count += leaf->count;
                left_bro->next = leaf->next;
                remove_node(leaf);
                for (int i = pos; i < father->count - 1; ++i) {
                    father->key[i - 1] = father->key[i];
                    father->child[i] = father->child[i + 1];
                }
                --father->count;
            } else { // merge with right brother
                for (int i = 0; i < right_bro->count; ++i) {
                    leaf->key[leaf->count + i] = right_bro->key[i];
                    leaf->data[leaf->count + i] = right_bro->data[i];
                }
                leaf->count += right_bro->count;
                leaf->next = right_bro->next;
                remove_node(right_bro);
                for (int i = pos + 1; i < father->count - 1; ++i) {
                    father->key[i - 1] = father->key[i];
                    father->child[i] = father->child[i + 1];
                }
                --father->count;
            }
        }
        return borrow;
    }

    bool inner_merge_or_borrow(inner_node *inner, inner_node *father, int pos) {
        bool borrow = false;
        inner_node *left_bro = nullptr, *right_bro = nullptr;
        if (pos > 0) { // try left
            left_bro = get_node(father->child[pos - 1]).as_inner();
            if (left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            right_bro = get_node(father->child[pos + 1]).as_inner();
            if (right_bro->count > MIN_LEAF_SIZE) {
                left_bro = nullptr;
                borrow = true;
            }
        }
        if (borrow) {
            if (left_bro) { // borrow from left bother
                for (int i = inner->count; i > 0; --i) {
                    if (i > 1) inner->key[i - 1] = inner->key[i - 2];
                    inner->child[i] = inner->child[i - 1];
                }
                inner->key[0] = father->key[pos - 1];
                inner->child[0] = left_bro->child[left_bro->count - 1];
                father->key[pos - 1] = left_bro->key[left_bro->count - 2];
                ++inner->count;
                --left_bro->count;
            } else { // borrow from right bother
                inner->key[inner->count - 1] = father->key[pos];
                inner->child[inner->count] = right_bro->child[0];
                father->key[pos] = right_bro->key[0];
                ++inner->count;
                --right_bro->count;
                for (int i = 0; i < right_bro->count; ++i) {
                    if (i + 1 < right_bro->count) right_bro->key[i] = right_bro->key[i + 1];
                    right_bro->child[i] = right_bro->child[i + 1];
                }
            }
        } else {
            if (left_bro) { // merge with left brother
                left_bro->key[left_bro->count - 1] = father->key[pos - 1];
                left_bro->child[left_bro->count] = inner->child[0];
                for (int i = 1; i < inner->count; ++i) {
                    left_bro->key[left_bro->count + i - 1] = inner->key[i - 1];
                    left_bro->child[left_bro->count + i] = inner->child[i];
                }
                left_bro->count += inner->count;
                remove_node(inner);
                for (int i = pos; i < father->count - 1; ++i) {
                    father->key[i - 1] = father->key[i];
                    father->child[i] = father->child[i + 1];
                }
                --father->count;
            } else { // merge with right brother
                inner->key[inner->count - 1] = father->key[pos];
                inner->child[inner->count] = right_bro->child[0];
                for (int i = 1; i < right_bro->count; ++i) {
                    inner->key[inner->count + i - 1] = right_bro->key[i - 1];
                    inner->child[inner->count + i] = right_bro->child[i];
                }
                inner->count += right_bro->count;
                remove_node(right_bro);
                for (int i = pos + 1; i < father->count - 1; ++i) {
                    father->key[i - 1] = father->key[i];
                    father->child[i] = father->child[i + 1];
                }
                --father->count;
            }
        }
        return borrow;
    }

    void print_node(int cur_index) {
        node *cur = get_node(cur_index);
        if (cur->is_inner()) {
            // if (cur_index != m_root) assert(cur->count >= MIN_NODE_SIZE);
            inner_node &x = *static_cast<inner_node *>(cur);
            std::cerr << "inner{ " << x.index << ", " << x.count << ": " << x.child[0] <<
                      " ";
            for (int i = 1; i < x.count; ++i) {
                std::cerr << "[" << x.key[i - 1] << "] " << x.child[i] << " ";
            }
            std::cerr << "}" << std::endl;
            for (int i = 0; i < x.count; ++i) {
                print_node(x.child[i]);
            }
        } else {
            // if (cur_index != m_root) assert(cur->count >= MIN_LEAF_SIZE);
            leaf_node &x = *static_cast<leaf_node *>(cur);
            std::cerr << "leaf{ " << x.index << ", " << x.count << ": ";
            for (int i = 0; i < x.count; ++i) {
                std::cerr << "[" << x.key[i] << "] " << x.data[i] << " ";
            }
            std::cerr << "}" << std::endl;
        }
    }

  public:
    void insert(const Key_t &key, const Data_t &data) {
        if (m_size == 0) {
            leaf_node *cur = new_node(false).as_leaf();
            m_root = cur->index;
            ++m_size;
            cur->count = 1;
            cur->key[0] = key;
            cur->data[0] = data;
            cur->next = 0;
            return;
        }
        ++m_size;
        path_top = -1;
        BNodePtr cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(cur.as_inner()->key[i], key) < 0)
                ++i;
            path[++path_top] = {cur, i};
            cur = get_node(cur.as_inner()->child[i]);
        }
        leaf_node *leaf = cur.as_leaf();
        int index; Key_t upload_key;
        leaf_node_insert(leaf, key, data, index, upload_key);
        while (path_top != -1 && index != 0) {
            auto kkey = upload_key;
            auto cchild = index;
            inner_node_insert(path[path_top].first.as_inner(),
                              path[path_top].second, kkey, cchild, index,
                              upload_key);
            --path_top;
        }
        if (index != 0) { // create new root
            inner_node *new_root = new_node(true).as_inner();
            new_root->count = 2;
            new_root->key[0] = upload_key;
            new_root->child[0] = m_root;
            new_root->child[1] = index;
            m_root = new_root->index;
        }
    }

    void search(const Key_t &key_L, const Key_t &key_R, Data_t *&res) {
        if (m_size == 0) return;
        BNodePtr cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(cur.as_inner()->key[i], key_L) <= 0) ++i;
            cur = get_node(cur.as_inner()->child[i]);
        }
        leaf_node *leaf = cur.as_leaf();
        while (true) {
            if (Camp(leaf->key[leaf->count - 1], key_L) < 0) {
                if (leaf->next == 0) break;
                leaf = get_node(leaf->next).as_leaf();
                continue;
            }
            int i = 0;
            while (i < leaf->count && Camp(leaf->key[i], key_L) < 0) ++i;
            while (i < leaf->count && Camp(leaf->key[i], key_R) <= 0) {
                // res.push(leaf->data[i]);
                *res = leaf->data[i];
                ++res;
                ++i;
            }
            if (i == leaf->count && leaf->next != 0) {
                leaf = get_node(leaf->next).as_leaf();
            } else {
                break;
            }
        }
    }

    void remove(const Key_t &key) {
        if (m_size == 0) return;
        path_top = -1;
        BNodePtr cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(cur.as_inner()->key[i], key) <= 0)
                ++i;
            path[++path_top] = {cur, i};
            cur = get_node(cur.as_inner()->child[i]);
        }
        leaf_node *leaf = cur.as_leaf();
        for (int i = 0; i < leaf->count; ++i) {
            if (leaf->key[i] == key) {
                for (int j = i; j < leaf->count - 1; ++j) {
                    leaf->key[j] = leaf->key[j + 1];
                    leaf->data[j] = leaf->data[j + 1];
                }
                --leaf->count;
                --m_size;
                if (i == 0) { // update the key in the inner node
                    for (int i = path_top; i >= 0; --i) {
                        if (path[i].second > 0) {
                            BNodePtr inner = path[i].first;
                            inner.as_inner()->key[path[i].second - 1] = leaf->key[0];
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (leaf->count < MIN_LEAF_SIZE) {
            // merge or borrow
            if (path_top == -1) {
                if (leaf->count == 0) { // size == 0
                    // assert(size == 0);
                    remove_node(leaf);
                    m_root = 0;
                }
                return;
            }
            if (leaf_merge_or_borrow(leaf,
                                     path[path_top].first.as_inner(),
                                     path[path_top].second)) return;
            while (path_top != -1) {
                inner_node *inner = path[path_top].first.as_inner();
                --path_top;
                if (inner->count < MIN_NODE_SIZE) {
                    if (path_top == -1) {
                        if (inner->count == 1) {
                            // assert(m_root == inner->get_index());
                            m_root = inner->child[0];
                            remove_node(inner);
                        }
                        return;
                    }
                    if (inner_merge_or_borrow(inner,
                                              path[path_top].first.as_inner(),
                                              path[path_top].second)) return;
                } else {
                    break;
                }
            }
        }
    }

    void debug() {
        std::cerr << "Tree size: " << m_size << std::endl;
        if (m_root) print_node(m_root);
        std::cerr << std::endl;
    }

    size_t size() const {
        return m_size;
    }

    bool empty() const {
        return size() == 0;
    }

};



typedef int Value_t;
typedef uint64_t Key_t;

template<uint64_t MOD = 1019260817, uint64_t BASE = 257>
class stringhash {
  public:
    uint64_t operator()(const std::string &str) const {
        uint64_t hash = 0;
        for (int i = 0; i < str.size(); ++i) {
            hash = (hash * BASE + str[i]) % MOD;
        }
        return hash;
    }
};

Value_t fuckyou[300000];

int main() {
    // BPlusTree<pair<Key_t, Value_t>, Value_t, 5, 5> bpt("data_file.db");
    BPlusTree<pair<Key_t, Value_t>, Value_t, 205, 204, 4096, 8000>
    bpt("data_file.db");
    std::cin.tie(0);
    std::cout.tie(0);
    std::ios::sync_with_stdio(0);
    int n; std::cin >> n;
    stringhash<> shash;
    for (int i = 1; i <= n; ++i) {
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "insert") {
            std::string key; Value_t value;
            std::cin >> key >> value;
            bpt.insert({shash(key), value}, value);
        } else if (cmd == "find") {
            std::string key;
            std::cin >> key;
            Value_t *ps = fuckyou;
            bpt.search({shash(key), -2147483648}, {shash(key), 2147483647}, ps);
            if (ps == fuckyou) {
                std::cout << "null" << std::endl;
            } else {
                for (Value_t *pp = fuckyou; pp != ps; ++pp) std::cout << *pp << " ";
                std::cout << std::endl;
            }
        } else if (cmd == "delete") {
            std::string key; Value_t value;
            std::cin >> key >> value;
            bpt.remove({shash(key), value});
        }
    }
    return 0;
}