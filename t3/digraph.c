#include <stdio.h>
#include <stdlib.h>
#include "digraph.h"

static int* digraph;
static int n;

int Cost(int city1, int city2) {
   return digraph[city1*n + city2];
}

/*------------------------------------------------------------------
 * Function:  Read_digraph
 * Purpose:   Read in the number of cities and the digraph of costs
 * In arg:
 *    fname:     digraph file name
 * Globals out:
 *    n:        the number of cities
 *    digraph:  the matrix file
 * Ret:
 *    The number of cities
 */
int Read_digraph(char fname[]) {
   int i, j;
   FILE* digraph_file;

   digraph_file = fopen(fname, "r");
   if (digraph_file == NULL) {
      fprintf(stderr, "Can't open %s\n", fname);
      return -1;
   }

   fscanf(digraph_file, "%d", &n);
   if (n <= 0) {
      fprintf(stderr, "Number of vertices in digraph must be positive\n");
      exit(-1);
   }
   digraph = malloc(n*n*sizeof(int));

   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++) {
         fscanf(digraph_file, "%d", &digraph[i*n + j]);
         if (i == j && digraph[i*n + j] != 0) {
            fprintf(stderr, "Diagonal entries must be zero\n");
            exit(-1);
         } else if (i != j && digraph[i*n + j] <= 0) {
            fprintf(stderr, "Off-diagonal entries must be positive\n");
            fprintf(stderr, "diagraph[%d,%d] = %d\n", i, j, digraph[i*n+j]);
            exit(-1);
         }
      }
   fclose(digraph_file);
   return n;
}  /* Read_digraph */

/*------------------------------------------------------------------
 * Function:  Print_digraph
 * Purpose:   Print the number of cities and the digraphrix of costs
 * Globals in:
 *    n:        number of cities
 *    digraph:  digraph of costs
 */
void Print_digraph(void) {
   int i, j;

   printf("Order = %d\n", n);
   printf("Matrix = \n");
   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
         printf("%2d ", digraph[i*n+j]);
      printf("\n");
   }
   printf("\n");
}  /* Print_digraph */

void Free_digraph(void) {
   free(digraph);
}
