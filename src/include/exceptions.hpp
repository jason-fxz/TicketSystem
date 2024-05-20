#ifndef __EXCEPTIONS_HPP
#define __EXCEPTIONS_HPP

#include <string>

namespace sjtu {

/**
 * @brief The exception class represents an exception in the program. (used in sjtu::STLite)
 *
 * 
 * This class provides a base class for defining custom exceptions in the program.
 * It contains a variant and detail member variables, and a what() function to retrieve the exception message.
 */
class exception {
  protected:
    const std::string variant = ""; ///< The variant of the exception.
    std::string detail = ""; ///< The detail of the exception.
  public:
    /**
     * @brief Default constructor for the exception class.
     */
    exception() {}

    /**
     * @brief Copy constructor for the exception class.
     * @param ec The exception object to be copied.
     */
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}

    /**
     * @brief Retrieves the exception message.
     * @return The exception message.
     */
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {
    /* __________________________ */
};

class runtime_error : public exception {
    /* __________________________ */
};

class invalid_iterator : public exception {
    /* __________________________ */
};

class container_is_empty : public exception {
    /* __________________________ */
};

}

#endif // __EXCEPTIONS_HPP