#include "fixedp.h"
#include "../debug.h"


#include <stdio.h>
#include <stdint.h>

/* Converts int into fixed-point by scaling up. */
fixedp fixedp_from_int(int n) {
    return n * fp_f;
}

/* Converts fixed-point into int by scaling down, rounding towards zero. */
int fixedp_to_int_floored(fixedp x) {
    return x / fp_f;
}

/* Converts fixed-point into int by scaling up. */
int fixedp_to_int_ceiled(fixedp x) {
    if (x % fp_f == 0) {
        return x / fp_f;
    } else {
        return (x / fp_f) + 1;
    }
}

/* Converts fixed-point into int by scaling down, towards nearest int. */
int fixedp_to_int_nearest(fixedp x) {
    /* Perturb number so that divison has desired effect. */
    if (x >= 0) {
        return (x + fp_f / 2) / fp_f;
    } else {
        return (x - fp_f / 2) / fp_f;
    }
}

/* Adds two fixed-pont numbers, returning a new fixedp. */
fixedp fixedp_add(fixedp x, fixedp y) {
    return x + y;
}

/* Subtracts two fixed-point numbers, returning a new fixedp. */
fixedp fixedp_subtract(fixedp x, fixedp y) {
    return x - y;
}

/* Adds an integer to a fixed-point number, returning a new fixedp. */
fixedp fixedp_add_with_int(fixedp x, int n) {
    return x + n * fp_f;
}

/* Subtracts an integer from a fixedp, returning a new fixedp. */
fixedp fixedp_sub_from_fp(fixedp x, int n) {
    return x - n * fp_f;
}

/* Multiplies two fixedp numbers, returning a new fixedp. */
fixedp fixedp_multiply(fixedp x, fixedp y) {
    return ((int64_t) x) * y / fp_f;
}

/* Multiplies a fixedp with an int, returning a new fixedp. */
fixedp fixedp_multiply_with_int(fixedp x, int n) {
    return x * n;
}

/* Divides two fixedp numbers, returning a new fixedp. */
fixedp fixedp_divide(fixedp x, fixedp y) {
    return ((int64_t) x) * fp_f / y;
}

/* Divides a fixedp by an int, returning a new fixedp. */
fixedp fixedp_divide_by_int (fixedp x, int n) {
    return x / n;
}

