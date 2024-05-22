#include "String.hpp"
#include "utility.hpp"
#include "utils.hpp"

namespace sjtu {

struct Train {
    trainID_t trainID;

    number_t stationNum;
    number_t seatNum;

    stationName_t stations[maxSTATION]; // 0 ~ stationNum - 1 
    price_t prices[maxSTATION]; // prefix sum of prices (0 ~ stationNum - 1)

    mytime_t arrivingTimes[maxSTATION]; 
    mytime_t leavingTimes[maxSTATION];

    trainType_t type;
    datetime_t salebeg;
    datetime_t saleend;

    bool checkdate(datetime_t date) const {
        return salebeg <= date && date <= saleend;
    }

    datetime_t endsaletime() const {
        return saleend + leavingTimes[0];
    }
    datetime_t begsaletime() const {
        return salebeg + leavingTimes[0];
    }

    pair<int, int> GetStationIndex(const stationName_t &beg, const stationName_t &end) const {
        pair<int, int> res(-1, -1);
        for (int i = 0; i < stationNum; ++i) {
            if (stations[i] == beg) res.first = i;
            if (stations[i] == end) res.second = i;
        }
        return res;
    }
    int GetStationIndex(const stationName_t &name) const {
        for (int i = 0; i < stationNum; ++i) {
            if (stations[i] == name) return i;
        }
        return -1;
    }
};
static_assert(sizeof(Train) <= 4096, "Train size exceeds 4096 bytes");

struct TrainState {
    int seatIndex;
    int trainIndex;    
    bool state;

    bool isReleased() const {
        return state;
    }
};
static_assert(sizeof(TrainState), "");


struct TrainPreview {
    trainID_t trainID;
    // stationName_t from;
    // stationName_t to;
    // int trainIndex;
    datetime_t leavingTime;
    datetime_t arrivingTime;
    price_t price;
    number_t seatCount;
};

struct TrainLite {
    int trainIndex;
    int seatIndex;
    price_t price; // prefix sum of prices to the station
    int leavingTimes;
    int arrivingTimes;
    char salebegDD;
    char saleendDD;
    char pos; // 0 ~ stationNum - 1
    bool checkdate(datetime_t date) {
        return salebegDD <= date.getDDate() && date.getDDate() <= saleendDD;
    
    }
};

struct Transfer {
    trainID_t trainID1;
    trainID_t trainID2;
    stationName_t from;
    stationName_t mid;
    stationName_t to;
    datetime_t leavingTime1;
    datetime_t arrivingTime1;
    datetime_t leavingTime2;
    datetime_t arrivingTime2;
    price_t price1;
    price_t price2;
    number_t seatCount1;
    number_t seatCount2;

    int cost() const {
        return price1 + price2;
    }

    int times() const {
        return arrivingTime2 - leavingTime1;
    }
};

struct TrainUnit {
    int trainIndex;
    datetime_t date;
    bool operator<(const TrainUnit &rhs) const {
        return trainIndex != rhs.trainIndex ? trainIndex < rhs.trainIndex : date < rhs.date;
    }
    bool operator==(const TrainUnit &rhs) const {
        return trainIndex == rhs.trainIndex && date == rhs.date;
    }

};


struct Seats {
    seatinfo_t count[maxDURATION];
}; // 36800 / 36864



}