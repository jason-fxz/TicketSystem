/**
 * @file File.hpp
 * @author JasonFan (jasonfanxz@gmail.com)
 * @brief File class template
 * @version 1.0
 * @date 2024-05-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __FILE_HPP
#define __FILE_HPP

#include <cstddef>
#include <fstream>
#include <cstring>

/**
 * @brief A class template for file handling.
 * 
 * This class provides functionality for creating, opening, reading, and writing to files.
 * It supports storing intergers as head information and writing blocks of data to the file.
 * 
 * @tparam info_len The number of integers to store as information.
 * @tparam BLOCK_SIZE The size of each block in bytes. Default is 4096.
 */
template<int info_len, size_t BLOCK_SIZE = 4096>
class File {
  private:
    std::fstream file; /**< The file stream used for file operations. */
    std::string file_name; /**< The name of the file. */
    char buffer[BLOCK_SIZE]; /**< The buffer used for writing blocks to the file. */
    char infobuffer[info_len * sizeof(int)]; /**< The buffer used for storing information. */
  public:
    /**
     * @brief Default constructor.
     * 
     * Initializes the buffer and ensures that the size of the information buffer is within the block size limit.
     */
    File() {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    /**
     * @brief Constructor with file name parameter.
     * 
     * Initializes the buffer, sets the file name, and ensures that the size of the information buffer is within the block size limit.
     * 
     * @param file_name The name of the file.
     */
    File(const std::string &file_name) : file_name(file_name) {
        static_assert(info_len * sizeof(int) <= BLOCK_SIZE, "info_len is too large");
        memset(buffer, 0, sizeof(buffer));
    }

    /**
     * @brief Destructor.
     * 
     * Writes the information buffer to the file and closes the file stream.
     */
    ~File() {
        file.seekp(0);
        file.write(infobuffer, info_len * sizeof(int));
        file.close();
    }

    /**
     * @brief Initializes the file.
     * 
     * Creates a new file if it does not exist or clears the existing file.
     * If a file name is provided, it updates the file name.
     * 
     * @param FN The name of the file (optional).
     */
    void init(std::string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out | std::ios::binary);
        file.write(buffer, BLOCK_SIZE);
        file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        memset(infobuffer, 0, sizeof(infobuffer));
    }

    /**
     * @brief Checks if the file exists.
     * 
     * @return True if the file exists, false otherwise.
     */
    bool exist() {
        file.open(file_name, std::ios::in);
        bool res = file.is_open();
        file.close();
        return res;
    }

    /**
     * @brief Opens the file.
     * 
     * If a file name is provided, it updates the file name.
     * Opens the file in read and write mode and reads the information buffer.
     * 
     * @param FN The name of the file (optional).
     */
    void open(std::string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
        file.seekg(0);
        file.read(infobuffer, info_len * sizeof(int));
    }

    /**
     * @brief Retrieves the value of the nth integer in the information buffer.
     * 
     * The index is 1-based.
     * 
     * @param tmp The variable to store the retrieved value.
     * @param n The index of the integer to retrieve.
     */
    void get_info(int &tmp, int n) {
        if (n > info_len) return;
        char *p = infobuffer + (n - 1) * sizeof(int);
        memcpy(&tmp, p, sizeof(int));
    }

    /**
     * @brief Writes the value of tmp to the nth integer in the information buffer.
     * 
     * The index is 1-based.
     * 
     * @param tmp The value to write.
     * @param n The index of the integer to write.
     */
    void write_info(int tmp, int n) {
        if (n > info_len) return;
        char *p = infobuffer + (n - 1) * sizeof(int);
        memcpy(p, &tmp, sizeof(int));
    }

    /**
     * @brief Writes a block to the file and returns the index of the block.
     * 
     * The function seeks to the end of the file, writes the buffer to the file, and returns the index of the block.
     * 
     * @return The index of the block.
     */
    int write() {
        file.seekp(0, std::ios::end);
        int index = file.tellp() / BLOCK_SIZE;
        memset(buffer, 0, BLOCK_SIZE);
        file.write(buffer, BLOCK_SIZE);
        return index;
    }

    /**
     * @brief Writes the value of t to a block in the file and returns the index of the block.
     * 
     * @tparam T The type of the object.
     * @param t The value to write
     * @return The index of the block.
     */
    template<typename T>
    int write(const T &t) {
        file.seekp(0, std::ios::end);
        int index = file.tellp() / BLOCK_SIZE;
        memcpy(buffer, &t, sizeof(T));
        file.write(buffer, BLOCK_SIZE);
        return index;
    }

    /**
     * @brief Updates the object at the specified index with the value of t.
     * 
     * The index should be obtained from the write() function.
     * 
     * @tparam T The type of the object.
     * @param t The value to update the object with.
     * @param index The index of the object.
     */
    template<typename T>
    void update(T &t, const int index) {
        file.seekp(index * BLOCK_SIZE);
        file.write(reinterpret_cast<const char *>(&t), sizeof(T));
    }

    /**
     * @brief Reads the value of the object at the specified index and assigns it to t.
     * 
     * The index should be obtained from the write() function.
     * 
     * @tparam T The type of the object.
     * @param t The variable to store the retrieved value.
     * @param index The index of the object.
     */
    template<typename T>
    void read(T &t, const int index) {
        file.seekg(index * BLOCK_SIZE);
        file.read(reinterpret_cast<char *>(&t), sizeof(T));
    }

};


template<class Tp, size_t BLOCK_SIZE = (sizeof(Tp) + 4095) / 4096 * 4096>
class DataFile : public File<0, BLOCK_SIZE> {
    using FILE = File<0, BLOCK_SIZE>;
public:
    DataFile(const std::string &file_name) : FILE(file_name + ".dat") {
        if (FILE::exist()) {
            FILE::open();
        } else {
            FILE::init();
        }
    }
    ~DataFile() {
    }
    void read(Tp &t, const int index) {
        FILE::read(t, index);
    }
    void update(Tp &t, const int index) {
        FILE::update(t, index);
    }
    int write(const Tp &t) {
        return FILE::write(t);
    }
    int write() {
        return FILE::write();
    }


};


#endif // __FILE_HPP