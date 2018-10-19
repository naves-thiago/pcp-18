#include "quadrature.h"
#include "llfifo.h"
#include "bag.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#define debug_print(...) do {printf(__VA_ARGS__); fflush(stdout); } while (0);
#define debug_print(...)

/**
 * @brief This struct stores a test function pointer
 * and the integration interval
 */
typedef struct {
	double start;
	double end;
	func_t f;
} test_function_t;

/**
 * @brief This struct stores the id of a waiting worker in the FIFO
 */
typedef struct {
	LLFifoItem item;
	int id;
} waiting_t;

static MPI_Datatype mpi_interval_type;

static double f_test4(double x) {
	return x*sin(x)+x*x*sin(10*x+M_PI/8.0)+2*sin(13*x+M_PI/3.0)+(x+3)*(x+3)/4.0;
}

/**
 * @brief List of available tests (selected via the first command line argument)
 */
static const test_function_t tests[] = {
	{.start = -5, .end = 5, .f = f_test4}
};

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
	int num_workers;
	MPI_Comm_size(MPI_COMM_WORLD, &num_workers);
	num_workers --; // Account for the master process
	LLFifo waiting; // Workers waiting for an interval
	llFifoInit(&waiting);
	waiting_t * wait_handles = (waiting_t *)malloc(num_workers * sizeof(waiting_t));
	for (int i=0; i<num_workers; i++) {
		wait_handles[i].id = i+1;
	}

	bag_t bag;
	bag_init(&bag, intervals);

	debug_print("Main: %u intervals\n", intervals);
	double step = fabs(test->end - test->start) / intervals;
	double a = test->start;
	double area = 0;

	interval_t interval;
	interval.area = 0;
	for (unsigned i=0; i<intervals-1; i++) {
		interval.start = a;
		interval.end = a + step;
		bag_push(&bag, interval);
		a += step;
	}

	interval.start = a;
	interval.end = test->end;
	bag_push(&bag, interval);

	// Each worker sends a messase to either request an interval / send the
	// calculated area (start = end = 0) or send an interval to add to the bag
	// (start != 0 || end != 0, area disregarded).
	// If an interval is available in the bag, the master replies with the
	// area (previous calculation), interval start and interval end.
	while (llFifoCount(&waiting) < num_workers || bag_count(&bag)) {
		MPI_Status status;
		recv(&interval, 1, mpi_interval_type, MPI_ANY_SOURCE, 0,
		     MPI_COMM_WORLD, &status);
		debug_print("Master {%f, %f, %f}\n", interval.area, interval.start,
		            interval.end);

		if (interval.start != 0 || interval.end != 0) {
			bag_push(&bag, interval);
		}
		else {
			// Did not send an interval -> worker is done
			area += interval.area;
			llFifoPush(&waiting, (LLFifoItem *)&wait_handles[status.MPI_SOURCE-1]);
		}

		if (llFifoCount(&waiting) && bag_pop(&bag, &interval)) {
			waiting_t * w = (waiting_t *)llFifoPop(&waiting);
			send(&interval, 1, mpi_interval_type, w->id, 0, MPI_COMM_WORLD);
		}
	}

	// Done. Signal workers to stop
	debug_print("Master done\n");
	interval.area = 0;
	interval.start = 0;
	interval.end = 0;
	for (int i=1; i<=num_workers; i++)
		send(&interval, 1, mpi_interval_type, i, 0, MPI_COMM_WORLD);

	printf("Area: %.16f\n", area);
}

void main_worker(void) {
	interval_t interval;
	interval.area = 0;
	interval.start = 0;
	interval.end = 0;

	// Send interval [0, 0] to receive the first interval
	// and area 0 to not affect the sum
	send(&interval, 1, mpi_interval_type, 0, 0, MPI_COMM_WORLD);

	while(1) {
		recv(&interval, 1, mpi_interval_type, 0, 0, MPI_COMM_WORLD,
		     MPI_STATUS_IGNORE);
		if (interval.start == 0 && interval.end == 0)
			return;

		debug_print("Worker: [%f, %f]\n", interval.start, interval.end);

		double area = interval.area;
		double a = interval.start;
		double b = interval.end;
		while (1) {
			double mid = (a + b) / 2.0;
			double area_left = calc_area(test->f, a, mid);
			double area_right = calc_area(test->f, mid, b);
			double area_lr = area_left + area_right;
			if (fabs(area - area_lr) <= precision) {
				interval.area = area_lr;
				interval.start = 0;
				interval.end = 0;
				debug_print("Worker: area = %f\n", area_lr);
				break;
			}
			else {
				interval.area = area_right;
				interval.start = mid;
				interval.end = b;
				debug_print("Inter {%f, %f, %f}\n", interval.area,
				            interval.start, interval.end);
				send(&interval, 1, mpi_interval_type, 0, 0, MPI_COMM_WORLD);
				b = mid;
				area = area_left;
			}
		}

		debug_print("Worker {%f, %f, %f}\n", interval.area,
		            interval.start, interval.end);
		send(&interval, 1, mpi_interval_type, 0, 0, MPI_COMM_WORLD);
		debug_print("Worker sent area\n");
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

	// Create the message_t type in MPI
	int block_lens[3] = {1, 1, 1};
	MPI_Datatype types[3] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
	MPI_Aint offsets[3];
	offsets[0] = offsetof(interval_t, area);
	offsets[1] = offsetof(interval_t, start);
	offsets[2] = offsetof(interval_t, end);
	MPI_Type_create_struct(3, block_lens, offsets, types, &mpi_interval_type);
	MPI_Type_commit(&mpi_interval_type);

	MPI_Comm_rank(MPI_COMM_WORLD, &procid);
	if (procid == 0)
		main_master(intervals);
	else
		main_worker();

	MPI_Type_free(&mpi_interval_type);
	MPI_Finalize();
	return 0;
}


