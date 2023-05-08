#ifndef PINT_HEAP_H
#define PINT_HEAP_H

#include "common.h"
#include "operators.h"
#include "vector.h"

namespace std_k {

template<class T, class comparison = less<T>> class heap {

  public:
    T&   top();
    void pop_top();

    void push(T value);
    void replace_top(T new_value);

    const vector<T>& get_vec() const { return data; }
    vector<T>&       get_vec() { return data; }

    bool empty() { return data.empty(); }

  private:
    void down_sift(unsigned int start);
    void up_sift(unsigned int start);

    vector<T> data;
};

template<class T, class comparison> T& heap<T, comparison>::top() {
    return data[0];
}

template<class T, class comparison> void heap<T, comparison>::pop_top() {
    data[0] = data.back();
    data.pop_back();
    down_sift(0);
}

template<class T, class comparison> void heap<T, comparison>::push(T value) {
    data.push_back(value);
    up_sift(data.size() - 1);
}

template<class T, class comparison>
void heap<T, comparison>::replace_top(T new_value) {
    data[0] = new_value;
    down_sift(0);
}

template<class T, class comparison>
void heap<T, comparison>::down_sift(unsigned int start) {
    while (1) {
        unsigned int left  = (start * 2) + 1;
        unsigned int right = left + 1;

        if (right < data.size()) {
            unsigned int smaller
                = (comparison()(data[left], data[right]) ? left : right);

            if (comparison()(data[smaller], data[start])) {
                T buffer      = data[smaller];
                data[smaller] = data[start];
                data[start]   = buffer;
                start         = smaller;
            } else {
                break;
            }
        } else if (left < data.size()
                   && comparison()(data[left], data[start])) {
            T buffer    = data[left];
            data[left]  = data[start];
            data[start] = buffer;
            start       = left;
        } else {
            break;
        }
    }
}

template<class T, class comparison>
void heap<T, comparison>::up_sift(unsigned int start) {
    while (start > 0) {
        unsigned int parent = (start - 1) / 2;
        if (comparison()(data[start], data[parent])) {
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
