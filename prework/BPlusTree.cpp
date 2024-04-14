#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

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


// B+ Tree database, Every Key should be unique!!
template < typename Key, typename Tp, size_t M, size_t L,
           size_t FILE_BLOCK_SIZE = 4096 * 2,
           size_t MAX_CACHE_SIZE = 40,
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
        int get_index() const {
            return index < 0 ? -index : index;
        }
        bool is_inner() const {
            return index > 0;
        }
        void set_index(int _index, bool is_inner) {
            index = is_inner ? _index : -_index;
        }
        bool operator==(const node &rhs) const {
            return index == rhs.index;
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

    int m_size; // size of the tree (number of the data)
    int m_root; // index of the root node
    File<2, FILE_BLOCK_SIZE> data_file;

    node *cache[MAX_CACHE_SIZE];
    unsigned long long update_time[MAX_CACHE_SIZE];
    unsigned long long time_count;

  public:
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
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            cache[i] = nullptr;
            update_time[i] = 0;
        }
        time_count = 0;
    }

    ~BPlusTree() {
        data_file.write_info(m_root, 1);
        data_file.write_info(m_size, 2);
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            if (cache[i]) {
                write_node(cache[i]);
                if (cache[i]->is_inner()) delete static_cast<inner_node *>(cache[i]);
                else delete static_cast<leaf_node *>(cache[i]);
            }
        }
    }
  private:
    node *get_node(int index) {
        ++time_count;
        int pos = -1, mintimepos = -1;
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            if (cache[i] != nullptr && cache[i]->index == index) {
                // cerr << "get_node from cache " << i << " index " << index << endl;
                update_time[i] = time_count;
                return cache[i];
            }
            if (cache[i] == nullptr) {
                pos = i;
            } else {
                if (mintimepos == -1 || update_time[mintimepos] > update_time[i]) {
                    mintimepos = i;
                }
            }
        }
        if (pos == -1) {
            pos = mintimepos;
            write_node(cache[pos]);
            if (cache[pos]->is_inner()) delete static_cast<inner_node *>(cache[pos]);
            else delete static_cast<leaf_node *>(cache[pos]);
        }
        // cerr  << "get_node from disk index =  " << index << endl;
        if (index > 0) {
            cache[pos] = new inner_node;
            data_file.template read<inner_node>(*static_cast<inner_node *>(cache[pos]),
                                                index);
            // cerr  << "get inner node: " << cache[pos]->index << endl;
        } else {
            cache[pos] = new leaf_node;
            data_file.template read<leaf_node>(*static_cast<leaf_node *>(cache[pos]),
                                               -index);
            // cerr  << "get leaf node: " << cache[pos]->index << endl;
        }
        // cache[pos]->print();
        update_time[pos] = time_count;
        return cache[pos];
    }

    node *new_node(bool is_inner) {
        ++time_count;
        int pos = -1, mintimepos = -1;
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            if (cache[i] == nullptr) {
                pos = i;
                break;
            }
            if (mintimepos == -1 || update_time[mintimepos] > update_time[i]) {
                mintimepos = i;
            }
        }
        if (pos == -1) {
            pos = mintimepos;
            write_node(cache[pos]);
            if (cache[pos]->is_inner()) delete static_cast<inner_node *>(cache[pos]);
            else delete static_cast<leaf_node *>(cache[pos]);
        }
        if (is_inner) {
            cache[pos] = new inner_node;
        } else {
            cache[pos] = new leaf_node;
        }
        cache[pos]->set_index(data_file.write(), is_inner);
        update_time[pos] = time_count;
        return cache[pos];
    }

    void remove_node(node *p) {
        // do something recycle the node
        // TODO: recycle the node
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            if (cache[i] == p) {
                if (cache[i]->is_inner()) delete static_cast<inner_node *>(cache[i]);
                else delete static_cast<leaf_node *>(cache[i]);
                cache[i] = nullptr;
                return;
            }
        }
    }

    void write_node(node *p) {
        if (p->is_inner()) {
            // cerr  << "write inner node " << p->get_index() << endl;
            data_file.template update<inner_node>(*static_cast<inner_node *>(p),
                                                  p->get_index());
        } else {
            // cerr  << "write leaf node " << p->get_index() << endl;
            data_file.template update<leaf_node>(*static_cast<leaf_node *>(p),
                                                 p->get_index());
        }
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
            leaf_node *new_leaf = static_cast<leaf_node *>(new_node(false));
            new_leaf->next = leaf->next;
            leaf->next = new_leaf->index;
            if (Camp(key, leaf->key[MIN_LEAF_SIZE - 1]) <=
                0) { // key <= leaf->key[MIN_LEAF_SIZE - 1]
                // insert key to the left node (leaf)
                for (int i = 0; i < MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE; ++i) {
                    new_leaf->key[i] = leaf->key[i + MIN_LEAF_SIZE - 1];
                    new_leaf->data[i] = leaf->data[i + MIN_LEAF_SIZE - 1];
                }
                leaf->count = MIN_LEAF_SIZE - 1;
                new_leaf->count = MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE;
                insert_valdata(leaf->key, leaf->data, leaf->count, key, data);
            } else {
                // insert key to the right node (new_leaf)
                for (int i = 0; i < MAX_LEAF_SIZE - MIN_LEAF_SIZE; ++i) {
                    new_leaf->key[i] = leaf->key[i + MIN_LEAF_SIZE];
                    new_leaf->data[i] = leaf->data[i + MIN_LEAF_SIZE];
                }
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
            inner_node *new_inner = static_cast<inner_node *>(new_node(true));
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
            left_bro = static_cast<leaf_node *>(get_node(father->child[pos - 1]));
            if (left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            right_bro = static_cast<leaf_node *>(get_node(father->child[pos + 1]));
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
            left_bro = static_cast<inner_node *>(get_node(father->child[pos - 1]));
            if (left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            right_bro = static_cast<inner_node *>(get_node(father->child[pos + 1]));
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
            leaf_node *cur = static_cast<leaf_node *>(new_node(false));
            m_root = cur->index;
            ++m_size;
            cur->count = 1;
            cur->key[0] = key;
            cur->data[0] = data;
            cur->next = 0;
            return;
        }
        ++m_size;
        stack<pair<int, int>> path; // path from root to leaf <index, pos>
        node *cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(static_cast<inner_node *>(cur)->key[i], key) < 0)
                ++i;
            path.push({cur->index, i});
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        int index; Key_t upload_key;
        leaf_node_insert(leaf, key, data, index, upload_key);
        while (!path.empty() && index != 0) {
            auto kkey = upload_key;
            auto cchild = index;
            inner_node_insert(static_cast<inner_node *>(get_node(path.top().first)),
                              path.top().second, kkey, cchild, index,
                              upload_key);
            path.pop();
        }
        if (index != 0) { // create new root
            inner_node *new_root = static_cast<inner_node *>(new_node(true));
            new_root->count = 2;
            new_root->key[0] = upload_key;
            new_root->child[0] = m_root;
            new_root->child[1] = index;
            m_root = new_root->index;
        }
    }

    void search(const Key_t &key_L, const Key_t &key_R, Data_t *&res) {
        if (m_size == 0) return;
        node *cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(static_cast<inner_node *>(cur)->key[i], key_L) <= 0) ++i;
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        while (true) {
            if (Camp(leaf->key[leaf->count - 1], key_L) < 0) {
                if (leaf->next == 0) break;
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
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
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
            } else {
                break;
            }
        }
        // sort(res.begin(), res.end());
    }

    void remove(const Key_t &key) {
        if (m_size == 0) return;
        stack<pair<int, int>> path; // path from root to leaf <index, pos>
        node *cur = get_node(m_root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(static_cast<inner_node *>(cur)->key[i], key) <= 0)
                ++i;
            path.push({cur->index, i});
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        for (int i = 0; i < leaf->count; ++i) {
            if (leaf->key[i] == key) {
                for (int j = i; j < leaf->count - 1; ++j) {
                    leaf->key[j] = leaf->key[j + 1];
                    leaf->data[j] = leaf->data[j + 1];
                }
                --leaf->count;
                --m_size;
                if (i == 0) { // update the key in the inner node
                    for (auto it = path.begin(); it != path.end(); ++it) {
                        if (it->second > 0) {
                            inner_node *inner = static_cast<inner_node *>(get_node(it->first));
                            inner->key[it->second - 1] = leaf->key[0];
                            break;
                        }
                    }
                }
                break;
            }
        }
        if (leaf->count < MIN_LEAF_SIZE) {
            // merge or borrow
            if (path.empty()) {
                if (leaf->count == 0) { // size == 0
                    // assert(size == 0);
                    remove_node(leaf);
                    m_root = 0;
                }
                return;
            }
            if (leaf_merge_or_borrow(leaf,
                                     static_cast<inner_node *>(get_node(path.top().first)),
                                     path.top().second)) return;
            while (!path.empty()) {
                inner_node *inner = static_cast<inner_node *>(get_node(path.top().first));
                path.pop();
                if (inner->count < MIN_NODE_SIZE) {
                    if (path.empty()) {
                        if (inner->count == 1) {
                            // assert(m_root == inner->get_index());
                            m_root = inner->child[0];
                            remove_node(inner);
                        }
                        return;
                    }
                    if (inner_merge_or_borrow(inner,
                                              static_cast<inner_node *>(get_node(path.top().first)),
                                              path.top().second)) return;
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
    BPlusTree<pair<Key_t, Value_t>, Value_t, 205, 204, 4096> bpt("data_file.db");
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