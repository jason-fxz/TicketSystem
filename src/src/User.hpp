#ifndef _USER_HPP_
#define _USER_HPP_

#include "String.hpp"
#include "utils.hpp"

namespace sjtu {


struct User {
    username_t  user;  ///< username
    password_t  pass;  ///< password
    name_t      name;  ///< name
    mailAddr_t  mail;  ///< mail address
    privilege_t priv;  ///< privilege
    
    

    User() = default;



    User(const username_t &user, const password_t &pass, const name_t &name, const mailAddr_t &mail, privilege_t priv)
        : user(user), pass(pass), name(name), mail(mail), priv(priv) {}


};


struct Order {
    username_t user;
    trainID_t trainID;
    stationName_t from;
    stationName_t to;
    datetime_t date;
    int leavingTime;
    int arrivingTime;
    number_t num;
    price_t price;
    int state; // 0: pending, 1: success, 2: refunded
    bool isPending() const {
        return state == 0;
    }
    bool isSuccess() const {
        return state == 1;
    }
    bool isRefunded() const {
        return state == 2;
    }

    long long totcost() const {
        return 1ll * price * num;
    }
    
    datetime_t getLeavingDatetime() const {
        return datetime_t(date.value + leavingTime);
    }
    datetime_t getArrivingDatetime() const {
        return datetime_t(date.value + arrivingTime);
    }

};

const char orderState[3][10] = {"pending", "success", "refunded"}; 
// enum class OrderState {
//     pending = 0,
//     success = 1,
//     refunded = 2
// };

} // namespace sjtu


#endif // _USER_HPP_