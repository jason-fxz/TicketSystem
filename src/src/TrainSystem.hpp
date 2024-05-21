#ifndef _TRAIN_SYSTEM_HPP_
#define _TRAIN_SYSTEM_HPP_

#include "Vector.hpp"
#include "utils.hpp"
#include "Train.hpp"
#include "User.hpp"
#include "File.hpp"
#include "BPlusTree.hpp"
#include "Vector.hpp"
#include <iterator>
#include <tuple>

namespace sjtu {

class TrainSystem {
    // BPlusTree<trainID_t, TrainState> TrainsStates; // trainID -> TrainState

    BPlusTree<size_t, TrainState> TrainsStates; // trainID_hash -> TrainState


    DataFile<Train> TrainsData; // TrainIndex -> Train
    DataFile<Seats> SeatsData;  // SeatIndex -> Seats
    BPlusTree<pair<size_t, int>, pair<int, int>> StationMap;  // stationName_hash -> (TrainIndex, SeatsIndex)
    BPlusTree<pair<TrainUnit, int>, int> TrainUnitMap; // TrainUnit -> OrderIndex
    DataFile<Order, sizeof(Order)> OrdersData; // OrderIndex -> Order

    Train tmpTrain;
    Seats tmpSeats;
    Transfer tmpTransfer;
    Order tmpOrder;

  public:
    TrainSystem() : TrainsStates("TrainsState"), SeatsData("SeatsData"),
        TrainsData("TrainsData"), StationMap("StationMap"), OrdersData("OrdersData"), TrainUnitMap("TrainUnitMap") {
    }

    ~TrainSystem() {
    }

    // [N] add_train -i -n -m -s -p -x -t -o -d -y
    bool add_train(const char *_i, const char *_n, const char *_m, const char *_s, const char *_p, const char *_x,
                   const char *_t, const char *_o, const char *_d, const char *_y) {
        size_t hash_i = string_hash(_i);
        if (TrainsStates.find(hash_i).second) return 0;
        std::memset(&tmpTrain, 0, sizeof(tmpTrain));
        tmpTrain.trainID = _i;
        tmpTrain.stationNum = atoi(_n);
        tmpTrain.seatNum = atoi(_m);
        getStrings(tmpTrain.stations, _s);
        getIntegers(tmpTrain.prices + 1, _p);
        tmpTrain.leavingTimes[0] = datetime_t(_x, 2).getTime();
        getIntegers(tmpTrain.arrivingTimes + 1, _t);
        getIntegers(tmpTrain.leavingTimes + 1, _o);;
        getDates(tmpTrain.salebeg, tmpTrain.saleend, _d);
        tmpTrain.type = _y[0];
        for (int i = 1; i < tmpTrain.stationNum; ++i) {
            tmpTrain.prices[i] += tmpTrain.prices[i - 1];
            tmpTrain.arrivingTimes[i] += tmpTrain.leavingTimes[i - 1];
            tmpTrain.leavingTimes[i] += tmpTrain.arrivingTimes[i];
        }
        TrainState tmpTrainState;
        tmpTrainState.seatIndex = -1;
        tmpTrainState.state = 0;
        tmpTrainState.trainIndex = TrainsData.write(tmpTrain);
        TrainsStates.insert(hash_i, tmpTrainState);
        return 1;
    }

    // [N] delete_train -i
    bool delete_train(const char *_i) {
        size_t hash_i = string_hash(_i);
        auto tmp = TrainsStates.find(hash_i);
        if (tmp.second == false) return 0;
        if (tmp.first.isReleased()) return 0;
        TrainsStates.remove(hash_i);
        return 1;
    }

