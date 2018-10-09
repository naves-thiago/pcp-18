#include "quadrature.h"
#include <stdio.h>
#include <mpi.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const double A = -5;
static const double B = 5;

static double f_test4(double x) {
	return x*sin(x)+x*x*sin(10*x+M_PI/8.0)+2*sin(13*x+M_PI/3.0)+(x+3)*(x+3)/4.0;
}

void main_master(void) {
	int numprocs;
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	int intervals = numprocs - 1;
	printf("Main: %d intervals\n", intervals);
	double step = fabs(B - A) / intervals;
	double a = A;
	double area = 0;
	for (int i=1; i<=intervals; i++) {
		double interval[2];
		interval[0] = a;
		if (i != intervals)
			interval[1] = a + step;
		else
			interval[1] = B; // Avoid rounding errors

		if (MPI_Send(&interval, 2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
			MPI_Abort(MPI_COMM_WORLD, 1);
		a += step;
	}

	for (int i=1; i<=intervals; i++) {
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

	printf("Worker: [%f, %f]\n", interval[0], interval[1]);
	double area = integrate(f_test4, interval[0], interval[1]);
	if (MPI_Send(&area, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD) != MPI_SUCCESS)
		MPI_Abort(MPI_COMM_WORLD, 3);
	printf("Worker: area = %f\n", area);
}

int main(int argc, char ** argv) {
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
