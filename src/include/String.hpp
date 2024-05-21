/**
 * @file String.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief a simple fix-length string class
 * @version 1.0
 * @date 2024-05-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __STRING_HPP
#define __STRING_HPP

#include <cstddef>
#include <iostream>
#include <cstring>

namespace sjtu {

/**
 * @brief a simple fix-length char[] string class
 *
 * @tparam len the length of the string
 */
template <size_t len>
class String {
  private:
    char data[len]; ///< the data of the string
  public:
    /**
     * @brief Construct a new String object
     *
     */
    String() { memset(data, 0, sizeof(data)); }
    /**
     * @brief Construct a new String object from a c-style string
     *
     * @param str
     */
    String(const char *str) {
        strcpy(data, str);
    }
    /**
     * @brief Construct a new String object from another String object
     *
     * @param other
     */
    String(const String &other) {
        memcpy(data, other.data, sizeof(data));
    }
    /**
     * @brief Destroy the String object
     *
     */
    ~String() = default;

    /**
     * @brief copy the data from another String object
     *
     * @param other
     * @return String&
     */
    String &operator=(const String &other) {
        if (this == &other) return *this;
        memcpy(data, other.data, sizeof(data));
        return *this;
    }

    /**
     * @brief assign the data from a c-style string
     *
     * @param str
     * @return String&
     */
    String &operator=(const char *str) {
        memset(data, 0, sizeof(data));
        strcpy(data, str);
        return *this;
    }

    /**
     * operator to check if two strings are equal
     */
    bool operator==(const String &rhs) const {
        return strcmp(data, rhs.data) == 0;
    }

    /**
     * operator to campare two strings in lexicographical order
     */
    bool operator<(const String &rhs) const {
        return strcmp(data, rhs.data) < 0;
    }

    /**
     * check if the string is empty
     */
    bool empty() const {
        return data[0] == '\0';
    }

    /**
     * assess the k-th character of the string
     * note that the index is 0-based
     * no boundary check!!
     * @param k the index of the character
     * @return the reference of the k-th character
     */
    char &operator[](const int &k) {
        return data[k];
    }

    /**
     * assess the k-th character of the string (const version)
     * note that the index is 0-based
     * no boundary check!!
     * @param k the index of the character
     * @return the reference of the k-th character
     */
    const char &operator[](const int &k) const {
        return data[k];
    }

    /**
     * get the c-style string
     */
    const char *c_str() const {
        return data;
    }


    /**
     * ostream operator to print the string
     */
    friend std::ostream &operator<<(std::ostream &os, const String &str) {
        os << str.data;
        return os;
    }

    /**
     * istream operator to read the string
    */
    friend std::istream &operator>>(std::istream &is, String &str) {
        is >> str.data;
        return is;
    }
};
const size_t RANDDOM_HASH_SEED = 114514998244353ll;
size_t string_hash(const char *str) {
    size_t hash = RANDDOM_HASH_SEED;
    for (int i = 0; str[i]; i++) {
        hash = hash * 131 + str[i];
    }
    return hash;
}

template <size_t len>
size_t string_hash(String<len> str) {
    size_t hash = RANDDOM_HASH_SEED;
    for (int i = 0; str[i]; i++) {
        hash = hash * 131 + str[i];
    }
    return hash;
}

} // namespace sjtu

namespace std {
/**
 * hash function for the String class
 */
template <size_t len>
struct hash<sjtu::String<len>> {
    size_t operator()(const sjtu::String<len> &str) const {
        size_t hash = sjtu::RANDDOM_HASH_SEED;
        for (int i = 0; i < len && str[i]; i++) {
            hash = hash * 131 + str[i];
        }
        return hash;
    }
};
}
#endif // __STRING_HPP