    // [N] release_train -i
    bool release_train(const char *_i) {
        size_t hash_i = string_hash(_i);
        auto tmp = TrainsStates.find(hash_i);
        if (tmp.second == false) return 0;
        if (tmp.first.isReleased()) return 0;
        TrainsData.read(tmpTrain, tmp.first.trainIndex);
        tmp.first.state = 1;
        for (datetime_t i = tmpTrain.salebeg; i <= tmpTrain.saleend; i = i + 1) {
            for (int j = 0; j < tmpTrain.stationNum - 1; ++j) {
                tmpSeats.count[i.getDDate()][j] = tmpTrain.seatNum;
            }
        }
        tmp.first.seatIndex = SeatsData.write(tmpSeats);
        TrainsStates.modify(hash_i, tmp.first);
        // Puting the train into the StationMap
        for (int i = 0; i < tmpTrain.stationNum; ++i) {
            StationMap.insert(pair(string_hash(tmpTrain.stations[i]), tmp.first.trainIndex), pair(tmp.first.trainIndex, tmp.first.seatIndex));
        }
        // TODO : release train !!! OKOKOKOKOK
        return 1;
    }

    // [N] query_train -i -d
    std::tuple<Train *, int *, datetime_t> query_train(const char *_i, const char *_d) {
        size_t hash_i = string_hash(_i);
        auto tmp = TrainsStates.find(hash_i);
        if (tmp.second == false) {
            CERR("train %s not exist\n", _i);
            return std::make_tuple(nullptr, nullptr, 0);
        }
        TrainsData.read(tmpTrain, tmp.first.trainIndex);
        datetime_t departingDate = datetime_t(_d, 1);
        if (!tmpTrain.checkdate(departingDate)) {
            CERR("%s %s\n", tmpTrain.salebeg.toString().c_str(), tmpTrain.saleend.toString().c_str());
            CERR("date %s not in range\n", _d);
            return std::make_tuple(nullptr, nullptr, 0);
        }
        if (tmp.first.isReleased()) {
            SeatsData.read(tmpSeats, tmp.first.seatIndex);
        } else {
            for (int j = 0; j < tmpTrain.stationNum - 1; ++j) {
                tmpSeats.count[departingDate.getDDate()][j] = tmpTrain.seatNum;
            }
        }
        return std::make_tuple(&tmpTrain, tmpSeats.count[departingDate.getDDate()], departingDate);
    }


    // [SF] query_ticket -s -t -d (-p time)
    vector<TrainPreview> query_ticket(const char *_s, const char *_t, const char *_d, const char *_p) {
        vector<TrainPreview> res;
        datetime_t departingDate = datetime_t(_d, 1) + datetime_t("23:59", 2);
        vector<pair<int, int>> indexs;
        auto hash_s = string_hash(_s);
        auto hash_t = string_hash(_t);
        StationMap.search(pair(hash_s, 0), pair(hash_s, 0x3f3f3f3f), indexs);
        for (auto index : indexs) {
            TrainsData.read(tmpTrain, index.first);
            SeatsData.read(tmpSeats, index.second);
            pair<int, int> stationIndex = tmpTrain.GetStationIndex(_s, _t);
            if (stationIndex.first == -1 || stationIndex.second == -1 || stationIndex.first > stationIndex.second) continue;
            datetime_t train_dep = (departingDate - tmpTrain.leavingTimes[stationIndex.first]);
            train_dep.remainDate();
            CERR("train_dep = %s\n", train_dep.toString().c_str());
            if (!tmpTrain.checkdate(train_dep)) continue;
            TrainPreview tmp;
            tmp.trainID = tmpTrain.trainID;
            tmp.from = _s;
            tmp.to = _t;
            tmp.leavingTime = train_dep + tmpTrain.leavingTimes[stationIndex.first];
            tmp.arrivingTime = train_dep + tmpTrain.arrivingTimes[stationIndex.second];
            tmp.price = tmpTrain.prices[stationIndex.second] - tmpTrain.prices[stationIndex.first];
            tmp.seatCount = 0x3f3f3f3f;
            for (int i = stationIndex.first; i < stationIndex.second; ++i) {
                tmp.seatCount = std::min(tmp.seatCount, tmpSeats.count[train_dep.getDDate()][i]);
            }
            res.push_back(tmp);
        }
        if (_p != nullptr && _p[0] == 'c') { // by cost
            sort(res.begin(), res.end(), [&](const TrainPreview & a, const TrainPreview & b) {
                return a.price != b.price ? a.price < b.price : a.trainID < b.trainID;
            });
        } else { // by time
            sort(res.begin(), res.end(), [&](const TrainPreview & a, const TrainPreview & b) {
                return (a.arrivingTime - a.leavingTime) != (b.arrivingTime - b.leavingTime) ?
                       (a.arrivingTime - a.leavingTime) < (b.arrivingTime - b.leavingTime) : a.trainID < b.trainID;
            });
        }
        return std::move(res);
    }

