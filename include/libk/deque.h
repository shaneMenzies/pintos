#ifndef PINT_DEQUE_H
#define PINT_DEQUE_H

#include "common.h"

namespace std_k {

#define DEQUE_INITIAL_SIZE 64
template<class T> class deque {

  public:
    deque()
        : current_array(new T[DEQUE_INITIAL_SIZE])
        , front_index(0)
        , back_index(0)
        , current_capacity(DEQUE_INITIAL_SIZE) {};
    deque(size_t initial_capacity)
        : current_array(new T[DEQUE_INITIAL_SIZE])
        , front_index(0)
        , back_index(0)
        , current_capacity(initial_capacity) {};
    deque(T* initial_array, size_t initial_capacity)
        : current_array(initial_array)
        , front_index(0)
        , back_index(0)
        , current_capacity(initial_capacity) {};
    ~deque() {
        clear();
        if (current_array != nullptr) delete current_array;
    };

    T& back() { return current_array[back_index]; }
    T& front() { return current_array[front_index]; }

    T& operator[](size_t index) {
        index = (index + front_index) % current_capacity;

        return current_array[index];
    }
    const T& operator[](size_t index) const { return operator[](index); }

    T& at(size_t index) {
        index = (index + front_index) % current_capacity;
        if (index <= back_index)
            return current_array[index];
        else
            return T();
    }
    const T& at(size_t index) const { return at(index); }

    size_t begin() const { return 0; }
    size_t end() const {
        if (back_index < front_index)
            return ((current_capacity - front_index) + back_index);
        else
            return (back_index - front_index);
    }

    bool   empty() const { return (back_index == front_index); }
    size_t size() const { return (end() + 1); }
    size_t max_size() const { return current_capacity; }

    void push_back(T value);

    void pop_back();

    void push_front(T value);

    void pop_front();

    void clear();

    void recap(size_t new_capacity);

    deque& operator=(const deque& x);

    deque& operator=(deque&& x);

  private:
    T*     current_array = nullptr;
    size_t front_index   = 0;
    size_t back_index    = 0;
    size_t current_capacity;
};

template<class T> void deque<T>::push_back(T value) {
    if ((back_index + 1) >= current_capacity) {
        if (front_index > 0) {
            back_index = 0;
        } else {
            recap(current_capacity * 2);
            back_index++;
        }
    } else {
        back_index++;
    }

    current_array[back_index] = value;
}

template<class T> void deque<T>::pop_back() {
    if (!empty()) {
        current_array[back_index].~T();
        if (back_index == 0) {
            back_index = current_capacity - 1;
        } else {
            back_index--;
        }
    }
}

template<class T> void deque<T>::push_front(T value) {
    if (front_index == 0) {
        if (back_index == (current_capacity - 1)) {
            recap(current_capacity * 2);
        }
        front_index = current_capacity - 1;
    } else if ((front_index - 1) == back_index) {
        recap(current_capacity * 2);
        front_index--;
    } else {
        front_index--;
    }

    current_array[front_index] = value;
}

template<class T> void deque<T>::pop_front() {
    if (!empty()) {
        current_array[front_index].~T();
        if (front_index == (current_capacity - 1)) {
            front_index = 0;
        } else {
            front_index++;
        }
    }
}

template<class T> void deque<T>::clear() {
    while (front_index != back_index) {
        back().~T();
        pop_back();
    }
}

template<class T> void deque<T>::recap(size_t new_capacity) {
    if (new_capacity > current_capacity) {
        // TODO: Use realloc instead (then can be smarter about only moving
        // certain elements and indices)

        // Need new, larger array
        T* new_array     = new T[new_capacity];
        current_capacity = new_capacity;

        // Copy existing elements
        for (size_t i = 0; i < size(); i++) { new_array[i] = operator[](i); }

        // New indices will be based at 0 again
        back_index  = end();
        front_index = 0;

        delete current_array;
        current_array = new_array;

    } else if (new_capacity < current_capacity) {
        // Check how much smaller the new size is
        if ((new_capacity * 4) < current_capacity) {
            // Should reallocate to smaller array
            T* new_array     = new T[new_capacity];
            current_capacity = new_capacity;

            // Copy existing elements
            for (size_t i = 0; i < size(); i++) {
                new_array[i] = operator[](i);
            }

            // New indices will be based at 0 again
            back_index  = end();
            front_index = 0;

            delete current_array;
            current_array = new_array;
        }
    }
}

template<class T> deque<T>& deque<T>::operator=(const deque<T>& x) {
    if (x.current_capacity > current_capacity) {
        delete current_array;
        current_array    = new T[x.current_capacity];
        current_capacity = x.current_capacity;
    } else if ((x.current_capacity * 2) < current_capacity) {
        delete current_array;
        current_array    = new T[x.current_capacity];
        current_capacity = x.current_capacity;
    }
    front_index = 0;
    back_index  = x.end();

    for (size_t i = 0; i < x.size(); i++) { current_array[i] = x[i]; }

    return *this;
}

template<class T> deque<T>& deque<T>::operator=(deque<T>&& x) {
    current_array    = x.current_array;
    front_index      = x.front_index;
    back_index       = x.back_index;
    current_capacity = x.current_capacity;
    return *this;
}

} // namespace std_k

#endif
