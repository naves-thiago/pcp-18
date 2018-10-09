#pragma once

/**
 * @brief Function pointer representing the function to be integrated
 */
typedef double (*func_t)(double);

/**
 * @brief Calculate the integral of f in the [a,b] interval
 */
double integrate(func_t f, double a, double b);
