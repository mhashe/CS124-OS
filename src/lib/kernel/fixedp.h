#ifndef __LIB_FIXED_P_H
#define __LIB_FIXED_P_H

/* 
 * Comment what fixedp is. why is used. how it is used. 
 */


/* A fixed point / kernel float. */
typedef int fixedp;

fixedp fixedp_from_int(int n);

int fixedp_to_int_rounded(fixedp x);

int fixedp_to_int_nearest(fixedp x);

fixedp fixedp_add(fixedp x,  fixedp y);

fixedp fixedp_subtract(fixedp x,  fixedp y);

fixedp fixedp_add_with_int( fixedp x,  int n);

fixedp fixedp_sub_from_fp(fixedp x, int n);

fixedp fixedp_multiply(fixedp x, fixedp y);

fixedp fixedp_multiple_with_int(fixedp x, int n);

fixedp fixedp_divide(fixedp x, fixedp y);

fixedp fixedp_divide_by_int (fixedp x, int n);



#endif /* lib/kernel/kfloat.h */