    // [N] query_transfer -s -t -d (-p time)
    Transfer *query_transfer(const char *_s, const char *_t, const char *_d, const char *_p) {
        datetime_t departingDate = datetime_t(_d, 1) + datetime_t("23:59", 2);
        tmpTransfer.mid = "";
        vector<pair<int, int>> indexs;
        vector<pair<int, int>> indexs2;
        Train tmpTrain2;
        Seats tmpSeats2;
        Transfer tttt;
        tttt.from = _s;
        tttt.to = _t;
        auto hash_s = string_hash(_s);
        StationMap.search(pair(hash_s, 0), pair(hash_s, 0x3f3f3f3f), indexs);
        for (auto index : indexs) {
            TrainsData.read(tmpTrain, index.first);
            SeatsData.read(tmpSeats, index.second);
            int begIndex = tmpTrain.GetStationIndex(_s);
            if (begIndex == -1) continue;
            datetime_t train1_dep = (departingDate - tmpTrain.leavingTimes[begIndex]);
            train1_dep.remainDate();
            if (!tmpTrain.checkdate(train1_dep)) continue;
            tttt.trainID1 = tmpTrain.trainID;
            tttt.leavingTime1 = train1_dep + tmpTrain.leavingTimes[begIndex];
            for (int i = begIndex + 1; i < tmpTrain.stationNum; ++i) {
                if (tmpTrain.stations[i] == _t) continue;
                tttt.arrivingTime1 = train1_dep + tmpTrain.arrivingTimes[i];
                // CERR("tttt.arrivingTime1 = %s\n", tttt.arrivingTime1.toString().c_str());
                tttt.price1 = tmpTrain.prices[i] - tmpTrain.prices[begIndex];
                tttt.seatCount1 = 0x3f3f3f3f;
                for (int j = begIndex; j < i; ++j) {
                    tttt.seatCount1 = std::min(tttt.seatCount1, tmpSeats.count[train1_dep.getDDate()][j]);
                }
                tttt.mid = tmpTrain.stations[i];
                indexs2.clear();
                auto hash_m = string_hash(tmpTrain.stations[i]);
                StationMap.search(pair(hash_m, 0), pair(hash_m, 0x3f3f3f3f), indexs2);
                for (auto index2 : indexs2) {
                    TrainsData.read(tmpTrain2, index2.first);
                    SeatsData.read(tmpSeats2, index2.second);
                    pair<int, int> stationIndex = tmpTrain2.GetStationIndex(tmpTrain.stations[i], _t);
                    if (stationIndex.first == -1 || stationIndex.second == -1 || stationIndex.first > stationIndex.second) continue;
                    if (tmpTrain2.trainID == tmpTrain.trainID) continue;
                    datetime_t train2_dep = (train1_dep + tmpTrain.arrivingTimes[i] - (tmpTrain2.leavingTimes[stationIndex.first] -
                                             tmpTrain2.leavingTimes[0]));
                    if (tmpTrain2.endsaletime() < train2_dep) {
                        continue;
                    }
                    if (train2_dep < tmpTrain2.begsaletime()) {
                        train2_dep = tmpTrain2.salebeg;
                    } else {
                        if (train2_dep.getTime() > tmpTrain2.leavingTimes[0]) {
                            train2_dep = train2_dep + 24 * 60;
                        }
                        train2_dep.remainDate();
                    }
                    tttt.trainID2 = tmpTrain2.trainID;
                    tttt.leavingTime2 = train2_dep + tmpTrain2.leavingTimes[stationIndex.first];
                    tttt.arrivingTime2 = train2_dep + tmpTrain2.arrivingTimes[stationIndex.second];
                    tttt.price2 = tmpTrain2.prices[stationIndex.second] - tmpTrain2.prices[stationIndex.first];
                    tttt.seatCount2 = 0x3f3f3f3f;
                    for (int j = stationIndex.first; j < stationIndex.second; ++j) {
                        tttt.seatCount2 = std::min(tttt.seatCount2, tmpSeats2.count[train2_dep.getDDate()][j]);
                    }
                    if (tmpTransfer.mid.empty()) {
                        tmpTransfer = tttt;
                    } else {
                        if (_p != nullptr && _p[0] == 'c') { // by cost
                            tmpTransfer = std::min(tmpTransfer, tttt, [&](const Transfer & a, const Transfer & b) {
                                if (a.cost() != b.cost()) return a.cost() < b.cost();
                                if (a.times() != b.times()) return a.times() < b.times();
                                if (a.trainID1 != b.trainID1) return a.trainID1 < b.trainID1;
                                return a.trainID2 < b.trainID2;
                            });
                        } else { // by time
                            tmpTransfer = std::min(tmpTransfer, tttt, [&](const Transfer & a, const Transfer & b) {
                                if (a.times() != b.times()) return a.times() < b.times();
                                if (a.cost() != b.cost()) return a.cost() < b.cost();
                                if (a.trainID1 != b.trainID1) return a.trainID1 < b.trainID1;
                                return a.trainID2 < b.trainID2;
                            });
                        }
                    }
                }
            }
        }
        if (tmpTransfer.mid.empty()) return nullptr;
        return &tmpTransfer;
    }


