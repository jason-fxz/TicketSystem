#ifndef _USER_SYSTEM_HPP
#define _USER_SYSTEM_HPP

#include "String.hpp"
#include "TrainSystem.hpp"
#include "utils.hpp"
#include "User.hpp"
#include "BPlusTree.hpp"
#include "Hashmap.hpp"
#include "File.hpp"
#include <cstddef>

namespace sjtu {

struct Empty {};

class UserSystem {
    HashMapFile<size_t, User, 20023> Users;
    Hashmap<size_t, Empty, 20023> loginUsers;
    BPlusTree<pair<size_t, int>, int, 4096, 200000> UserOrders;

    User tmpUser;

  public:
    UserSystem() : Users("Users"), UserOrders("UserOrders") {}

    ~UserSystem() {
    }


    // [N] add_user -c -u -p -n -m -g
    bool add_user(const char *_c, const char *_u, const char *_p, const char *_n, const char *_m, const char *_g) {
        size_t hash_c = string_hash(_c);
        size_t hash_u = string_hash(_u);
        tmpUser.user = _u;
        tmpUser.pass = _p;
        tmpUser.name = _n;
        tmpUser.mail = _m;
        tmpUser.priv = _g[1] ? 10 : _g[0] - '0';
        if (Users.empty()) [[unlikely]] {
            // first user
            tmpUser.priv = 10;
            Users.insert({hash_u, tmpUser});
            return 1;
        } else {
            if (Users.count(hash_u)) return 0;
            if (loginUsers.count(hash_c) == 0) return 0;
            if (Users[hash_c].priv <= tmpUser.priv) return 0;
            Users.insert({hash_u, tmpUser});
            return 1;
        }
    }

    // [F] login -u -p
    bool login(const char *_u, const char *_p) {
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_u)) return 0;
        if (Users.count(hash_u) == false) return 0;
        if (Users[hash_u].pass != _p) return 0;
        loginUsers.insert(pair(hash_u, Empty()));
        return 1;
    }

    // [F] logout -u
    bool logout(const char *_u) {
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_u) == 0) return 0;
        loginUsers.erase(hash_u);
        return 1;
    }

    // [SF] query_profile -c -u
    User *query_profile(const char *_c, const char *_u) {
        size_t hash_c = string_hash(_c);
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_c) == 0) {
            CERR("user %s not login\n", _c);
            return nullptr;
        }
        if (Users.count(hash_u) == false) {
            CERR("user %s not exist\n", _u);
            return nullptr;
        }
        tmpUser = Users[hash_u];
        auto tmpc = Users[hash_c];
        if (tmpc.priv < tmpUser.priv || (tmpc.priv == tmpUser.priv && tmpc.user != tmpUser.user)) {
            CERR("user %s has no privilege to query user %s\n", _c, _u);
            return nullptr;
        }
        return &tmpUser;
    }

    // [F] modify_profile -c -u (-p) (-n) (-m) (-g)
    User *modify_profile(const char *_c, const char *_u, const char *_p, const char *_n, const char *_m, const char *_g) {
        size_t hash_c = string_hash(_c);
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_c) == 0) return nullptr;
        if (Users.count(hash_u) == 0) return nullptr;
        tmpUser = Users[hash_u];
        auto tmpc = Users[hash_c];
        if (tmpc.priv < tmpUser.priv || (tmpc.priv == tmpUser.priv && tmpc.user != tmpUser.user)) {
            CERR("user %s has no privilege to query user %s\n", _c, _u);
            return nullptr;
        }
        if (_p != nullptr) tmpUser.pass = _p;
        if (_n != nullptr) tmpUser.name = _n;
        if (_m != nullptr) tmpUser.mail = _m;
        if (_g != nullptr) tmpUser.priv = _g[1] ? 10 : _g[0] - '0';
        if (_g != nullptr && tmpUser.priv >= tmpc.priv) {
            CERR("-g can't be greater than or equal to %d\n", tmpc.priv);
            return nullptr;
        }
        Users[hash_u] = tmpUser;
        return &tmpUser;
    }

    // query_order -u
    bool query_order(vector<int> &res, const char *_u) {
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_u) == 0) return 0;
        UserOrders.search(pair(hash_u, 0), pair(hash_u, 0x3f3f3f3f), res);
        return 1;
    }


    bool isLogin(const char *_u) {
        size_t hash_u = string_hash(_u);
        return loginUsers.count(hash_u);
    }

    void addOrder(const char *_u, int OrderIndex) {
        size_t hash_u = string_hash(_u);
        UserOrders.insert(pair(hash_u, OrderIndex), OrderIndex);
    }

    // refund_ticket -u (-n 1)
    int refund_ticket(const char *_u, const char *_n) {
        size_t hash_u = string_hash(_u);
        if (loginUsers.count(hash_u) == 0) return -1;
        int idx = 1;
        if (_n != nullptr) idx = atoi(_n);
        vector<int> OrderIndex;
        UserOrders.search(pair(hash_u, 0), pair(hash_u, 0x3f3f3f3f), OrderIndex);
        if (idx > OrderIndex.size()) return -1;
        return *(OrderIndex.end() - idx);
    }




};

}





#endif // _USER_SYSTEM_HPP
