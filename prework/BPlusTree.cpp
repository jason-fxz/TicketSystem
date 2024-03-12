#include <climits>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;


template <class Tp>
int Camp(const Tp &x, const Tp &y) {
    return x < y ? -1 : (x == y ? 0 : 1);
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
        static_assert(0 <= k && k < len, "String [] out of bound");
        return data[k];
    }

    char operator[](const int &k) const {
        static_assert(0 <= k && k < len, "String [] out of bound");
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
    fstream file;
    string file_name;
    char buffer[BLOCK_SIZE];
    char infobuffer[info_len * sizeof(int)];
  public:
    File() {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    File(const string &file_name) : file_name(file_name) {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    ~File() {
        file.seekp(0);
        file.write(infobuffer, info_len * sizeof(int));
        file.close();
    }

    // init the file: create file if not exist or clear the file
    void init(string FN = "") {
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
    void open(string FN = "") {
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




// B+ Tree database, Every Key should be unique!!
template <typename Key, typename Tp, size_t M, size_t L>
class BPlusTree {
#define MAX_NODE_SIZE (M)
#define MIN_NODE_SIZE ((M + 1) / 2)
#define MAX_LEAF_SIZE (L)
#define MIN_LEAF_SIZE ((L + 1) / 2)
#define MAX_CACHE_SIZE 40
    using Data_t = std::pair<Tp, bool>;
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
        // virtual void print() {
        //     cerr << "index " << index << " count " << count << endl;
        // }
        // virtual Key_t get_minkey() const = 0;
        // virtual ~node() {};
    };

    struct inner_node : node {
        Key_t key[MAX_NODE_SIZE - 1];
        int child[MAX_NODE_SIZE];
        // virtual Key_t get_minkey() const override {
        //     return key[0];
        // }
        // virtual ~inner_node() {};
        // virtual void print() override {
        //     node::print();
        //     cerr << child[0] << " ";
        //     for (int i = 1; i < node::count - 1; ++i) {
        //         cerr << "[" << key[i - 1] << "] " << child[i] << " ";
        //     }
        //     cerr << endl;
        // }

    };

    struct leaf_node : node {
        Key_t key[MAX_LEAF_SIZE];
        Data_t data[MAX_LEAF_SIZE];
        int next;
        // virtual Key_t get_minkey() const override {
        //     return key[0];
        // }
        // // virtual ~leaf_node() {};
        // virtual void print() override {
        //     node::print();
        //     for (int i = 0; i < node::count; ++i) {
        //         cerr << "[" << key[i] << "] " << data[i].first << "(" << data[i].second << ") ";
        //     }
        //     cerr << endl;
        // }
    };

    int size; // size of the tree (number of the data)
    int root; // index of the root node
    File<2> data_file;

    node *cache[MAX_CACHE_SIZE];
    unsigned long long update_time[MAX_CACHE_SIZE];
    unsigned long long time_count;

  public:
    BPlusTree(std::string data_file_name) : data_file(data_file_name) {
        static_assert(sizeof(inner_node) <= 4096,
                      "inner_node is too large, please use smaller M");
        static_assert(sizeof(leaf_node) <= 4096,
                      "leaf_node is too large, please use smaller L");
        if (!data_file.exist()) {
            data_file.init();
            // cerr << "init" << endl;
        } else {
            data_file.open();
            // cerr << "open" << endl;
        }
        data_file.get_info(root, 1);
        data_file.get_info(size, 2);
        // cerr << "root " << root << " size " << size << endl;
        for (int i = 0; i < MAX_CACHE_SIZE; ++i) {
            cache[i] = nullptr;
            update_time[i] = 0;
        }
        time_count = 0;
    }

    ~BPlusTree() {
        data_file.write_info(root, 1);
        data_file.write_info(size, 2);
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
            data_file.read<inner_node>(*static_cast<inner_node *>(cache[pos]), index);
            // cerr  << "get inner node: " << cache[pos]->index << endl;
        } else {
            cache[pos] = new leaf_node;
            data_file.read<leaf_node>(*static_cast<leaf_node *>(cache[pos]), -index);
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

    void write_node(node *p) {
        if (p->is_inner()) {
            // cerr  << "write inner node " << p->get_index() << endl;
            data_file.update<inner_node>(*static_cast<inner_node *>(p), p->get_index());
        } else {
            // cerr  << "write leaf node " << p->get_index() << endl;
            data_file.update<leaf_node>(*static_cast<leaf_node *>(p), p->get_index());
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
            if (Camp(key, leaf->key[MIN_LEAF_SIZE - 1]) <= 0) { // key <= leaf->key[MIN_LEAF_SIZE - 1]
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
  public:
    void insert(const Key_t &key, const Data_t &data) {
        // cerr << "insert " << key << " " << data << endl;
        // cerr << "root " << root << " size " << size << endl;
        if (size == 0) {
            leaf_node *cur = static_cast<leaf_node *>(new_node(false));
            root = cur->index;
            ++size;
            cur->count = 1;
            cur->key[0] = key;
            cur->data[0] = data;
            cur->next = 0;
            ++size;
            return;
        }
        ++size;
        std::vector<std::pair<int, int>> path; // path from root to leaf <index, pos>
        node *cur = get_node(root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1
                   && Camp(static_cast<inner_node *>(cur)->key[i], key) < 0)
                ++i;
            path.push_back({cur->index, i});
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        int index; Key_t upload_key;
        leaf_node_insert(leaf, key, data, index, upload_key);
        while (!path.empty() && index != 0) {
            auto kkey = upload_key;
            auto cchild = index;
            inner_node_insert(static_cast<inner_node *>(get_node(path.back().first)),
                              path.back().second, kkey, cchild, index,
                              upload_key);
            path.pop_back();
        }
        if (index != 0) { // create new root
            inner_node *new_root = static_cast<inner_node *>(new_node(true));
            new_root->count = 2;
            new_root->key[0] = upload_key;
            new_root->child[0] = root;
            new_root->child[1] = index;
            root = new_root->index;
            // cerr << "change root to " << root << endl;
        }
    }

    void search(const Key_t &key_L, const Key_t &key_R, std::vector<Tp> &res) {
        // cerr << "search " << key << endl;
        if (size == 0) return;
        node *cur = get_node(root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1 && Camp(static_cast<inner_node *>(cur)->key[i], key_L) <= 0) ++i;
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        while (true) {
            // cerr << "fuck" << std::endl;
            if (Camp(leaf->key[leaf->count - 1], key_L) < 0) {
                if (leaf->next == 0) break;
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
                continue;
            }
            int i = 0;
            while (i < leaf->count && Camp(leaf->key[i], key_L) < 0) ++i;
            while (i < leaf->count && Camp(leaf->key[i], key_R) <= 0) {
                if (leaf->data[i].second) res.push_back(leaf->data[i].first);
                ++i;
            }
            if (i == leaf->count && leaf->next != 0) {
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
            } else {
                break;
            }
        }
        sort(res.begin(), res.end());
    }

    void naive_remove(const Key_t &key) {
        if (size == 0) return;
        node *cur = get_node(root);
        while (cur->is_inner()) {
            int i = 0;
            while (i < cur->count - 1 && Camp(static_cast<inner_node *>(cur)->key[i], key) < 0) ++i;
            cur = get_node(static_cast<inner_node *>(cur)->child[i]);
        }
        leaf_node *leaf = static_cast<leaf_node *>(cur);
        while (true) {
            // cerr << "fuck" << std::endl;
            if (Camp(leaf->key[leaf->count - 1], key) < 0) {
                if (leaf->next == 0) break;
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
                continue;
            }
            int i = 0;
            while (i < leaf->count && Camp(leaf->key[i], key) < 0) ++i;
            while (i < leaf->count && Camp(leaf->key[i], key) == 0) {
                if (leaf->data[i].second == 1) {
                    leaf->data[i].second = 0;
                    break;
                }
                ++i;          
            }
            if (i == leaf->count && leaf->next != 0) {
                leaf = static_cast<leaf_node *>(get_node(leaf->next));
            } else {
                break;
            }
        }
    }

    void print_node(int cur_index) {
        node *cur = get_node(cur_index);
        if (cur->is_inner()) {
            inner_node &x = *static_cast<inner_node *>(cur);
            cerr << "inner{ " << x.index << ", " << x.count << ": " << x.child[0] << " ";
            for (int i = 1; i < x.count; ++i) {
                cerr << "[" << x.key[i - 1] << "] " << x.child[i] << " ";
            }
            cerr << "}" << endl;
            for (int i = 0; i < x.count; ++i) {
                print_node(x.child[i]);
            }
        } else {
            leaf_node &x = *static_cast<leaf_node *>(cur);
            cerr << "leaf{ " << x.index << ", " << x.count << ": ";
            for (int i = 0; i < x.count; ++i) {
                cerr << "[" << x.key[i] << "] " << x.data[i].first << "(" << x.data[i].second <<
                     ") ";
            }
            cerr << "}" << endl;
        }
    }

    void debug() {
        cerr << "Tree size: " << size << endl;
        if (root) print_node(root);
        cerr << endl;
    }

};
typedef int Value_t;
typedef String<55> Key_t;
int main() {
    // BPlusTree<Key_t, Value_t, 5, 5> bpt("data_file.db");
    BPlusTree<pair<Key_t, Value_t>, Value_t, 64, 60> bpt("data_file.db");
    cin.tie(0);
    cout.tie(0);
    std::ios::sync_with_stdio(0);
    int n; cin >> n;
    for (int i = 1; i <= n; ++i) {
        std::string cmd;
        std::cin >> cmd;
        if (cmd == "insert") {
            Key_t key; Value_t value;
            std::cin >> key >> value;
            bpt.insert({key, value}, {value, 1});
        } else if (cmd == "find") {
            Key_t key;
            std::cin >> key;
            std::vector<int> res;
            bpt.search({key, INT_MIN}, {key, INT_MAX}, res);
            if (res.empty()) {
                cout << "null" << endl;
            } else {
                for (auto &i : res) std::cout << i << " ";
                std::cout << std::endl;
            }
        } else if (cmd == "delete") {
            Key_t key; Value_t value;
            std::cin >> key >> value;
            bpt.naive_remove({key, value});
        }
        // cerr << "i = " << i << endl;
        // bpt.debug();
    }
    return 0;
}