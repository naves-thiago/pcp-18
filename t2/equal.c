#include "quadrature.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define debug_print(...) printf(__VA_ARGS__)

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
 * @brief List of available tests (selected via the first command line argument
 */
static const test_function_t tests[] = {
	{.start = -5, .end = 5, .f = f_test4}
};

/**
 * @brief User selected test
 */
static const test_function_t * test;

void main_master(void) {
	int intervals;
	MPI_Comm_size(MPI_COMM_WORLD, &intervals);
	debug_print("Main: %d intervals\n", intervals);
	double step = fabs(test->end - test->start) / intervals;
	double a = test->start;
	double area = 0;
	for (int i=1; i<intervals; i++) {
		double interval[2];
		interval[0] = a;
		interval[1] = a + step;

		if (MPI_Send(&interval, 2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
			MPI_Abort(MPI_COMM_WORLD, 1);
		a += step;
	}

	debug_print("Master: [%f, %f]\n", a, test->end);
	area = integrate(test->f, a, test->end);
	debug_print("Master: area = %f\n", area);

	for (int i=1; i<intervals; i++) {
		double received;
		if (MPI_Recv(&received, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
					 MPI_STATUS_IGNORE) != MPI_SUCCESS)
			MPI_Abort(MPI_COMM_WORLD, 2);

		area += received;
	}
	printf("Area: %.16f\n", area);
}

void main_worker(void) {
	double interval[2];
	if (MPI_Recv(interval, 2, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
	             MPI_STATUS_IGNORE) != MPI_SUCCESS)
		MPI_Abort(MPI_COMM_WORLD, 2);

	debug_print("Worker: [%f, %f]\n", interval[0], interval[1]);
	double area = integrate(test->f, interval[0], interval[1]);
	if (MPI_Send(&area, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
		MPI_Abort(MPI_COMM_WORLD, 3);
	debug_print("Worker: area = %f\n", area);
}

int main(int argc, char ** argv) {
	unsigned test_id = 0;
	if (argc > 1)
		test_id = atoi(argv[1]);

	if (test_id > sizeof(tests)/sizeof(tests[0])) {
		printf("Invalid test number\n");
		return 4;
	}

	test = &tests[test_id];
	int procid;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &procid);
	if (procid == 0)
		main_master();
	else
		main_worker();

	MPI_Finalize();
	return 0;
}