    // buy_ticket -u -i -d -n -f -t     (return an order, )
    pair<Order *, int> buy_ticket(const char *_u, const char *_i, const char *_d, const char *_n, const char *_f,
                                  const char *_t, const char *_q) {
        size_t hash_i = string_hash(_i);
        auto tmp = TrainsStates.find(hash_i);
        if (tmp.second == false) {
            CERR("train %s not exist\n", _i);
            return {nullptr, 0};
        }
        if (!tmp.first.isReleased()) {
            CERR("train %s not released\n", _i);
            return {nullptr, 0};
        }
        TrainsData.read(tmpTrain, tmp.first.trainIndex);
        SeatsData.read(tmpSeats, tmp.first.seatIndex);
        if (tmpTrain.seatNum < atoi(_n)) {
            CERR("train %s Not enough seatsc (ps: seatNum < Num)\n", _i);
            return {nullptr, 0};
        }
        pair<int, int> stationIndex = tmpTrain.GetStationIndex(_f, _t);
        if (stationIndex.first == -1 || stationIndex.second == -1 || stationIndex.first > stationIndex.second) {
            CERR("train %s stationErr\n", _i);
            return {nullptr, 0};
        }
        datetime_t train_dep = (datetime_t(_d, 1) + datetime_t("23:59", 2) - tmpTrain.leavingTimes[stationIndex.first]);
        train_dep.remainDate();
        if (!tmpTrain.checkdate(train_dep)) {
            CERR("train %s OutofDate\n", _i);
            return {nullptr, 0};
        }
        tmpOrder.trainID = _i;
        tmpOrder.date = train_dep;
        tmpOrder.from = _f;
        tmpOrder.to = _t;
        tmpOrder.num = atoi(_n);
        tmpOrder.price = tmpTrain.prices[stationIndex.second] - tmpTrain.prices[stationIndex.first];
        tmpOrder.state = 0;
        tmpOrder.user = _u;
        tmpOrder.leavingTime = tmpTrain.leavingTimes[stationIndex.first];
        tmpOrder.arrivingTime = tmpTrain.arrivingTimes[stationIndex.second];
        int seatCount = 0x3f3f3f3f;
        for (int i = stationIndex.first; i < stationIndex.second; ++i) {
            seatCount = std::min(seatCount, tmpSeats.count[train_dep.getDDate()][i]);
        }
        if (seatCount >= tmpOrder.num) {
            tmpOrder.state = 1;
            for (int i = stationIndex.first; i < stationIndex.second; ++i) {
                tmpSeats.count[train_dep.getDDate()][i] -= tmpOrder.num;
            }
            SeatsData.update(tmpSeats, tmp.first.seatIndex);
        } else { // not enough seats
            if (_q == nullptr || _q[0] == 'f') {
                CERR("train %s Not enough seats And user don't queue\n", _i);
                return {nullptr, 0};
            }
        }
        int orderIndex = OrdersData.write(tmpOrder);
        if (tmpOrder.isPending()) {
            TrainUnitMap.insert(pair(TrainUnit{tmp.first.trainIndex, train_dep}, orderIndex), orderIndex);
        }
        return {&tmpOrder, orderIndex};
    }

