#ifndef PINT_QUEUE_H
#define PINT_QUEUE_H

#include "common.h"
#include "deque.h"

namespace std_k {

template<class T> class queue {

  public:
    queue()
        : base() {};
    queue(size_t initial_capacity)
        : base(initial_capacity) {};
    queue(T* initial_array, size_t initial_capacity)
        : base(initial_array, initial_capacity) {};
    ~queue() { base.~deque(); };

    T& front() { return base.front(); }
    T& back() { return base.back(); }

    bool   empty() const { return base.empty(); }
    size_t size() const { return base.size(); }

    void push(T value) { base.push_back(value); }
    void pop() { base.pop_front(); }

    void clear() { base.clear(); }

    queue& operator=(const queue& x) { base = (const deque<T>&)x.base; }

    queue& operator=(queue&& x) { base = (deque<T> &&) x.base; }

  private:
    deque<T> base;
};
} // namespace std_k

#endif
