#include "test_functions.h"
#include "quadrature.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>

#ifdef DEBUG
#define debug_print(...) do {printf(__VA_ARGS__); fflush(stdout); } while (0);
#else
#define debug_print(...)
#endif

/**
 * @brief User selected test
 */
static const test_function_t * test;

void error(void) {
	volatile int i = 0;
	printf("PID %d ready for attach\n", getpid());
	fflush(stdout);
	while (i==0) {
		sleep(5);
	}
}

// MPI wrappers
// DO NOT CREATE A NON-STATIC FUNCTION CALLED send. BREAKS MPI AT RUNTIME !
static void send(const void *buf, int count, MPI_Datatype datatype, int dest,
                 int tag, MPI_Comm comm) {
	if (MPI_Send(buf, count, datatype, dest, tag, comm) != MPI_SUCCESS) {
		error();
		MPI_Abort(MPI_COMM_WORLD, 20);
	}
}

static void recv(void *buf, int count, MPI_Datatype datatype, int source,
                 int tag, MPI_Comm comm, MPI_Status *status) {
	if (MPI_Recv(buf, count, datatype, source, tag, comm, status) != MPI_SUCCESS) {
		error();
		MPI_Abort(MPI_COMM_WORLD, 10);
	}
}

void main_master(unsigned intervals) {
	int num_procs;
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	num_procs --; // Account for the master process

	debug_print("Main: %u intervals\n", intervals);
	double step = fabs(test->end - test->start) / intervals;
	double a = test->start;
	double area = 0;
	int curr_interval = 0;

	// Each worker will request an interval then send the result, which doubles as a
	// new inerval request.
	// The first interval request has data (area) 0, as to not affect the sum.
	// An interal where start = end = 0 signals that there are no more intervals available
	while (curr_interval < intervals + num_procs) {
		double data[2];
		MPI_Status status;
		recv(data, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		area += data[0];

		if (curr_interval < intervals) {
			data[0] = a;
			if (curr_interval < intervals - 1)
				data[1] = a + step;
			else
				data[1] = test->end;
			a += step;
		}
		else {
			data[0] = 0;
			data[1] = 0;
		}

		send(data, 2, MPI_DOUBLE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
		curr_interval ++;
	}

	printf("Area: %.16f\n", area);
}

void main_worker(void) {
	double interval[2];
	interval[0] = 0;
	// Send area 0 to receive the first interval
	send(interval, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

	while(1) {
		recv(interval, 2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if (interval[0] == 0 && interval[1] == 0)
			break;

		debug_print("Worker: [%f, %f]\n", interval[0], interval[1]);
		double area = integrate(test->f, interval[0], interval[1]);
		send(&area, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		debug_print("Worker: area = %f\n", area);
	}
}

int main(int argc, char ** argv) {
	unsigned test_id = 0;
	unsigned intervals = 1;
	if (argc > 1)
		intervals = atoi(argv[1]);

	if (argc > 2)
		test_id = atoi(argv[2]);

	if (test_id > sizeof(tests)/sizeof(tests[0])) {
		printf("Invalid test number\n");
		return 4;
	}

	test = &tests[test_id];
	int procid;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &procid);
	if (procid == 0)
		main_master(intervals);
	else
		main_worker();

	MPI_Finalize();
	return 0;
}

