#ifndef PINTOS_STRING_H
#define PINTOS_STRING_H

#include "cstring.h"
#include "vector.h"

namespace std_k {

class string : public vector<char> {
  public:
    string()
        : vector<char>(64) {}
    string(const char* initial_value) {
        resize(strlen(initial_value));
        strcpy(data(), initial_value);
    }

    operator char*() { return data(); }

    operator const char*() const { return data(); }

    void push_back(const char* target) {
        char* old_end = &back();

        resize(size() + strlen(target));
        strcpy(old_end, target);
    }
};

} // namespace std_k

#endif // PINTOS_STRING_H
