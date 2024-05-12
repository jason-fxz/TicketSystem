#include <iostream>
#include <string>
#include "utility.hpp"
#include "exceptions.hpp"
#include "String.hpp"
#include "File.hpp"
#include <map>

namespace sjtu {



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


template <typename Tp>
inline void quickcopy(Tp *dest, const Tp *src, size_t n) {
    memmove(dest, src, n * sizeof(Tp));
}

// B+ Tree database, Every Key should be unique!!
template < typename Key, typename Tp, size_t M, size_t L,
           size_t FILE_BLOCK_SIZE = 4096,
           size_t MAX_CACHE_SIZE = 1000,
           size_t MAX_NODE_SIZE = M,
           size_t MIN_NODE_SIZE = (M + 1) / 2,
           size_t MAX_LEAF_SIZE = L,
           size_t MIN_LEAF_SIZE = (L + 1) / 2 >
class BPlusTree {
    using Data_t = Tp;
    using Key_t = Key;
    using File_t = File<3, FILE_BLOCK_SIZE>;
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
        File_t *file;
        size_t *tag;
        node *ptr;

        int get_ref() {
            return tag ? (*tag & ((1u << 31) - 1)) : 0;
        }
        bool is_dirty() {
            return tag && (*tag & (1u << 31));
        }
        void inc_ref() {
            if (tag) ++(*tag);
        }
        void dec_ref() {
            if (tag) --(*tag);
        }

