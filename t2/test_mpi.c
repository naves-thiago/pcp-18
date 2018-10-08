#include<stdio.h>
#include<mpi.h>

int main(int argc, char ** argv) {
	int ierr, procid, numprocs;
	ierr = MPI_Init(&argc, &argv);
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	printf("ID %d/%d\n", procid, numprocs);

	MPI_Send(&procid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	printf("%d: Sent %d\n", procid, procid);

	if (procid == 0) {
		for (int i=0; i<numprocs; i++) {
			int val;
			MPI_Status status;
			ierr = MPI_Recv(&val, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			if (ierr != MPI_SUCCESS)
				printf("Receive failed\n");
			else
				printf("Received: %d\n", val);
		}
	}

	ierr = MPI_Finalize();
	return 0;
}
