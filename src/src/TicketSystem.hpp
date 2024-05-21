#ifndef _TICKET_SYSTEM_HPP_
#define _TICKET_SYSTEM_HPP_

#include "File.hpp"
#include "User.hpp"
#include "utils.hpp"
#include "UserSystem.hpp"
#include "TrainSystem.hpp"

namespace sjtu {

class TicketSystem : command_helper, TrainSystem, UserSystem {
    void add_user() {
        if (UserSystem::add_user(arg('c'), arg('u'), arg('p'), arg('n'), arg('m'), arg('g'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void login() {
        if (UserSystem::login(arg('u'), arg('p'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void logout() {
        if (UserSystem::logout(arg('u'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void query_profile() {
        auto tmp = UserSystem::query_profile(arg('c'), arg('u'));
        if (tmp == nullptr) {
            puts("-1");
        } else {
            printf("%s %s %s %d\n", tmp->user.c_str(), tmp->name.c_str(), tmp->mail.c_str(), int(tmp->priv));
        }
    }

    void modify_profile() {
        auto tmp = UserSystem::modify_profile(arg('c'), arg('u'), arg('p'), arg('n'), arg('m'), arg('g'));
        if (tmp == nullptr) {
            puts("-1");
        } else {
            printf("%s %s %s %d\n", tmp->user.c_str(), tmp->name.c_str(), tmp->mail.c_str(), int(tmp->priv));
        }
    }

    void add_train() {
        if (TrainSystem::add_train(arg('i'), arg('n'), arg('m'), arg('s'), arg('p'), arg('x'), arg('t'), arg('o'), arg('d'),
                                   arg('y'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void delete_train() {
        if (TrainSystem::delete_train(arg('i'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void release_train() {
        if (TrainSystem::release_train(arg('i'))) {
            puts("0");
        } else {
            puts("-1");
        }
    }

    void query_train() {
        auto tmp = TrainSystem::query_train(arg('i'), arg('d'));
        if (std::get<0>(tmp) == nullptr) {
            puts("-1");
        } else {
            Train *train = std::get<0>(tmp);
            int *seats = std::get<1>(tmp);
            datetime_t date = std::get<2>(tmp);
            printf("%s %c\n", train->trainID.c_str(), train->type);
            for (int i = 0; i < train->stationNum; ++i) {
                datetime_t arri = (i == 0) ? 0 : date + train->arrivingTimes[i];
                datetime_t leav = (i == train->stationNum - 1) ? 0 : date + train->leavingTimes[i];
                printf("%s %s -> %s %d", train->stations[i].c_str(), arri.toString().c_str(), leav.toString().c_str(),
                       train->prices[i]);
                if (i != train->stationNum - 1) {
                    printf(" %d\n", seats[i]);
                } else {
                    puts(" x\n");
                }
            }
        }
    }

    vector<TrainPreview> res;
    void query_ticket() {
        res.clear();
        res.reserve(1145);
        TrainSystem::query_ticket(res, arg('s'), arg('t'), arg('d'), arg('p'));
        printf("%d\n", (int)res.size());
        for (int i = 0; i < res.size(); ++i) {
            printf("%s %s %s -> %s %s %d %d\n", res[i].trainID.c_str(), res[i].from.c_str(), res[i].leavingTime.toString().c_str(),
                   res[i].to.c_str(),
                   res[i].arrivingTime.toString().c_str(), res[i].price, res[i].seatCount);
        }
    }

    void query_transfer() {
        auto tmp = TrainSystem::query_transfer(arg('s'), arg('t'), arg('d'), arg('p'));
        if (tmp == nullptr) puts("0");
        else {
            printf("%s %s %s -> %s %s %d %d\n", tmp->trainID1.c_str(), tmp->from.c_str(), tmp->leavingTime1.toString().c_str(),
                   tmp->mid.c_str(),
                   tmp->arrivingTime1.toString().c_str(), tmp->price1, tmp->seatCount1);
            printf("%s %s %s -> %s %s %d %d\n", tmp->trainID2.c_str(), tmp->mid.c_str(), tmp->leavingTime2.toString().c_str(),
                   tmp->to.c_str(),
                   tmp->arrivingTime2.toString().c_str(), tmp->price2, tmp->seatCount2);
        }
    }

    void buy_ticket() { // -u -i -d -n -f -t (-q false)
        if (UserSystem::isLogin(arg('u')) == false) {
            puts("-1");
            return;
        }
        auto tmp = TrainSystem::buy_ticket(arg('u'), arg('i'), arg('d'), arg('n'), arg('f'), arg('t'), arg('q'));
        if (tmp.first == nullptr) {
            puts("-1");
            return;
        }
        UserSystem::addOrder(arg('u'), tmp.second);
        if (tmp.first->isPending()) {
            puts("queue");
        } else {
            printf("%lld\n", tmp.first->totcost());
        }
    }


    void query_order() {
        vector<int> OrderIndex;
        if (!UserSystem::query_order(OrderIndex, arg('u'))) {
            puts("-1");
            return;
        }
        reverse(OrderIndex.begin(), OrderIndex.end());
        vector<Order> Orders;
        TrainSystem::query_order(Orders, OrderIndex);
        printf("%d\n", (int)Orders.size());
        for (const auto &o : Orders) {
            printf("[%s] %s %s %s -> %s %s %d %d\n", orderState[o.state], o.trainID.c_str(),
                   o.from.c_str(), o.getLeavingDatetime().toString().c_str(),
                   o.to.c_str(), o.getArrivingDatetime().toString().c_str(), o.price, o.num);
        }
    }

    void refund_ticket() {
        int tmp = UserSystem::refund_ticket(arg('u'), arg('n'));
        if (tmp == -1) {
            puts("-1");
        } else {
            auto tt = TrainSystem::refund_ticket(tmp);
            if (tt) {
                puts("0");
            } else {
                puts("-1");
            }
        }
    }


  public:
    int NextCMD() {
        command_helper::process();
        switch (command_helper::getCMD()) {
        case CMD::AU: add_user(); break;
        case CMD::LI: login(); break;
        case CMD::LO: logout(); break;
        case CMD::QP: query_profile(); break;
        case CMD::MP: modify_profile(); break;
        case CMD::AT: add_train(); break;
        case CMD::DT: delete_train(); break;
        case CMD::RT: release_train(); break;
        case CMD::QT: query_train(); break;
        case CMD::QI: query_ticket(); break;
        case CMD::QR: query_transfer(); break;
        case CMD::BT: buy_ticket(); break;
        case CMD::QO: query_order(); break;
        case CMD::RI: refund_ticket(); break;
        case CMD::EX: puts("bye"); return 1;
        case CMD::CL: puts("0"); return 2;
        default: throw "WTF CMD?";
        }
        return 0;
    }


};

}


#endif