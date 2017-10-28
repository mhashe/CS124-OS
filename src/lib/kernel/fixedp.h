#ifndef __LIB_FIXED_P_H
#define __LIB_FIXED_P_H


#define fp_f    (2<<(14-1)) /* Using 17.14 fixed point representation. */


/* A fixed point number, i.e. a number scaled by fp_f. */
typedef int fixedp;


/* Converts int into fixed-point by scaling up. */
fixedp fixedp_from_int(int n);


/* Converts int into fixed-point by scaling down, rounding towards zero. */
int fixedp_to_int_floored(fixedp x);


/* Converts int into fixed-point by scaling down, towards nearest int. */
int fixedp_to_int_nearest(fixedp x);


/* Adds two fixed-pont numbers, returning a new fixedp. */
fixedp fixedp_add(fixedp x,  fixedp y);


/* Subtracts two fixed-point numbers, returning a new fixedp. */
fixedp fixedp_subtract(fixedp x,  fixedp y);


/* Adds an integer to a fixed-point number, returning a new fixedp. */
fixedp fixedp_add_with_int( fixedp x,  int n);


/* Subtracts an integer from a fixedp, returning a new fixedp. */
fixedp fixedp_sub_from_fp(fixedp x, int n);


/* Multiplies two fixedp numbers, returning a new fixedp. */
fixedp fixedp_multiply(fixedp x, fixedp y);


/* Multiplies a fixedp with an int, returning a new fixedp. */
fixedp fixedp_multiply_with_int(fixedp x, int n);


/* Divides two fixedp numbers, returning a new fixedp. */
fixedp fixedp_divide(fixedp x, fixedp y);


/* Divides a fixedp by an int, returning a new fixedp. */
fixedp fixedp_divide_by_int (fixedp x, int n);


#endif /* lib/kernel/fixedp.h */

