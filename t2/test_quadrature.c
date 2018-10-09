#include "quadrature.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t intervals;

static const double A = -5;
static const double B = 5;
//static const double B = M_PI;

static double f_test(double x) {
	return 10*x*x*x-30*x*x+20;
}

static double f_test2(double x) {
	return sin(x) + sin(2*x + M_PI_4) + sin(3*x + M_PI_2);
}

static double f_test3(double x) {
	return x*x*x - 5*x*x + 20;
}

static double f_test4(double x) {
	return x*sin(x)+x*x*sin(10*x+M_PI_4/2.0)+2*sin(13*x+M_PI/3.0)+(x+3)*(x+3)/4.0;
}

int main(int argc, char ** argv) {
	intervals = 1;
	if (argc > 1)
		intervals = atoi(argv[1]);
	
	double step = fabs(B - A) / intervals;
	double a = A;
	double area = 0;
	for (uint32_t i=0; i<intervals-1; i++) {
		area += integrate(f_test4, a, a + step);
		a += step;
	}

	// Avoid rounding errors
	area += integrate(f_test4, a, B);

	printf("Area: %.16f\n", area);
	return 0;
}
