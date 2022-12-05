#ifndef PINT_HASH_TABLE_H
#define PINT_HASH_TABLE_H

#include "common.h"
#include "libk/asm.h"
#include "libstdc++/type_traits"
#include "misc.h"
#include "pair.h"

namespace std_k {

#define HASH_TABLE_DEFAULT_CAP       64
#define HASH_TABLE_DEFAULT_CAP_POWER 6;

template<typename key, typename value> class hash_table {
  public:
    hash_table() {
        slots = new pair<hash_type, value>[capacity];

        for (unsigned int i = 0; i < capacity; i++) { slots[i].first = 0; }
    }
    hash_table(size_t initial_capacity)
        : capacity(round_next_binary_power(initial_capacity))
        , capacity_power(log2(capacity)) {
        slots = new pair<hash_type, value>[capacity];

        for (unsigned int i = 0; i < capacity; i++) { slots[i].first = 0; }
    }

    void insert(pair<key, value> target) {
        hash_type target_hash  = calculate_hash(target.first);
        size_t    target_index = calculate_index(target_hash);

        while (slots[target_index].first != 0) {
            target_index++;
            collisions++;

            if (target_index == capacity) target_index = 0;
        }

        slots[target_index].first  = target_hash;
        slots[target_index].second = target.second;

        used_slots++;

        if ((used_slots > ((capacity) / capacity_power))
            || (collisions > (1 << (capacity_power - 2)))) {
            auto_rehash();
        }
    }

    value find(key target) {
        hash_type target_hash  = calculate_hash(target);
        size_t    target_index = calculate_index(target_hash);

        while (slots[target_index].first != target_hash) {

            if (slots[target_index].first == 0) return (value)0;

            target_index++;

            if (target_index == capacity) target_index = 0;
        }

        return slots[target_index].second;
    }

    void erase(key target) {
        hash_type target_hash  = calculate_hash(target);
        size_t    target_index = calculate_index(target_hash);

        while (slots[target_index].first != target_hash) {

            if (slots[target_index].first == 0) return;

            target_index++;
            collisions--;

            if (target_index == capacity) target_index = 0;
        }

        used_slots--;
        slots[target_index].first = 0;
    }

    void rehash(size_t new_capacity) {
        size_t                  old_capacity = capacity;
        pair<hash_type, value>* old_slots    = slots;

        capacity       = round_next_binary_power(new_capacity);
        capacity_power = log2(capacity);
        slots          = new pair<hash_type, value>[capacity];
        collisions     = 0;
        used_slots     = 0;

        for (size_t i = 0; i < old_capacity; i++) {
            if (old_slots[i].first != 0) { insert_hash(old_slots[i]); }
        }

        delete old_slots;
    }

  private:
    using hash_type = unsigned long int;

    hash_type calculate_hash(key target) {
        if constexpr (std::is_convertible<key, hash_type>::value) {
            return ((hash_type)target) * hash_modifier;
        } else if constexpr ((sizeof(key) >= sizeof(hash_type))
                             && ((sizeof(key) % sizeof(hash_type)) == 0)) {
            hash_type total;
            for (unsigned int i = 0; i < (sizeof(key) / sizeof(hash_type));
                 i++) {
                total += (((hash_type*)(&target))[i] * i);
            }

            return (total * hash_modifier);
        } else {
            hash_type total;
            for (unsigned int i = 0; i < sizeof(key); i++) {
                total += (((char*)(&target))[i] * i);
            }

            return (total * hash_modifier);
        }
    }

    size_t calculate_index(hash_type target) {
        unsigned int shift_power = 64 - capacity_power;

        target ^= (target >> shift_power);
        return ((hash_modifier * target) >> (shift_power));
    }

    void insert_hash(pair<hash_type, value> target) {
        size_t target_index = calculate_index(target.first);

        while (slots[target_index].first != 0) {
            target_index++;
            collisions++;

            if (target_index == capacity) target_index = 0;
        }

        slots[target_index].first  = target.first;
        slots[target_index].second = target.second;

        used_slots++;

        if ((used_slots > ((capacity) / capacity_power))
            || (collisions > (1 << (capacity_power - 2)))) {}
    }

    void auto_rehash() {
        size_t                  old_capacity = capacity;
        pair<hash_type, value>* old_slots    = slots;

        capacity *= 2;
        capacity_power++;
        slots      = new pair<hash_type, value>[capacity];
        collisions = 0;
        used_slots = 0;

        for (size_t i = 0; i < old_capacity; i++) {
            if (old_slots[i].first != 0) { insert_hash(old_slots[i]); }
        }

        delete old_slots;
    }

    pair<hash_type, value>* slots;
    hash_type               hash_modifier  = rd_seed() | 1;
    size_t                  used_slots     = 0;
    unsigned int            collisions     = 0;
    size_t                  capacity       = HASH_TABLE_DEFAULT_CAP;
    unsigned int            capacity_power = HASH_TABLE_DEFAULT_CAP_POWER;
};

} // namespace std_k

#endif
