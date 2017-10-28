#include "fixedp.h"
#include "../debug.h"

#include <stdio.h>
#include <stdint.h>


fixedp fixedp_from_int(int n) {
    return n * fp_f;
}

int fixedp_to_int_floored(fixedp x) {
    return x / fp_f;
}

int fixedp_to_int_nearest(fixedp x) {
    if (x >= 0) {
        return (x + fp_f / 2) / fp_f;
    } else {
        return (x - fp_f / 2) / fp_f;
    }
}

fixedp fixedp_add(fixedp x, fixedp y) {
    return x + y;
}

fixedp fixedp_subtract(fixedp x, fixedp y) {
    return x - y;
}

fixedp fixedp_add_with_int(fixedp x, int n) {
    return x + n * fp_f;
}

fixedp fixedp_sub_from_fp(fixedp x, int n) {
    return x - n * fp_f;
}

fixedp fixedp_multiply(fixedp x, fixedp y) {
    return ((int64_t) x) * y / fp_f;
}

fixedp fixedp_multiple_with_int(fixedp x, int n) {
    return x * n;
}

fixedp fixedp_divide(fixedp x, fixedp y) {
    return ((int64_t) x) * y / fp_f;
}

fixedp fixedp_divide_by_int (fixedp x, int n) {
    return x / n;
}

