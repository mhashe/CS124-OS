#ifndef ERR_HAND_H
#define ERR_HAND_H

/*
 * This function reads errno and prints the associated error statement. Had to
 * remove EDEADLOCK, ENOTSUP, EWOULDBLOCK as they are duplicated error codes.
 *
 * Returns:
 *      0: Normal execution
 *      any other integer: error's error code
 */
int print_err();

#endif