    void query_order(vector<Order> &res, vector<int> index) {
        for (auto i : index) {
            OrdersData.read(tmpOrder, i);
            res.push_back(tmpOrder);
        }
    }


    bool refund_ticket(int orderIndex) {
        OrdersData.read(tmpOrder, orderIndex);
        auto tmp = TrainsStates.find(string_hash(tmpOrder.trainID)).first;
        TrainsData.read(tmpTrain, tmp.trainIndex);
        SeatsData.read(tmpSeats, tmp.seatIndex);
        if (tmpOrder.isRefunded()) {
            CERR("Order has been refunded\n");
            return 0;
        } else if (tmpOrder.isPending()) {
            TrainUnitMap.remove(pair(TrainUnit{tmp.trainIndex, tmpOrder.date}, orderIndex));
            tmpOrder.state = 2;
            OrdersData.update(tmpOrder, orderIndex);
        } else {
            pair<int, int> stationIndex = tmpTrain.GetStationIndex(tmpOrder.from, tmpOrder.to);
            datetime_t train_dep = tmpOrder.date;
            for (int i = stationIndex.first; i < stationIndex.second; ++i) {
                tmpSeats.count[train_dep.getDDate()][i] += tmpOrder.num;
            }
            tmpOrder.state = 2;
            OrdersData.update(tmpOrder, orderIndex);
            // check if there are any pending orders
            vector<int> indexs;
            TrainUnitMap.search(pair(TrainUnit{tmp.trainIndex, train_dep}, 0), pair(TrainUnit{tmp.trainIndex, train_dep},
                                0x3f3f3f3f), indexs);
            for (auto idx : indexs) {
                OrdersData.read(tmpOrder, idx);
                if (tmpOrder.isPending()) {
                    stationIndex = tmpTrain.GetStationIndex(tmpOrder.from, tmpOrder.to);
                    int seatCount = 0x3f3f3f3f;
                    for (int i = stationIndex.first; i < stationIndex.second; ++i) {
                        seatCount = std::min(seatCount, tmpSeats.count[train_dep.getDDate()][i]);
                    }
                    if (seatCount < tmpOrder.num) continue;
                    for (int i = stationIndex.first; i < stationIndex.second; ++i) {
                        tmpSeats.count[train_dep.getDDate()][i] -= tmpOrder.num;
                    }
                    tmpOrder.state = 1;
                    OrdersData.update(tmpOrder, idx);
                    TrainUnitMap.remove(pair(TrainUnit{tmp.trainIndex, train_dep}, idx));
                } else {
                    CERR("WTF? Order in queue is not pending?\n");
                    throw;
                }
            }
            SeatsData.update(tmpSeats, tmp.seatIndex);
        }
        return 1;
    }
};


} // namespace sjtu


#endif // _TRAIN_SYSTEM_HPP_