/*
本题输入指令有以下三种情况：

insert [index] [value]

以 [index] 为索引，插入值为 [value] 的条目
delete [index] [value]

删除以 [index] 为索引，值为 [value] 的条目。请注意，删除的条目可能不存在
find [index]

找出所有索引为 [index] 的条目，并以 [value] 升序顺序输出，每个 [value] 间用一个空格隔开。若索引值为 [index] 的条目不存在，则输出 [null]。在整个指令输出结束后输出 \n
其中，[index] 为长度不超过 64 个字节的字符串(无空白字符)，[value] 为 int 范围内的非负整数。

数据中，[index] 和 [value] 均可能重复，但同一个 [index] 对应的 [value] 不可重复。
*/


#include <cstddef>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <list>

namespace acm {

template <class T>
int Camp(const T &x, const T &y) {
    return (x == y) ? 0 : (x < y ? -1 : 1);
}

int Camp(const char *x, const char *y) {
    return strcmp(x, y);
}


using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::ostream;


template<class T, int info_len = 2>
class File {
  private:
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
  public:
    File() = default;

    File(const string &file_name) : file_name(file_name) {}

    ~File() {
        file.close();
    }

    // init the file: create file if not exist or clear the file
    void init(string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out | std::ios::binary);
        int tmp = 0;
        for (int i = 0; i < info_len; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
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
    }

    //读出第 n 个 int 的值赋给 tmp，1_base
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        file.seekg(sizeof(int) * (n - 1));
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    //将 tmp 写入第 n 个 int 的位置，1_base
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        file.seekp(sizeof(int) * (n - 1));
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    // 在末尾写入
    int writenew(T &t) {
        int index;
        file.seekp(0, std::ios::end);
        index = file.tellp();
        index = (index - sizeof(int) * info_len) / sizeofT;
        file.write(reinterpret_cast<const char *>(&t), sizeofT);
        return index;
    }

    //用 t 的值更新位置索引 index 对应的对象，保证调用的 index 都是由 write 函数产生
    void update(T &t, const int index) {
        file.seekp(index * sizeofT + sizeof(int) * info_len);
        file.write(reinterpret_cast<const char *>(&t), sizeofT);
    }

    //读出位置索引 index 对应的 T 对象的值并赋值给 t，保证调用的 index 都是由 write 函数产生
    void read(T &t, const int index) {
        file.seekg(index * sizeofT + sizeof(int) * info_len);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
    }

    // 设置读指针
    void setread(const int index = 0) {
        file.seekg(index * sizeofT + sizeof(int) * info_len);
    }

    // 从当前位置读
    void read(T &t) {
        file.read(reinterpret_cast<char *>(&t), sizeofT);
    }

    // 设置写指针
    void setwrite(const int index = 0) {
        file.seekp(index * sizeofT + sizeof(int) * info_len);
    }

    // 从当前位置写入
    void write(T &t) {
        file.write(reinterpret_cast<const char *>(&t), sizeofT);
    }

};


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

// key - value blocklist database
typedef String<64> key_t;
typedef int value_t;
class blocklist {
  private:
    static const int BLOCK_SIZE = 256;
    static const int MAX_SIZE = 255;
    static const int MIN_SIZE = 64;
    static const int MERGE_SIZE = 200;

    struct pair_t {
        key_t key;
        value_t value;

        pair_t() {}
        pair_t(const key_t &_key, const value_t &_value) : key(_key), value(_value) {}
        pair_t(const pair_t &rhs) : key(rhs.key), value(rhs.value) {}

        ~pair_t() = default;

        pair_t &operator=(const pair_t &rhs) {
            if (this == &rhs) return *this;
            key = rhs.key;
            value = rhs.value;
            return *this;
        }

        bool operator<(const pair_t &rhs) const {
            return Camp(key, rhs.key) == 0 ? value < rhs.value : Camp(key, rhs.key) < 0;
        }

        bool operator==(const pair_t &rhs) const {
            return Camp(key, rhs.key) == 0 && value == rhs.value;
        }

        friend bool operator<(const key_t &lhs, const pair_t &rhs) {
            return Camp(lhs, rhs.key) < 0;
        }

        friend ostream &operator<<(ostream &os, const pair_t &obj) {
            os << obj.key << " : " << obj.value;
            return os;
        }
    };

    struct Nodehead {
        int index; // index of Nodedata in the file
        int count; // count of pairs in this node
        pair_t max_pair; // max pair in this node
    };

    // head 是存放 Nodehead 的链表， free_head 是空闲的 Nodehead 链表。
    std::list<Nodehead> head, free_head; // 嫌麻烦，还是 std::list 算了。
    using list_it = std::list<Nodehead>::iterator;

    struct Nodedata {
        pair_t data[BLOCK_SIZE];
        pair_t &operator[](const int &k) { return data[k]; }
    };

    Nodedata cache1, cache2; // 用于读写的缓存

    /* head_file 按顺序存放 Nodehead。其中 info 部分存两个 int: size, free_size。
       分别表示 使用中 Nodehead 数量与 空闲 Nodehead 数量。
       将内容读入后，在内存中以链表存储。
       data_file 存放 Nodedata 按照 head_file.index 索引。
    */
    File<Nodehead> head_file;
    File<Nodedata> data_file;


    // 创建一个新的 NodeData 并返回 index
    int newnode() {
        int index = 0;
        if (free_head.empty()) {
            index = data_file.writenew(
                        cache1); // 写入一个新的节点 Data, cache1 内容无所谓，只是预留空间。
        } else {
            index = free_head.back().index;
            free_head.pop_back();
        }
        return index;
    }

    // 删除一个节点，并回收
    void recyc(const list_it &it) {
        free_head.push_back(*it);
        head.erase(it);
    }