      public:
        BNodePtr() : file(nullptr), tag(nullptr), ptr(nullptr) {}
        BNodePtr(File_t *dbfile, node *_ptr) : file(dbfile),
            tag(new size_t(1)), ptr(_ptr) {}
        BNodePtr(File_t *dbfile, int index) : file(dbfile),
            tag(new size_t(1)) {
            if (index > 0) {
                ptr = new inner_node;
                file->template read<inner_node>(*static_cast<inner_node *>(ptr), index);
            } else {
                ptr = new leaf_node;
                file->template read<leaf_node>(*static_cast<leaf_node *>(ptr), -index);
            }
        }
        BNodePtr(const BNodePtr &other) : file(other.file), tag(other.tag),
            ptr(other.ptr) {
            if (tag) ++(*tag);
        }
        BNodePtr(BNodePtr &&other) : file(other.file), tag(other.tag),
            ptr(other.ptr) {
            other.tag = nullptr;
            other.ptr = nullptr;
            other.file = nullptr;
        }
        BNodePtr &operator=(const BNodePtr &other) {
            if (this == &other) return *this;
            if (tag == other.tag) return *this; // pointer to the same object
            clear();
            file = other.file;
            tag = other.tag;
            ptr = other.ptr;
            if (tag) ++(*tag);
            return *this;
        }
        void set_dirty() {
            if (tag) *tag |= 1u << 31;
        }
        void clear() {
            if (tag) {
                dec_ref();
                if (get_ref() == 0) {
                    if (ptr->is_inner()) {
                        if (is_dirty()) file->template update<inner_node>(*static_cast<inner_node *>
                                (ptr),
                                ptr->get_index());
                        delete static_cast<inner_node *>(ptr);
                    } else {
                        if (is_dirty()) file->template update<leaf_node>(*static_cast<leaf_node *>(ptr),
                                ptr->get_index());
                        delete static_cast<leaf_node *>(ptr);
                    }
                    delete tag;
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
    int m_recycle_head; // head of the recycle list
    File_t data_file;

    queue < int, MAX_CACHE_SIZE + 2 > cache;
    std::map<int, BNodePtr> cache_map;

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
        } else {
            data_file.open();
        }
        data_file.get_info(m_root, 1);
        data_file.get_info(m_size, 2);
        data_file.get_info(m_recycle_head, 3);
        init_cache();
    }

    ~BPlusTree() {
        data_file.write_info(m_root, 1);
        data_file.write_info(m_size, 2);
        data_file.write_info(m_recycle_head, 3);
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
        BNodePtr p;
        if (m_recycle_head != 0) {
            int tmp = m_recycle_head;
            p = get_node(is_inner ? m_recycle_head : -m_recycle_head);
            m_recycle_head = p->count;
            p->set_index(tmp, is_inner);
        } else {
            p = BNodePtr(&data_file, is_inner ? static_cast<node *>(new inner_node) :
                         static_cast<node *>(new leaf_node));
            p->set_index(data_file.write(), is_inner);
            cache.push(p->index);
            cache_map.insert({p->index, p});
        }
        p.set_dirty();
        return p;
    }

    void remove_node(node *p) {
        p->count = m_recycle_head;
        m_recycle_head = p->get_index();
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
                quickcopy(new_leaf->key, leaf->key + MIN_LEAF_SIZE - 1,
                          MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE);
                quickcopy(new_leaf->data, leaf->data + MIN_LEAF_SIZE - 1,
                          MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE);
                leaf->count = MIN_LEAF_SIZE - 1;
                new_leaf->count = MAX_LEAF_SIZE + 1 - MIN_LEAF_SIZE;
                insert_valdata(leaf->key, leaf->data, leaf->count, key, data);
            } else {
                // insert key to the right node (new_leaf)
                // for (int i = 0; i < MAX_LEAF_SIZE - MIN_LEAF_SIZE; ++i) {
                //     new_leaf->key[i] = leaf->key[i + MIN_LEAF_SIZE];
                //     new_leaf->data[i] = leaf->data[i + MIN_LEAF_SIZE];
                // }
                quickcopy(new_leaf->key, leaf->key + MIN_LEAF_SIZE,
                          MAX_LEAF_SIZE - MIN_LEAF_SIZE);
                quickcopy(new_leaf->data, leaf->data + MIN_LEAF_SIZE,
                          MAX_LEAF_SIZE - MIN_LEAF_SIZE);
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
        BNodePtr Left_bro, Right_bro;
        leaf_node *left_bro = nullptr, *right_bro = nullptr;
        if (pos > 0) { // try left
            Left_bro = get_node(father->child[pos - 1]);
            left_bro = Left_bro.as_leaf();
            // left_bro = get_node(father->child[pos - 1]).as_leaf();
            if (Left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            Right_bro = get_node(father->child[pos + 1]);
            right_bro = Right_bro.as_leaf();
            // right_bro = get_node(father->child[pos + 1]).as_leaf();
            if (Right_bro->count > MIN_LEAF_SIZE) {
                left_bro = nullptr;
                borrow = true;
            }
        }
        if (borrow) { // borrow
            if (left_bro) { // borrow from left bother
                Left_bro.set_dirty();
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
                Right_bro.set_dirty();
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
                Left_bro.set_dirty();
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
                Right_bro.set_dirty();
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
        BNodePtr Left_bro, Right_bro;
        inner_node *left_bro = nullptr, *right_bro = nullptr;
        if (pos > 0) { // try left
            Left_bro = get_node(father->child[pos - 1]);
            left_bro = Left_bro.as_inner();
            if (left_bro->count > MIN_LEAF_SIZE) {
                borrow = true;
            }
        }
        if (!borrow && pos + 1 < father->count) { // try right
            Right_bro = get_node(father->child[pos + 1]);
            right_bro = Right_bro.as_inner();
            if (right_bro->count > MIN_LEAF_SIZE) {
                left_bro = nullptr;
                borrow = true;
            }
        }
        if (borrow) {
            if (left_bro) { // borrow from left bother
                Left_bro.set_dirty();
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
                Right_bro.set_dirty();
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
                Left_bro.set_dirty();
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
                Right_bro.set_dirty();
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
        pair<BNodePtr, int>
        path[40]; // path from root to leaf <index, pos>, 40 is enough
        int path_top;
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
        cur.set_dirty();
        leaf_node *leaf = cur.as_leaf();
        int index; Key_t upload_key;
        leaf_node_insert(leaf, key, data, index, upload_key);
        while (path_top != -1 && index != 0) {
            auto kkey = upload_key;
            auto cchild = index;
            path[path_top].first.set_dirty();
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
        pair<BNodePtr, int>
        path[40]; // path from root to leaf <index, pos>, 40 is enough
        int path_top;
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
        cur.set_dirty();
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
                            inner.set_dirty();
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
            path[path_top].first.set_dirty();
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
                    path[path_top].first.set_dirty();
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


}

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

using namespace sjtu;
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