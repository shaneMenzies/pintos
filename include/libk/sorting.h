#ifndef PINT_SORTING_H
#define PINT_SORTING_H

#include "common.h"

namespace std_k {

template<class T> int binary_match(T* array, unsigned int num_items, T target) {

    if (num_items > 0) {
        num_items--;
        if (num_items == 0 && array[0] == target) {
            return 0;
        } else {
            int left  = 0;
            int right = num_items;
            int middle;
            while (left <= right) {

                middle = (left + right) / 2;

                T current_value = array[middle];
                if (current_value < target) {
                    left = middle + 1;
                } else if (current_value > target) {
                    right = middle - 1;
                } else {
                    return middle;
                }
            }
        }
    }
    return -1;
}

template<class T>
int binary_find_new_index(T* array, unsigned int num_items, T target) {

    if (num_items > 0) {
        num_items--;
        if (num_items == 0) {
            return (array[0] >= target) ? 0 : 1;
        } else if (array[num_items - 1] < target) {
            return num_items;
        } else {
            int left  = 0;
            int right = num_items;
            int middle;
            while (left <= right) {

                middle = (left + right) / 2;

                if (array[middle] < target && array[middle + 1] > target) {
                    return (middle + 1);
                } else if (array[middle] < target) {
                    left = middle + 1;
                } else {
                    right = middle - 1;
                }
            }
        }
    } else {
        return 0;
    }
    return -1;
}

template<class T>
int binary_find_new_index_pointer(T** array, unsigned int num_items,
                                  T* target) {

    if (num_items > 0) {
        num_items--;
        if (num_items == 0) {
            return (*array[0] >= *target) ? 0 : 1;
        } else if (*array[num_items - 1] < *target) {
            return num_items;
        } else {
            int left  = 0;
            int right = num_items;
            int middle;
            while (left <= right) {

                middle = (left + right) / 2;

                if (*array[middle]<*target&& * array[middle + 1]> * target) {
                    return (middle + 1);
                } else if (*array[middle] < *target) {
                    left = middle + 1;
                } else {
                    right = middle - 1;
                }
            }
        }
    } else {
        return 0;
    }
    return -1;
}

template<class T>
int binary_match_pointer(T** array, unsigned int num_items, T target) {

    if (num_items > 0) {
        num_items--;
        if (num_items == 0 && (*(array[0]) == target)) {
            return 0;
        } else {
            int left  = 0;
            int right = num_items;
            int middle;
            while (left <= right) {

                middle = (left + right) / 2;

                T current_value = *(array[middle]);
                if (current_value < target) {
                    left = middle + 1;
                } else if (current_value > target) {
                    right = middle - 1;
                } else {
                    return middle;
                }
            }
        }
    }
    return -1;
}

template<class T>
void insert_at(T array[], unsigned int num_items, T target, int index) {
    if (index > (int)num_items) {
        return;
    } else if (num_items == 0 && index == 0) {
        array[0] = target;
        return;
    }

    for (int i = (num_items - 1); i >= index && i > 0; i--) {
        array[i] = array[i - 1];
    }

    array[index] = target;
}

template<class T> void remove_at(T array[], unsigned int num_items, int index) {
    if (index >= (int)num_items) { return; }

    for (int i = 0; i < (int)(num_items - 1); i++) { array[i] = array[i + 1]; }
}

template<class T>
void insert_sorted(T array[], unsigned int num_items, T target) {
    int target_index = binary_find_new_index<T>(array, num_items, target);
    if (target_index >= 0) insert_at<T>(array, num_items, target, target_index);
}

template<class T> void insertion_sort(T array[], int num_items) {

    int index = 1;

    while (index < num_items) {
        T   current_item = array[index];
        int correct_pos  = index - 1;

        while (correct_pos >= 0 && array[correct_pos] > current_item) {
            array[correct_pos + 1] = array[correct_pos];
            correct_pos--;
        }

        array[correct_pos + 1] = current_item;
        index++;
    }
}

template<class T> void insertion_sort_pointer(T* array[], int num_items) {

    int index = 1;

    while (index < num_items) {
        T*  current_item = array[index];
        int correct_pos  = index - 1;

        while (correct_pos >= 0 && *(array[correct_pos]) > *(current_item)) {
            array[correct_pos + 1] = array[correct_pos];
            correct_pos--;
        }

        array[correct_pos + 1] = current_item;
        index++;
    }
}
} // namespace std_k

#endif
