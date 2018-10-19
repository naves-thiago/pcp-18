#pragma once

#include "quadrature.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief This struct stores a test function pointer
 * and the integration interval
 */
typedef struct {
	double start;
	double end;
	func_t f;
} test_function_t;

static double f_test4(double x) {
	return x*sin(x)+x*x*sin(10*x+M_PI/8.0)+2*sin(13*x+M_PI/3.0)+(x+3)*(x+3)/4.0;
}

/**
 * @brief List of available tests
 */
static const test_function_t tests[] = {
	{.start = -5, .end = 5, .f = f_test4}
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))
