#ifndef PINTOS_OPERATORS_H
#define PINTOS_OPERATORS_H

#include "functional.h"

namespace std_k {

template<typename T> struct equal_to {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left != right);
    }
};

template<typename T> struct not_equal_to {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left != right);
    }
};

template<typename T> struct greater {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left > right);
    }
};
template<typename T> struct less {
    less() {}
    constexpr bool operator()(const T& left, const T& right) const {
        return (left < right);
    }
};

template<typename T> struct greater_equal {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left >= right);
    }
};
template<typename T> struct less_equal {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left <= right);
    }
};

template<typename T> struct logical_and {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left && right);
    }
};
template<typename T> struct logical_or {
    constexpr bool operator()(const T& left, const T& right) const {
        return (left || right);
    }
};
template<typename T> struct logical_not {
    constexpr bool operator()(const T& left) const { return (!left); }
};

} // namespace std_k

#endif // PINTOS_OPERATORS_H
