#ifndef __STRING_HPP
#define __STRING_HPP

#include <cstddef>
#include <iostream>
#include <cstring>

namespace sjtu {


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
        return data[k];
    }

    char operator[](const int &k) const {
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

} // namespace sjtu

#endif // __STRING_HPP