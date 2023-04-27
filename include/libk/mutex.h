#ifndef PINTOS_MUTEX_H
#define PINTOS_MUTEX_H

#include "common.h"

namespace std_k {

class mutex {
  private:
    bool locked;

  public:
    mutex()
        : locked(false) {}

    bool is_locked() { return __atomic_load_n(&locked, __ATOMIC_ACQUIRE); }

    void lock() {
        while (1) {
            if (try_lock()) {
                return;
            } else {
                asm volatile("nop");
            }
        }
    }

    bool try_lock() {
        bool expected = false;
        return __atomic_compare_exchange_n(&locked, &expected, true, false,
                                           __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
    }

    void unlock() { locked = false; }
};

class shared_mutex {
  private:
    bool exclusive_locked;
    int  shared_locks;

  public:
    shared_mutex()
        : exclusive_locked(false)
        , shared_locks(0) {}

    bool is_locked() {
        return __atomic_load_n(&exclusive_locked, __ATOMIC_ACQUIRE);
    }
    int get_shared_locks() {
        return __atomic_load_n(&shared_locks, __ATOMIC_ACQUIRE);
    }
    bool is_exclusive_locked() {
        return is_locked() && (get_shared_locks() == 0);
    }
    bool is_shared_locked() { return is_locked() && (get_shared_locks() != 0); }

    void lock() {
        while (1) {
            if (try_lock()) {
                return;
            } else {
                asm volatile("nop");
            }
        }
    }

    bool try_lock() {
        bool expected = false;
        return __atomic_compare_exchange_n(&exclusive_locked, &expected, true,
                                           false, __ATOMIC_ACQUIRE,
                                           __ATOMIC_ACQUIRE);
    }

    void unlock() {
        __atomic_store_n(&exclusive_locked, false, __ATOMIC_RELEASE);
    }

    void lock_shared() {
        while (!(is_shared_locked() || try_lock())) { asm volatile("nop"); }
        __atomic_add_fetch(&shared_locks, 1, __ATOMIC_ACQUIRE);
    }

    bool try_lock_shared() {
        if ((is_shared_locked() || try_lock())) {
            __atomic_add_fetch(&shared_locks, 1, __ATOMIC_ACQUIRE);
            return true;
        } else {
            return false;
        }
    }

    void unlock_shared() {
        __atomic_add_fetch(&shared_locks, -1, __ATOMIC_RELEASE);
        if (shared_locks == 0) { unlock(); }
    }
};

} // namespace std_k

#endif // PINTOS_MUTEX_H
