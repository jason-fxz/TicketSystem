#ifndef STACK_HPP
#define STACK_HPP

#include <cstddef>

template <class Tp>
class Stack {
  private:
    struct node {
        Tp value;
        node *next;
    };
    size_t m_size;
    node *m_top;
  public:

    Stack() : m_size(0), m_top(nullptr) {}
    ~Stack() {
        while (m_top) {
            node *tmp = m_top;
            m_top = m_top->next;
            delete tmp;
        }
    }

    struct iterator {
        node *ptr;
        iterator(node *_ptr) : ptr(_ptr) {}
        Tp operator*() const {
            return ptr->value;
        }
        Tp *operator->() const {
            return &(ptr->value);
        }
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }
        bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }

        iterator &operator++() {
            ptr = ptr->next;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ptr = ptr->next;
            return tmp;
        }
    };

    iterator begin() {
        return iterator(m_top);
    }

    iterator end() {
        return iterator(nullptr);
    }

    Tp top() const {
        return m_top->value;
    }

    void push(const Tp &value) {
        node *tmp = new node{value, m_top};
        m_top = tmp;
        ++m_size;
    }
    void pop() {
        node *tmp = m_top;
        m_top = m_top->next;
        delete tmp;
        --m_size;
    }
    size_t size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
};

#endif // STACK_HPP