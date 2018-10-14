#pragma once
#include <math.h>

/**
 * @brief Function pointer representing the function to be integrated
 */
typedef double (*func_t)(double);

/**
 * @brief Calculate the integral of f in the [a,b] interval
 */
double integrate(func_t f, double a, double b);

/**
 * @brief Maximum allowed difference in area
 */
static const double precision = 0.0000000000000001; // 10^-16

/**
 * @brief Calculate the area of the trapezoid
 */
static inline double calc_area(func_t f, double a, double b) {
	return (f(a) + f(b)) * fabs(b-a) * 0.5;
}
