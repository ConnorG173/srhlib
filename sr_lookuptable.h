#ifndef SR_LOOKUPTABLE_H
#define SR_LOOKUPTABLE_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    double key;
    double val;
} keyval_t;

struct lut;
typedef struct lut lut_t;
typedef struct lut {
    keyval_t* keyval_arr;
    size_t num_keys;
    double (* const lut_get)(lut_t*, double);
    void (* lut_destructor)(lut_t*);
    bool on_heap;
} lut_t;

/**
 * @brief Initialize a lut_t on the stack with array on the heap
 * @param vals Array of values to be interpolated on
 * @param keys Array of keys for each value 
 * @param num_keys Number of elements in the arrays
 */
lut_t init_lut_arr_heap(double* restrict vals, double* restrict keys, size_t num_keys);

/**
 * @brief Initialize a lut_t on the stack given a pre-defined key/value array
 * @param keyval_arr Array of constant key/value pairs
 * @param num_keys Number of key/value pairs
 */
lut_t init_lut_premade_arr(const keyval_t* keyval_arr, size_t num_keys);

#ifdef SR_LOOKUPTABLE_IMPLEMENTATION

#include <memory.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int find_lookup_index(lut_t* lut, double input)
{
    int left = 0;
    int right = lut->num_keys - 2;
    int mid;
    while (left <= right)
    {
        mid = left + (right - left) / 2;
        if (lut->keyval_arr[mid].key <= input < lut->keyval_arr[mid + 1].key )
        {
            return mid;
        }
        if (lut->keyval_arr[mid + 1].key <= input)
        {
            left = mid + 1;
        }
        if (lut->keyval_arr[mid].key > input)
        {
            right = mid - 1;
        }
    }
    return -1;
}

double interpolate(lut_t* self, double input, int lower_keyval_idx)
{
    double lower_key = self->keyval_arr[lower_keyval_idx].key;
    double higher_key = self->keyval_arr[lower_keyval_idx + 1].key;
    double lower_val = self->keyval_arr[lower_keyval_idx].val;
    double higher_val = self->keyval_arr[lower_keyval_idx + 1].val;

    return lower_val + (higher_val - lower_val) / (higher_key - lower_key) * (input - lower_key);
}

/**
 * @brief Get the interpolated value given an imput
 * @param self Pointer to the lut that you want to interpolate with, usually self
 * @param input Input to interpolate on
 * @returns Interpolated value as double. NAN if error.
 */
double lut_get(lut_t* self, double input)
{
    if (self->keyval_arr == NULL)
    {
        perror("Tried to get a value from a destroyed or un-initialized LUT!\n");
        return NAN;
    }
    if (self->num_keys <= 1) { perror("Can't Interpolate on 1 or less table entries!\n"); return NAN; }
    if (input > self->keyval_arr[self->num_keys - 1].val) { perror("Input Out of Range of lut!\n"); return NAN; }

    int lower_keyval_idx = find_lookup_index(self, input);
    if (lower_keyval_idx == -1) { perror("Couldn't find suitable interp start index\n"); return NAN; }
    if (lower_keyval_idx >= self->num_keys - 1) { return self->keyval_arr[self->num_keys - 1].val; }

    return interpolate(self, input, lower_keyval_idx);
}

void destroy_lut_doublefree_error(lut_t* lut)
{
    perror("Tried to double-free a LUT!\n");
    return;
}

void destroy_lut_noheap_error(lut_t* lut)
{
    perror("Tried to destroy a const array LUT!\n");
    return;
}

void destroy_lut_heap(lut_t* lut)
{
    if (!lut->on_heap) { perror("Tried to Free a Non-Heap LUT!\n"); return; }
    free(lut->keyval_arr);
    lut->keyval_arr = NULL;
    lut->lut_destructor = destroy_lut_doublefree_error;
}

int compare_for_sort(const void* a, const void* b)
{
    keyval_t* keyval1 = (keyval_t*) a;
    keyval_t* keyval2 = (keyval_t*) b;

    if (keyval1->key == keyval2->key) { perror("You have 2 values with the same key!\nOne will be discarded\n"); return 0; }
    if (keyval1->key < keyval2->key) { return -1; }
    else { return 1; }
}

void delete_and_shift(keyval_t* to_remove, size_t index, size_t num_keys)
{
    memmove(to_remove, to_remove + 1, ((num_keys - 1) - index) * sizeof(keyval_t));
}

lut_t init_lut_arr_heap(double* restrict vals, double* restrict keys, size_t num_keys)
{
    //create key/value pair array from 2 arrays of doubles
    keyval_t* keyval_arr = malloc(num_keys * sizeof(keyval_t));
    for (int i = 0; i < num_keys; i++)
    {
        keyval_arr[i].val = vals[i];
        keyval_arr[i].key = keys[i];
    }
    qsort((void*)keyval_arr, num_keys, sizeof(keyval_t), compare_for_sort);

    size_t curr_num_keys = num_keys;
    for (int i = 0; i < num_keys - 1; i++) //Remove duplicates
    {
        if (keyval_arr[i].key == keyval_arr[i + 1].key)
        {
            delete_and_shift(keyval_arr + i + 1, i, curr_num_keys);
            curr_num_keys--;
        }
    }

    keyval_arr = realloc(keyval_arr, curr_num_keys * sizeof(keyval_t));

    lut_t lut = {
        .keyval_arr = keyval_arr,
        .num_keys = curr_num_keys,
        .on_heap = true,

        //FPs
        .lut_get = lut_get,
        .lut_destructor = destroy_lut_heap,
    };

    return lut;
} 

lut_t init_lut_premade_arr(const keyval_t* keyval_arr, size_t num_keys)
{
    lut_t lut = {
        .keyval_arr = keyval_arr,
        .num_keys = num_keys,
        .on_heap = false,
        
        //FPs
        .lut_get = lut_get,
        .lut_destructor = destroy_lut_noheap_error,
    };
    return lut;
}

#endif

#endif