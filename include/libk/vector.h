#ifndef PINT_VECTOR_H
#define PINT_VECTOR_H

#include "common.h"

namespace std_k {

#define VECTOR_INITIAL_SIZE 64

template<class T> class vector {
  public:
    vector()
        : current_size(0)
        , current_capacity(VECTOR_INITIAL_SIZE) {
        current_array = new T[VECTOR_INITIAL_SIZE];
    };
    vector(size_t initial_capacity)
        : current_size(0)
        , current_capacity(initial_capacity) {
        current_array = new T[initial_capacity];
    };
    vector(T* initial_array, size_t initial_capacity, size_t initial_size = 0)
        : current_array(initial_array)
        , current_size(initial_size)
        , current_capacity(initial_capacity) {}
    vector(const vector& source)
        : current_size(source.current_size)
        , current_capacity(source.current_capacity) {
        current_array = new T[current_capacity];
        for (size_t i = source.begin(); i < source.end(); i++) {
            current_array[i] = source[i];
        }
    }
    ~vector() {
        for (size_t i = 0; i < current_size; i++) { current_array[i].~T(); }
        delete current_array;
    };

    size_t           size() const { return current_size; };
    constexpr size_t max_size() const { return (size_t)(~0); };
    size_t           capacity() const { return current_capacity; };
    bool             empty() const { return (current_size == 0); };

    void resize(size_t new_size);

    T&       operator[](size_t index) { return current_array[index]; };
    const T& operator[](size_t index) const {
        return (const T&)current_array[index];
    };
    T& at(size_t index) {
        if (index < current_size)
            return current_array[index];
        else
            return current_array[0];
    };
    T& front() { return current_array[0]; };
    T& back() {
        if (current_size)
            return current_array[(current_size - 1)];
        else
            return current_array[0];
    };
    T*       data() { return current_array; };
    const T* data() const { return current_array; };

    size_t begin() const { return 0; }
    size_t end() const { return current_size; }

    vector& operator=(const vector& x);

    vector& operator=(vector&& x);

    void push_back(const T value);
    void pop_back();

    size_t insert(const size_t position, const T value);
    size_t erase(const size_t position);

    void clear();

  private:
    T* current_array;

    size_t current_size;
    size_t current_capacity;
};

template<class T> void vector<T>::resize(size_t new_size) {
    if (new_size > current_size) {
        // TODO: Use realloc instead

        // Check if within current capacity
        if (new_size >= current_capacity) {
            // Need new, larger array
            T* new_array     = new T[new_size * 2];
            current_capacity = new_size * 2;

            // Copy existing elements
            for (size_t i = 0; i < current_size; i++) {
                new_array[i] = current_array[i];
            }

            // Default-value initialize new elements
            for (size_t i = current_size; i < new_size; i++) {
                new_array[i] = T();
            }

            delete current_array;
            current_array = new_array;
        } else {
            // Can fit in current array
            // Default-value initialize new elements
            for (size_t i = current_size; i < new_size; i++) {
                current_array[i] = T();
            }
        }

        current_size = new_size;
    } else if (new_size < current_size) {
        // Check how much smaller the new size is
        if ((new_size * 4) < current_capacity) {
            // Should reallocate to smaller array
            T* new_array     = new T[new_size * 2];
            current_capacity = new_size * 2;

            // Copy non-changing elements
            for (size_t i = 0; i < new_size; i++) {
                new_array[i] = current_array[i];
            }

            // Destroy elements past new size
            for (size_t i = new_size; i < current_size; i++) {
                current_array[i].~T();
            }

            delete current_array;
            current_array = new_array;
        } else {
            // No change of capacity, just destroy elements past new size
            for (size_t i = new_size; i < current_size; i++) {
                current_array[i].~T();
            }
        }
        current_size = new_size;
    }
}

template<class T> vector<T>& vector<T>::operator=(const vector<T>& x) {
    if (x.current_capacity > current_capacity) {
        delete current_array;
        current_array    = new T[x.current_capacity];
        current_capacity = x.current_capacity;
    } else if ((x.current_capacity * 2) < current_capacity) {
        delete current_array;
        current_array    = new T[x.current_capacity];
        current_capacity = x.current_capacity;
    }

    for (size_t i = 0; i < x.current_size; i++) {
        current_array[i] = x.current_array[i];
    }

    current_size = x.current_size;
    return *this;
}

template<class T> vector<T>& vector<T>::operator=(vector<T>&& x) {
    current_array    = x.current_array;
    current_size     = x.current_size;
    current_capacity = x.current_capacity;
    return *this;
}

template<class T> void vector<T>::push_back(const T value) {
    if ((current_size + 1) > current_capacity) {
        // Reallocate to larger array
        T* new_array = new T[current_capacity * 2];
        current_capacity *= 2;

        // Copy existing elements
        for (size_t i = 0; i < current_size; i++) {
            new_array[i] = current_array[i];
        }
        new_array[current_size] = value;

        delete current_array;
        current_array = new_array;
    } else {
        // Can fit into current array
        current_array[current_size] = value;
    }

    current_size++;
}

template<class T> void vector<T>::pop_back() {
    if (current_size > 0) {
        current_size--;
        if ((current_size * 4) < current_capacity) {
            // Should reallocate to smaller array
            T* new_array     = new T[current_size * 2];
            current_capacity = current_size * 2;

            // Copy non-changing elements
            for (size_t i = 0; i < current_size; i++) {
                new_array[i] = current_array[i];
            }

            // Destroy last element
            current_array[current_size].~T();

            delete current_array;
            current_array = new_array;
        } else {
            // Destroy last element
            current_array[current_size].~T();
        }
    }
}

template<class T>
size_t vector<T>::insert(const size_t position, const T value) {
    if ((current_size + 1) > current_capacity) {
        // Reallocate to larger array
        current_capacity = (current_capacity ? current_capacity * 2 : 4);
        T* new_array     = new T[current_capacity];

        // Copy elements
        for (size_t i = 0; i < position; i++) {
            new_array[i] = current_array[i];
        }
        new_array[position] = value;
        for (size_t i = position; i < current_size; i++) {
            new_array[i + 1] = current_array[i];
        }

        delete current_array;
        current_array = new_array;
    } else {
        // Rearrange elements
        for (size_t i = 0; i < position; i++) {
            current_array[i] = current_array[i];
        }
        T previous              = current_array[position];
        current_array[position] = value;
        for (size_t i = position + 1; i <= current_size; i++) {
            T buffer         = current_array[i];
            current_array[i] = previous;
            previous         = buffer;
        }
    }

    current_size++;
    return position;
}

template<class T> size_t vector<T>::erase(const size_t position) {
    if (current_size > 0) {
        current_size--;
        if ((current_size * 4) < current_capacity) {
            // Should reallocate to smaller array
            T* new_array     = new T[current_size * 2];
            current_capacity = current_size * 2;

            // Copy elements
            for (size_t i = 0; i < position; i++) {
                new_array[i] = current_array[i];
            }
            current_array[position].~T();
            for (size_t i = position; i < current_size; i++) {
                new_array[i] = current_array[i + 1];
            }

            delete current_array;
            current_array = new_array;
        } else {
            // Rearrange elements
            current_array[position].~T();
            for (size_t i = position; i < current_size; i++) {
                current_array[i] = current_array[i + 1];
            }
        }
    }

    return ((position < current_size) ? position : (current_size - 1));
}

template<class T> void vector<T>::clear() {
    // Clear existing elements
    if (current_size) {
        // Destroy current array
        for (size_t i = 0; i < current_size; i++) { current_array[i].~T(); }
    }

    // Reset array if significantly larger than default
    if (current_capacity > (VECTOR_INITIAL_SIZE * 2)) {
        delete current_array;
        current_array    = new T[VECTOR_INITIAL_SIZE];
        current_capacity = VECTOR_INITIAL_SIZE;
    }
}

} // namespace std_k

#endif
