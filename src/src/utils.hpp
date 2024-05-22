#ifndef UTILS_HPP
#define UTILS_HPP

#include "utility.hpp"
#include "String.hpp"
#include <cstddef>
#include <type_traits>


#ifdef DEBUG
#define CERR(...) fprintf(stderr, __VA_ARGS__)
#else 
#define CERR(...)
#endif

namespace sjtu {





typedef String<21> username_t;
typedef String<31> password_t;
typedef String<16> name_t;
typedef String<32> mailAddr_t;
typedef char       privilege_t;


const int maxSTATION = 100;
const int maxDURATION = 92; // 6-1 ~ 8-31
typedef String<21> trainID_t;
typedef String<31> stationName_t;
typedef int number_t; // use for stationNum / seatNum
typedef int price_t;
typedef short mytime_t;
typedef char trainType_t;

typedef int seatinfo_t[maxSTATION];

// use for date / time, 0 ~ 366 * 24 * 60
struct datetime_t {
    // static constexpr int daysOfMonth[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
    static constexpr int daysOfMonth[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

    int value;
    datetime_t() = default;
    datetime_t(int value) : value(value) {}
    datetime_t(int month, int day, int hour, int minute) {
        value = (daysOfMonth[month - 1] + day - 1) * 24 * 60 + hour * 60 + minute;
    }
    // type=1 mm-dd ; type=2 hh:mm ; type=3 mm-dd hh:mm
    datetime_t(const char *str, int type) {
        int month = 1, day = 1, hour = 0, minute = 0;
        if (type == 1) sscanf(str, "%d-%d", &month, &day);
        else if (type == 2) sscanf(str, "%d:%d", &hour, &minute);
        else sscanf(str, "%d-%d %d:%d", &month, &day, &hour, &minute);
        value = (daysOfMonth[month - 1] + day - 1) * 24 * 60 + hour * 60 + minute;
    }

    std::string toString() const {
        if (value == 0) return "xx-xx xx:xx";
        int time = value % (24 * 60);
        int day = value / (24 * 60) + 1;
        int month = 1;
        while (daysOfMonth[month] < day) month++;
        day -= daysOfMonth[month - 1];
        char buf[32];
        sprintf(buf, "%02d-%02d %02d:%02d", month, day, time / 60, time % 60);
        return buf;
    }

    void remainDate() {
        value = value / (24 * 60) * (24 * 60);
    }

    int getDate() {
        return value / (24 * 60);
    }

    // From 06-01
    int getDDate() {
        return value / (24 * 60) - 152;
    }

    int getTime() {
        return value % (24 * 60);
    }

    bool operator<(const datetime_t &rhs) const {
        return value < rhs.value;
    }
    bool operator<=(const datetime_t &rhs) const {
        return value <= rhs.value;
    }
    bool operator!=(const datetime_t &rhs) const {
        return value != rhs.value;
    }
    bool operator==(const datetime_t &rhs) const {
        return value == rhs.value;
    }
    datetime_t operator+(const datetime_t &rhs) const {
        return datetime_t(value + rhs.value);
    }
    int operator-(const datetime_t &rhs) const {
        return value - rhs.value;
    }
};

template <class integer>
void getIntegers(integer *arr, const char *s) {
    static_assert(std::is_integral_v<integer>, "must be integer!");
    do {
        *arr = 0;
        while (*s && *s != '|') {
            *arr = ((*arr) << 3) + ((*arr) << 1) + ((*s) ^ 48);
            ++s;
        }
        ++arr;
    } while (*(s++));
}

template <class string>
void getStrings(string *arr, const char *s) {
    do {
        int i = 0;
        while (*s && *s != '|') {
            (*arr)[i++] = *s;
            ++s;
        }
        (*arr)[i] = '\0';
        ++arr;
    } while (*(s++));
}

template <class Datetime>
void getDates(Datetime &begin, Datetime &end, const char *s) {
    static_assert(std::is_same_v<Datetime, datetime_t>, "must be datetime_t!");
    begin = datetime_t(s, 1);
    end = datetime_t(s + 6, 1);
}

enum class CMD {
    AT, // add_train,
    RT, // release_train,
    DT, // delete_train,
    QT, // query_train,
    AU, // add_user,
    LI, // login,
    LO, // logout,
    QP, // query_profile,
    MP, // modify_profile,
    QI, // query_ticket,
    QR, // query_transfer,
    BT, // buy_ticket,
    QO, // query_order,
    RI, // refund_ticket,
    CL, // clean,
    EX, // exit
};



class command_helper {
    char buf[1024];
    char *ps[26];
    int index[50], cnt;
    char timestamp[32];

    void read_until(char *buf, char c) {
        int i = 0;
        while ((buf[i] = getchar()) != c && buf[i] != EOF) ++i;
        buf[i] = '\0';
    }
    void init() {
        int len = strlen(buf);
        cnt = 0;
        index[0] = 0;
        for (int i = 0, j = 0; i < len; i = j + 1) {
            while (j < len && buf[j] != ' ') ++j;
            buf[j] = '\0';
            index[cnt++] = i;
        }
        for (int i = 0; i < 26; ++i) {
            ps[i] = nullptr;
        }
        for (int i = 1; i < cnt; i += 2) {
            ps[buf[index[i] + 1] - 'a'] = buf + index[i + 1];
        }
    }

  public:
    void process() {
        read_until(timestamp, ' ');
        printf("%s ", timestamp);
        CERR("%s ", timestamp);
        read_until(buf, '\n');
        init();
    }

    bool isTimeStamp(const char *x) {
        return strcmp(x, timestamp) == 0;
    }

    const char *arg(char c) {
        return ps[c - 'a'];
    }

    CMD getCMD() {
        if (strcmp(buf, "add_train") == 0) return CMD::AT;
        if (strcmp(buf, "release_train") == 0) return CMD::RT;
        if (strcmp(buf, "delete_train") == 0) return CMD::DT;
        if (strcmp(buf, "query_train") == 0) return CMD::QT;
        if (strcmp(buf, "add_user") == 0) return CMD::AU;
        if (strcmp(buf, "login") == 0) return CMD::LI;
        if (strcmp(buf, "logout") == 0) return CMD::LO;
        if (strcmp(buf, "query_profile") == 0) return CMD::QP;
        if (strcmp(buf, "modify_profile") == 0) return CMD::MP;
        if (strcmp(buf, "query_ticket") == 0) return CMD::QI;
        if (strcmp(buf, "query_transfer") == 0) return CMD::QR;
        if (strcmp(buf, "buy_ticket") == 0) return CMD::BT;
        if (strcmp(buf, "query_order") == 0) return CMD::QO;
        if (strcmp(buf, "refund_ticket") == 0) return CMD::RI;
        if (strcmp(buf, "clean") == 0) return CMD::CL;
        if (strcmp(buf, "exit") == 0) return CMD::EX;
        // return CMD::EX;
        throw "Unknown command!";
    }




};






} // namespace sjtu


#endif
