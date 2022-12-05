#ifndef PINT_FUNCTIONAL_H
#define PINT_FUNCTIONAL_H

#include "callable.h"
#include "common.h"
#include "libk/asm.h"
#include "libstdc++/tuple"

namespace std_k {
template<typename T> class function;
template<typename R, typename... Args> class function<R(Args...)> {
  public:
    function() {}
    constexpr function(R (*new_target)(Args...)) { stored_target = new_target; }

    void     operator=(R (*new_target)(Args...)) { stored_target = new_target; }
    explicit operator bool() const { return (stored_target != nullptr); }

    constexpr R operator()(Args... arguments) const {
        return stored_target(arguments...);
    }
    constexpr R call(Args... arguments) const {
        return stored_target(arguments...);
    }
    constexpr auto target() const { return stored_target; }

  protected:
    R (*stored_target)(Args...) = nullptr;
};

template<typename R, typename... Args>
function(R (*)(Args...)) -> function<R(Args...)>;

template<typename R> class function<R(void)> : public callable<R> {
  public:
    function() {}
    function(R (*new_target)(void)) { stored_target = new_target; }

    void     operator=(R (*new_target)(void)) { stored_target = new_target; }
    explicit operator bool() const { return (stored_target != nullptr); }

    R    operator()() const override { return stored_target(); }
    R    call() const override { return stored_target(); }
    auto target() const { return stored_target; }

  protected:
    R (*stored_target)(void) = nullptr;
};

template<typename T> class preset_function;
template<typename R, typename... Args>
class preset_function<R(Args...)> : public callable<R> {
  public:
    preset_function()
        : stored_target(nullptr)
        , stored_arguments({}) {}
    preset_function(R (*new_target)(Args...), Args... new_args)
        : stored_target(new_target)
        , stored_arguments(new_args...) {}

    explicit operator bool() const { return ((bool)stored_target); }

    R operator()() const override {
        return apply(stored_target.target(), stored_arguments);
    }
    R call() const override {
        return apply(stored_target.target(), stored_arguments);
    }
    auto target() const { return stored_target.target(); }
    void set_target(R (*new_target)(Args...)) { stored_target = new_target; }
    void set_args(Args... new_args) {
        stored_arguments = tuple<Args...>(new_args...);
    }

  private:
    function<R(Args...)> stored_target;
    tuple<Args...>       stored_arguments;
};

template<typename R, typename... Args>
preset_function(R (*)(Args...), Args...) -> preset_function<R(Args...)>;

} // namespace std_k

#endif