    // 找到 pair_t 所对应区块
    list_it findhead(const pair_t &pair) {
        list_it it = head.begin();
        while (it != head.end() && Camp(pair, it->max_pair) > 0) ++it;
        return it;
    }

    // 分裂
    bool split(const list_it &it) {
        if (it->count < MAX_SIZE) return false;
        list_it jt = head.insert(std::next(it), Nodehead{newnode(), 0, it->max_pair});
        int k = it->count / 2;
        memcpy(cache2.data, cache1.data + k, sizeof(pair_t) * (it->count - k));
        jt->count = it->count - k;
        it->count = k;
        it->max_pair = cache1[k - 1];
        return true;
    }
    
    // 合并
    void merge(const list_it &it) {
        if (it->count > MIN_SIZE) { }
        else if (it != head.begin() && it->count + std::prev(it)->count <= MERGE_SIZE) {
            list_it jt = std::prev(it);
            data_file.read(cache2, jt->index);
            memcpy(cache1.data + jt->count, cache1.data, sizeof(pair_t) * it->count);
            memcpy(cache1.data, cache2.data, sizeof(pair_t) * jt->count);
            it->count += jt->count;
            recyc(jt);
        } else if (std::next(it) != head.end()
                   && it->count + std::next(it)->count <= MERGE_SIZE) {
            list_it jt = std::next(it);
            data_file.read(cache2, jt->index);
            memcpy(cache1.data + it->count, cache2.data, sizeof(pair_t) * jt->count);
            it->count += jt->count;
            it->max_pair = jt->max_pair;
            recyc(jt);
        }
    }

  public:
    blocklist(const char *head_file_name, const char *data_file_name)
        : head_file(head_file_name), data_file(data_file_name) {
        if (!head_file.exist())  {
            // 创建新文件，写入一个空节点
            head_file.init();
            data_file.init();
            head_file.write_info(1, 1);
            head_file.write_info(0, 2);
            cache1[0] = pair_t("", 0);
            head.push_back(Nodehead{data_file.writenew(cache1), 1, cache1[0]});
        } else {
            // 从已有文件中读取 head, free_head
            head_file.open();
            data_file.open();
            int size, free_size;
            head_file.get_info(size, 1);
            head_file.get_info(free_size, 2);
            head.resize(size);
            free_head.resize(free_size);
            list_it it = head.begin();
            head_file.setread(0);
            for (int i = 0; i < size; ++i) {
                head_file.read(*it);
                ++it;
            }
            it = free_head.begin();
            for (int i = 0; i < free_size; ++i) {
                head_file.read(*it);
                ++it;
            }
        }
    }

    ~blocklist() {
        int size = head.size(), free_size = free_head.size();
        head_file.write_info(size, 1);
        head_file.write_info(free_size, 2);
        list_it it = head.begin();
        head_file.setwrite();
        for (int i = 0; i < size; ++i) {
            head_file.write(*it);
            ++it;
        }
        it = free_head.begin();
        for (int i = 0; i < free_size; ++i) {
            head_file.write(*it);
        }
    }

  public:
    void insert(const key_t &key, const value_t &value) {
        pair_t pair(key, value);
        list_it it = findhead(pair);
        if (it == head.end()) {
            --it;
            data_file.read(cache1, it->index);
            cache1[it->count++] = pair;
            it->max_pair = pair;
        } else {
            data_file.read(cache1, it->index);
            int k = 0;
            while (k < it->count && Camp(pair, cache1[k]) > 0) ++k;
            for (int i = it->count; i > k; --i) cache1[i] = cache1[i - 1];
            cache1[k] = pair;
            ++it->count;
        }
        if (split(it)) {
            data_file.update(cache1, it->index);
            data_file.update(cache2, std::next(it)->index);
        } else {
            data_file.update(cache1, it->index);
        }
    }

    void remove(const key_t &key, const value_t &value) {
        pair_t pair(key, value);
        list_it it = findhead(pair);
        if (it == head.end()) return;
        data_file.read(cache1, it->index);
        int k = 0;
        while (k < it->count && Camp(pair, cache1[k]) > 0) ++k;
        if (k == it->count || Camp(pair, cache1[k]) != 0) return; // 找不到键值对
        for (int i = k; i < it->count - 1; ++i) cache1[i] = cache1[i + 1];
        --it->count;
        merge(it);
        data_file.update(cache1, it->index);
    }

    void fetchall(const key_t &key, std::vector<int> &res) {
        res.clear();
        pair_t pair(key, 0);
        list_it it = findhead(pair);
        while (it != head.end()) {
            data_file.read(cache1, it->index);
            int k = 0;
            while (k < it->count && cache1[k].key < key) ++k;
            while (k < it->count && key == cache1[k].key) {
                res.push_back(cache1[k].value);
                ++k;
            }
            if (k == it->count && it->max_pair.key == key) {
                ++it;
            } else break;
        }
    }

};

}


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0); std::cout.tie(0);
    std::vector<int> tmp;
    acm::key_t key;
    acm::value_t value;
    acm::String<10> opt;
    acm::blocklist database("head.db", "data.db");
    int n; std::cin >> n;
    while (n--) {
        std::cin >> opt;
        if (opt == "insert") {
            std::cin >> key >> value;
            database.insert(key, value);
        } else if (opt == "delete") {
            std::cin >> key >> value;
            database.remove(key, value);
        } else if (opt == "find") {
            std::cin >> key;
            database.fetchall(key, tmp);
            if (tmp.empty()) {
                std::cout << "null" << std::endl;
            } else {
                for (int &i : tmp) {
                    std::cout << i << " ";
                }
                std::cout << std::endl;
            }
        }
    }
    return 0;
}