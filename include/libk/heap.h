#ifndef PINT_HEAP_H
#define PINT_HEAP_H

#include "common.h"
#include "vector.h"

namespace std_k {

template<class T> class min_heap {

  public:
    T&   top();
    void pop_top();

    void push(T value);
    void replace_top(T new_value);

    vector<T>& get_vec() { return data; }

    bool empty() { return data.empty(); }

  private:
    void down_sift(unsigned int start);
    void up_sift(unsigned int start);

    vector<T> data;
};

template<class T> T& min_heap<T>::top() { return data[0]; }

template<class T> void min_heap<T>::pop_top() {
    data[0] = data.back();
    data.pop_back();
    down_sift(0);
}

template<class T> void min_heap<T>::push(T value) {
    data.push_back(value);
    up_sift(data.size() - 1);
}

template<class T> void min_heap<T>::replace_top(T new_value) {
    data[0] = new_value;
    down_sift(0);
}

template<class T> void min_heap<T>::down_sift(unsigned int start) {
    while (1) {
        unsigned int left  = (start * 2) + 1;
        unsigned int right = left + 1;

        if (right < data.size()) {
            unsigned int smaller = ((data[left] < data[right]) ? left : right);

            if (data[smaller] < data[start]) {
                T buffer      = data[smaller];
                data[smaller] = data[start];
                data[start]   = buffer;
                start         = smaller;
            } else {
                break;
            }
        } else if (left < data.size() && data[left] < data[start]) {
            T buffer    = data[left];
            data[left]  = data[start];
            data[start] = buffer;
            start       = left;
        } else {
            break;
        }
    }
}

template<class T> void min_heap<T>::up_sift(unsigned int start) {
    while (start > 0) {
        unsigned int parent = (start - 1) / 2;
        if (data[start] < data[parent]) {
            T buffer     = data[parent];
            data[parent] = data[start];
            data[start]  = buffer;
            start        = parent;
        } else {
            break;
        }
    }
}

} // namespace std_k

#endif
