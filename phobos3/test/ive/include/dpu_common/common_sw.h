#ifndef _COMMON_SW_H_
#define _COMMON_SW_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/**
 * @brief get the slpe of alpha blending function
 *
 * @param x_left the left node
 * @param x_right the right node
 * @param y_left the value of left node
 * @param y_right the value of right node
 * @param shift_bit the decimal place precision - number of shift bit
 * @return (int) the slope
 */
int cm_sw_get_slope(int x_left, int x_right, int y_left, int y_right, int shift_bit);

#endif