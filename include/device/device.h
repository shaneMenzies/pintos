#ifndef PINTOS_DEVICE_H
#define PINTOS_DEVICE_H

#include "device_id.h"
#include "device_tree.h"
#include "libk/avl_tree.h"
#include "libk/cstring.h"
#include "libk/sorting.h"
#include "libk/string.h"

struct device {
    std_k::string name;
    const char*   model;

    struct property {
        const char* name  = nullptr;
        uint64_t    value = 0;

        property(const char* name)
            : name(name) {}

        property(const char* name, uint64_t value)
            : name(name)
            , value(value) {}

        friend bool operator<(const property& left, const property& right) {
            return (std_k::strcmp(left.name, right.name) > 0);
        }
        friend bool operator>(const property& left, const property& right) {
            return (right < left);
        }

        friend bool operator==(const property& left, const property& right) {
            return (std_k::strcmp(left.name, right.name) == 0);
        }

        struct predefined {
            constexpr static const char* const serial_number = "serial_number";
            constexpr static const char* const version       = "version";
        };
    };
    static property invalid_property;

    uint64_t  num_properties;
    property* properties;

    devices::device_tree_t::node tree_node;

    device(const char* name, const char* model, property* properties,
           uint64_t num_properties)
        : name(name)
        , model(model)
        , num_properties(num_properties)
        , properties(properties)
        , tree_node(this) {

        // Sort list of properties for efficient sorting
        std_k::insertion_sort<property>(properties, num_properties);
    }

    device(const char* name, const char* model, property* properties,
           uint64_t                   num_properties,
           devices::device_tree_node* initial_children[],
           size_t                     initial_num_children)
        : name(name)
        , model(model)
        , num_properties(num_properties)
        , properties(properties)
        , tree_node(this, initial_children, initial_num_children) {

        // Sort list of properties for efficient sorting
        std_k::insertion_sort<property>(properties, num_properties);
    }

    property& get_property(const char* target) {

        if (!num_properties) return invalid_property;

        // Properties are sorted, so can do a binary search
        int left  = 0;
        int right = (num_properties - 1);
        while (left <= right) {
            int current = (left + right) / 2;
            int result  = std_k::strcmp(properties[current].name, target);

            if (result < 0) {
                left = current + 1;
            } else if (result > 0) {
                right = current - 1;
            } else {
                return properties[current];
            }
        }

        // Target not found
        return invalid_property;
    }

    friend bool operator<(const device& left, const device& right) {
        return (std_k::strcmp(left.name, right.name) > 0);
    }
    friend bool operator>(const device& left, const device& right) {
        return (right < left);
    }

    friend bool operator==(const device& left, const device& right) {
        return (std_k::strcmp(left.name, right.name) == 0);
    }
};

struct device_pointer_compare {
    constexpr bool operator()(const device* left, const device* right) {
        return (left < right);
    }
    device_pointer_compare() {}
};

#endif // PINTOS_DEVICE_H
