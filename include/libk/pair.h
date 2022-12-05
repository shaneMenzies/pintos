#ifndef PINT_PAIR_H
#define PINT_PAIR_H

namespace std_k {

template<typename A, typename B> struct pair {

    pair<A, B>() {}
    pair<A, B>(A first, B second)
        : first(first)
        , second(second) {}

    A first;
    B second;

    auto& operator[](size_t index) {
        if (index == 1) {
            return first;
        } else {
            return second;
        }
    }
};

} // namespace std_k

#endif