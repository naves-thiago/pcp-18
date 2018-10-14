#include "quadrature.h"
#include <math.h>

static double integrate_recursive(func_t f, double a, double b, double area) {
	double mid = (a + b) / 2.0;
	double area_left = calc_area(f, a, mid);
	double area_right = calc_area(f, mid, b);
	double area_lr = area_left + area_right;
	if (fabs(area - area_lr) <= precision)
		return area_lr;

	return integrate_recursive(f, a, mid, area_left) +
	       integrate_recursive(f, mid, b, area_right);
}

double integrate(func_t f, double a, double b) {
	return integrate_recursive(f, a, b, calc_area(f, a, b));
}


