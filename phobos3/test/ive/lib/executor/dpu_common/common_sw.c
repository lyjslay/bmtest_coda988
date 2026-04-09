#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int cm_sw_get_slope(int x_left, int x_right, int y_left, int y_right, int shift_bit) {
    return round((1 << shift_bit) * (y_right - y_left) / (x_right - x_left));
}
