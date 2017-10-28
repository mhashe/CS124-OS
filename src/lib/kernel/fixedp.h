#ifndef __LIB_FIXED_P_H
#define __LIB_FIXED_P_H

/* 
 * Comment what kfloat is. why is used. how it is used. 
 */


/* A fixed point / kernel float. */
typedef int fixedp;

fixedp fixedp_from_int(int n UNUSED);

int fixedp_to_int_rounded(fixedp x UNUSED);

int fixedp_to_int_nearest(fixedp x UNUSED);

fixedp fixedp_add(UNUSED fixedp x, UNUSED fixedp y);

fixedp fixedp_subtract(UNUSED fixedp x, UNUSED fixedp y);

fixedp fixedp_add_with_int(UNUSED fixedp x, UNUSED int n);

fixedp fixedp_sub_from_fp(UNUSED fixedp x, UNUSED int n);

fixedp fixedp_multiply(UNUSED fixedp x, UNUSED fixedp y);

fixedp fixedp_multiple_with_int(UNUSED fixedp x, UNUSED int n);

fixedp fixedp_divide(UNUSED fixedp x, UNUSED fixedp y);

fixedp fixedp_divide_by_intUNUSED (fixedp x, UNUSED int n);



#endif /* lib/kernel/kfloat.h */
