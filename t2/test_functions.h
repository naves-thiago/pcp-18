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

static double f0(double x) {
	return x*sin(x)+x*x*sin(10*x+M_PI/8.0)+2*sin(13*x+M_PI/3.0)+(x+3)*(x+3)/4.0;
}

static double f1(double x) {
// sum[0.1*cos(x*y^2/40), {y,0,10}] ; x=-30..30
	double tmp =  0;
	for (int y=0; y<=10; y++)
		tmp += 0.1*cos(x*y*y/40.0);
	return tmp;
}

static double f2(double x) {
// sum[0.1*cos(x*y^2/40), {y,0,10}] ; x=-30..30
	double tmp =  0;
	for (int y=0; y<=50; y++)
		tmp += 0.1*cos(x*y*y/40.0);
	return tmp;
}

/**
 * @brief List of available tests
 */
static const test_function_t tests[] = {
	{.start = -5, .end = 5, .f = f0},
	{.start = -30, .end = 30, .f = f1},
	{.start = -10, .end = 10, .f = f2},
};

#define NTESTS (sizeof(tests) / sizeof(tests[0]))
