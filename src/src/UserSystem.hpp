#ifndef _USER_SYSTEM_HPP
#define _USER_SYSTEM_HPP

#include "utils.hpp"
#include "User.hpp"
#include "BPlusTree.hpp"
#include "Hashmap.hpp"

namespace sjtu {

struct Empty {};

class UserSystem {
    BPlusTree<username_t, User> Users;
    Hashmap<username_t, Empty, 20023> loginUsers;
    BPlusTree<pair<username_t, int>, int> UserOrders;

    User tmpUser;

public:
    UserSystem() : Users("Users"), UserOrders("UserOrders") {}
   
    ~UserSystem() {

    }


    // [N] add_user -c -u -p -n -m -g
    bool add_user(const char *_c, const char *_u, const char *_p, const char *_n, const char *_m, const char *_g) {
        tmpUser.user = _u;
        tmpUser.pass = _p;
        tmpUser.name = _n;
        tmpUser.mail = _m;
        tmpUser.priv = _g[1] ? 10 : _g[0] - '0';
        if (Users.empty()) [[unlikely]] {
            // first user
            tmpUser.priv = 10;
            Users.insert(tmpUser.user, tmpUser);
            return 1;
        } else {
            if (Users.find(tmpUser.user).second) return 0;
            if (loginUsers.count(_c) == 0) return 0;
            if (Users.find(_c).first.priv <= tmpUser.priv) return 0;
            Users.insert(tmpUser.user, tmpUser);
            return 1;
        }
    }

    // [F] login -u -p
    bool login(const char *_u, const char *_p) {
        if (loginUsers.count(_u)) return 0;
        auto tmp = Users.find(_u);
        if (tmp.second == false) return 0;
        if (tmp.first.pass != _p) return 0;
        loginUsers.insert(std::make_pair(_u, Empty()));
        return 1;
    }

    // [F] logout -u
    bool logout(const char *_u) {
        if (loginUsers.count(_u) == 0) return 0;
        loginUsers.erase(_u);
        return 1;
    }

    // [SF] query_profile -c -u
    User *query_profile(const char *_c, const char *_u) {
        if (loginUsers.count(_c) == 0) {
            CERR("user %s not login\n", _c);
            return nullptr;
        }
        auto tmp = Users.find(_u);
        tmpUser = tmp.first;
        if (tmp.second == false) {
            CERR("user %s not exist\n", _u);
            return nullptr;
        }
        auto tmpc = Users.find(_c);
        if (tmpc.first.priv < tmp.first.priv || (tmpc.first.priv == tmp.first.priv && tmpc.first.user != tmp.first.user)) {
            CERR("user %s has no privilege to query user %s\n", _c, _u);
            return nullptr;
        }
        return &tmpUser;
    }

    // [F] modify_profile -c -u (-p) (-n) (-m) (-g)
    User *modify_profile(const char *_c, const char *_u, const char *_p, const char *_n, const char *_m, const char *_g) {
        if (loginUsers.count(_c) == 0) return nullptr;
        auto tmp = Users.find(_u);
        auto tmpc = Users.find(_c);
        if (tmp.second == false) return nullptr;
        tmpUser = tmp.first;
        if (tmpc.first.priv < tmp.first.priv || (tmpc.first.priv == tmp.first.priv && tmpc.first.user != tmp.first.user)) {
            CERR("user %s has no privilege to query user %s\n", _c, _u);
            return nullptr;
        }
        if (_p != nullptr) tmpUser.pass = _p;
        if (_n != nullptr) tmpUser.name = _n;
        if (_m != nullptr) tmpUser.mail = _m;
        if (_g != nullptr) tmpUser.priv = _g[1] ? 10 : _g[0] - '0';
        if (_g != nullptr && tmpUser.priv >= tmpc.first.priv) {
            CERR("-g can't be greater than or equal to %d\n", tmpc.first.priv);
            return nullptr;
        }
        Users.modify(tmpUser.user, tmpUser);
        return &tmpUser;
    }

    // query_order -u
    bool query_order(vector<int> &res, const char *_u) {
        if (loginUsers.count(_u) == 0) return 0;
        UserOrders.search(pair(_u, 0), pair(_u, 0x3f3f3f3f), res);
        return 1;
    }


    bool isLogin(const char *_u) {
        return loginUsers.count(_u);
    }

    void addOrder(const char *_u, int OrderIndex) {
        UserOrders.insert(pair(_u, OrderIndex), OrderIndex);
    }

    // refund_ticket -u (-n 1)
    int refund_ticket(const char *_u, const char *_n) {
        if (loginUsers.count(_u) == 0) return -1;
        int idx = 1;
        if (_n != nullptr) idx = atoi(_n);
        vector<int> OrderIndex;
        UserOrders.search(pair(_u, 0), pair(_u, 0x3f3f3f3f), OrderIndex);
        if (idx > OrderIndex.size()) return -1;
        return *(OrderIndex.end() - idx);
    }




}; 
    
}





#endif // _USER_SYSTEM_HPP
