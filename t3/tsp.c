#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <mpi.h>
#include <signal.h>
#include "timer.h"
#include "queue.h"
#include "tour.h"
#include "digraph.h"
#include "debug.h"

static const int INFINITY = 1000000;
static const int FALSE = 0;
static const int TRUE = 1;
static const int TAG_DONE = 1;
static const int TAG_BEST_TOUR = 2;
static const int TAG_WORK = 3;

typedef struct {
   int curr_tc;  // Number of threads that have entered the barrier
   int max_tc;   // Number of threads that need to enter the barrier
   pthread_mutex_t mutex;
   pthread_cond_t ok_to_go;
}  barrier_struct;
typedef barrier_struct* my_barrier_t;

typedef struct {
   my_stack_t stack; // Stack where to put tours
   pthread_mutex_t stack_mutex, cond_mutex;
   pthread_cond_t cond;
} stack_request_t;

typedef struct {
   my_stack_t stack;
   long rank;
} thread_info_t;

/* Global Vars: */
int n;  /* Number of cities in the problem */
int proc_threads; /* Number of threads per process */
int procs = 1; /* Total number of processes */
int proc_id; /* Process id */
city_t home_town = 0;
my_queue_t queue;
int queue_size;
int init_tour_count;
my_barrier_t bar_str;
int min_split_sz;
stack_request_t stack_request;
tour_t best_tour;
pthread_mutex_t best_tour_mutex;
int running_threads;
int running_procs;
int process_done;
pthread_mutex_t running_mutex;
thread_info_t* thread_infos;
void Usage(char* prog_name);

void Send_work_if_needed(my_stack_t stack, int my_rank);
void Request_work(my_stack_t stack, long my_rank);
tour_t Get_work(my_stack_t stack, long my_rank);
void* Par_tree_search(void* rank);
void* Proxy_send(void* p);
void* Proxy_receive(void* p);

void Partition_tree(long my_rank, my_stack_t stack);
void Set_init_tours(long my_rank, int* my_first_tour_p,
      int* my_last_tour_p);
void Build_initial_queue(void);

int Get_upper_bd_queue_sz(void);
long long Factorial(int k);

/* Barrier */
my_barrier_t My_barrier_init(int thr_count);
void My_barrier_destroy(my_barrier_t bar);
void My_barrier(my_barrier_t bar);

void Split_stack(my_stack_t stack, my_stack_t dst_stack,
      long my_rank);

int  Best_tour(tour_t tour);
int  Update_best_tour_local(tour_t tour);
void Update_best_tour_global(tour_t tour);
int  Feasible(tour_t tour, city_t city);

/******************************************************************/

void gdb(void) {
	volatile int i = 0;
	printf("PID %d ready for attach\n", getpid());
	fflush(stdout);
	while (i==0) {
		sleep(5);
	}
}

int main(int argc, char* argv[]) {
   double start, finish;
   long thread;
   pthread_t* thread_handles;
   int mpi_thread_level;

   MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_level);
   if (mpi_thread_level != MPI_THREAD_MULTIPLE) {
      printf("MPI Init thread failed\n");
      MPI_Finalize();
      return 0;
   }
   MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
   MPI_Comm_size(MPI_COMM_WORLD, &procs);

   if (argc != 4) Usage(argv[0]);
   proc_threads = strtol(argv[1], NULL, 10);
   if (proc_threads <= 0) {
      fprintf(stderr, "Thread count must be positive\n");
      Usage(argv[0]);
   }
   n = Read_digraph(argv[2]);
   if (n <= 0)
      Usage(argv[0]);

   min_split_sz = strtol(argv[3], NULL, 10);
   if (min_split_sz <= 0) {
      fprintf(stderr, "Min split size should be positive\n");
      Usage(argv[0]);
   }

   stack_request.stack = NULL;

   thread_handles = malloc(proc_threads*sizeof(pthread_t));
   thread_infos = malloc(proc_threads*sizeof(thread_info_t));
   bar_str = My_barrier_init(proc_threads);
   pthread_mutex_init(&best_tour_mutex, NULL);
   pthread_mutex_init(&running_mutex, NULL);

   best_tour = Alloc_tour(NULL, n);
   Init_tour(best_tour, INFINITY);

   running_threads = proc_threads;
   GET_TIME(start);
   for (thread = 0; thread < proc_threads; thread++) {
      thread_infos[thread].stack = Init_stack(n);
      thread_infos[thread].rank = proc_id * proc_threads + thread;
      pthread_create(&thread_handles[thread], NULL,
            Par_tree_search, (void*) &thread_infos[thread]);
   }

   for (thread = 0; thread < proc_threads; thread++)
      pthread_join(thread_handles[thread], NULL);
   GET_TIME(finish);

   Print_tour(-1, best_tour, "Best tour");
   printf("Cost = %d\n", best_tour->cost);
   printf("Elapsed time = %e seconds\n", finish-start);

#  ifdef STATS
   printf("Stack splits = %d\n", stack_splits);
#  endif

   free(best_tour->cities);
   free(best_tour);
   free(thread_infos);
   free(thread_handles);
   Free_digraph();
   My_barrier_destroy(bar_str);
   pthread_mutex_destroy(&running_mutex);
   pthread_mutex_destroy(&best_tour_mutex);
   pthread_mutex_destroy(&stack_request.cond_mutex);
   pthread_mutex_destroy(&stack_request.stack_mutex);
   pthread_cond_destroy(&stack_request.cond);
   MPI_Finalize();
   Free_queue(queue);  // TODO find the best place for this
   return 0;
}  /* main */



/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Inform user how to start program and exit
 * In arg:    prog_name
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: mpirun -n <procs> %s <thread_count> "
                   "<digraph file> <min split size>\n",
         prog_name);
   MPI_Finalize();
   exit(0);
}  /* Usage */


/*------------------------------------------------------------------
 * Function:    Proxy_send
 * Purpose:     Acts as a proxy to receive messages and send work to
 *              other processes.
 */
void* Proxy_receive_msg(void* p) {
   while (!process_done) {
      MPI_Status status;
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      switch (status.MPI_TAG) {
         case TAG_DONE:
         {
            running_procs --;
            int tmp;
            MPI_Recv(&tmp, 1, MPI_INT, MPI_ANY_SOURCE, TAG_DONE, MPI_COMM_WORLD,
                  MPI_STATUS_IGNORE);
         }
         break;

         case TAG_BEST_TOUR:
         {
            tour_t t = Alloc_tour(NULL, n);
            if (proc_id == 0) {
               MPI_Recv(t->cities, n, MPI_INT, MPI_ANY_SOURCE, TAG_BEST_TOUR,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else {
               MPI_Bcast(t->cities, n, MPI_INT, 0, MPI_COMM_WORLD);
            }
            Update_best_tour_global(t);
            Free_tour(t, NULL);
         }
         break;

         case TAG_WORK:
         break;

         default:
            printf("Unexpected TAG received\n");
      }
      //...
   }
   return NULL;
}


/*------------------------------------------------------------------
 * Function:    Proxy_receive
 * Purpose:     Acts as a proxy to receive messages from other processes.
 */
void* Proxy_request_work(void* p) {
   while (running_procs > 1) {
      if (stack_request.stack != NULL && running_threads == 0) {
         if (pthread_mutex_trylock(&stack_request.cond_mutex) == 0) {
            // MPI_Send ...
            // MPI_Recv ...
            // Copy received data to stack_request.stack
            stack_request.stack = NULL;
            pthread_cond_signal(&stack_request.cond);
            pthread_mutex_unlock(&stack_request.cond_mutex);
         }
         else {
            printf("Requesting work from another process would deadlock!\n");
            while(1);
         }
      }
   }
   return NULL;
}


/*------------------------------------------------------------------
 * Function:    Send_work_if_needed
 * Purpose:     Fulfils work requests from other threads.
 * In arg:
 *    stack:    Thread stack
 */
void Send_work_if_needed(my_stack_t stack, int my_rank) {
   if (stack_request.stack != NULL && stack->list_sz >= min_split_sz) {
      if (pthread_mutex_trylock(&stack_request.cond_mutex) == 0) {
         Split_stack(stack, stack_request.stack, my_rank);
         stack_request.stack = NULL;
         pthread_cond_signal(&stack_request.cond);
         pthread_mutex_unlock(&stack_request.cond_mutex);
      }
   }
}

/*------------------------------------------------------------------
 * Function:    Request_work
 * Purpose:     Requests tours from other threads or processes.
 * Out arg:
 *    stack:    thread stack
 */
void Request_work(my_stack_t stack, long my_rank) {
   pthread_mutex_lock(&running_mutex);
   running_threads --;
   pthread_mutex_unlock(&running_mutex);

   if (running_threads == 0) {
      pthread_mutex_lock(&stack_request.cond_mutex);
      pthread_cond_signal(&stack_request.cond);
      pthread_mutex_unlock(&stack_request.cond_mutex);
      return;
   }

   // Wait to be able to request
   pthread_mutex_lock(&stack_request.stack_mutex);

   // Check if the program is done
   if (running_threads == 0) {
      pthread_mutex_unlock(&stack_request.stack_mutex);
      return;
   }

   // Request
   stack_request.stack = stack;

   // Wait for response
   pthread_mutex_lock(&stack_request.cond_mutex);
   pthread_cond_wait(&stack_request.cond, &stack_request.cond_mutex);

   // Check if this thread is done
   if (!Empty_stack(stack)) {
      pthread_mutex_lock(&running_mutex);
      running_threads ++;
      pthread_mutex_unlock(&running_mutex);
   }

   // Free the request
   pthread_mutex_unlock(&stack_request.cond_mutex);
   pthread_mutex_unlock(&stack_request.stack_mutex);
}

/*------------------------------------------------------------------
 * Function:    Get_work
 * Purpose:     Gets the next tour from the stack or requests tours
 *              from other threads / processes.
 * In/Out arg:
 *    stack:    thread stack
 * Ret val:     Next tour
 */
tour_t Get_work(my_stack_t stack, long my_rank) {
   if (Empty_stack(stack)) {
      Request_work(stack, my_rank);
      Print_stack(stack, my_rank, "Received");
   }

   return Pop(stack);
}

/*------------------------------------------------------------------
 * Function:    Par_tree_search
 * Purpose:     Search function executed on each thread
 * In arg:
 *    rank:     thread rank
 * Globals in:
 *    n:        total number of cities in the problem
 * Notes:
 * 1. The Update_best_tour function will modify the global var
 *    best_tour
 */
void* Par_tree_search(void* rank) {
   thread_info_t* info = (thread_info_t*)rank;
   long my_rank = info->rank;
   city_t nbr;
   my_stack_t stack = info->stack;  // Stack for searching
   my_stack_t avail;  // Stack for unused tours
   tour_t curr_tour;

   avail = Init_stack(n);
   Partition_tree(my_rank, stack);

   while ((curr_tour = Get_work(stack, my_rank))) {
      Send_work_if_needed(stack, my_rank);
      if (City_count(curr_tour) == n) {
         if (Best_tour(curr_tour)) {
            Update_best_tour_global(curr_tour);
         }
      } else {
         for (nbr = n-1; nbr >= 1; nbr--)
            if (Feasible(curr_tour, nbr)) {
               Add_city(curr_tour, nbr);
               Push_copy(stack, curr_tour, avail);
               Remove_last_city(curr_tour);
            }
      }
      Free_tour(curr_tour, avail);
   }
   Free_stack(avail);
   Free_stack(stack);

   return NULL;
}  /* Par_tree_search */


/*------------------------------------------------------------------
 * Function:  Partition_tree
 * Purpose:   Assign each thread its initial collection of subtrees
 * In arg:
 *    my_rank
 * Out args:
 *    stack:  stack will store each thread's initial tours
 *
 * Global scratch:
 *    queue_size
 *    queue
 *
 */
void Partition_tree(long my_rank, my_stack_t stack) {
   int my_first_tour, my_last_tour, i;

   if (my_rank == 0) queue_size = Get_upper_bd_queue_sz();
   My_barrier(bar_str); // TODO: Only used in this function
   printf("Th %ld > queue_size = %d\n", my_rank, queue_size);

   if (queue_size == 0) pthread_exit(NULL);

   if (my_rank == 0) Build_initial_queue();
   My_barrier(bar_str);
   Set_init_tours(my_rank, &my_first_tour, &my_last_tour);

   printf("Th %ld > init_tour_count = %d, first = %d, last = %d\n", 
         my_rank, init_tour_count, my_first_tour, my_last_tour);

   for (i = my_last_tour; i >= my_first_tour; i--) {

      Print_tour(my_rank, Queue_elt(queue,i), "About to push");

      Push(stack, Queue_elt(queue,i));
   }
   Print_stack(stack, my_rank, "After set up");
}  /* Partition_tree */

/*------------------------------------------------------------------
 * Function:   Set_init_tours
 * Purpose:    Determine which tours in the initial queue should be
 *             assigned to this thread
 * In arg:
 *    my_rank
 * Out args:
 *    my_first_tour_p
 *    my_last_tour_p
 * Globals in:
 *    proc_threads
 *    init_tour_count:  the number of tours in the initial queue
 *
 * Note:  A block partition is used.
 */
void Set_init_tours(long my_rank, int* my_first_tour_p,
      int* my_last_tour_p) {
   int quotient, remainder, my_count;

   quotient = init_tour_count/(procs*proc_threads);
   remainder = init_tour_count % (procs*proc_threads);
   if (my_rank < remainder) {
      my_count = quotient+1;
      *my_first_tour_p = my_rank*my_count;
   } else {
      my_count = quotient;
      *my_first_tour_p = my_rank*my_count + remainder;
   }
   *my_last_tour_p = *my_first_tour_p + my_count - 1;
}   /* Set_init_tours */

/*------------------------------------------------------------------
 * Function:  Build_initial_queue
 * Purpose:   Build queue of tours to be divided among processes/threads.
 *            The queue should have at least one tour for each thread on
 *            each process.
 * Global Scratch:
 *    queue
 * Global Out
 *    init_queue_size
 *
 * Note:  Called by 1 thread on each process
 */
void Build_initial_queue(void) {
   int curr_sz = 0;
   city_t nbr;
   tour_t tour = Alloc_tour(NULL, n);

   Init_tour(tour, 0);
   queue = Init_queue(2*queue_size);

   /* Breadth-first search */
   Enqueue(queue, tour);  // Enqueues a copy
   Free_tour(tour, NULL);
   curr_sz++;
   while (curr_sz < procs * proc_threads) {
      tour = Dequeue(queue);
      curr_sz--;
      for (nbr = 1; nbr < n; nbr++)
         if (!Visited(tour, nbr)) {
            Add_city(tour, nbr);
            Enqueue(queue, tour);
            curr_sz++;
            Remove_last_city(tour);
         }
      Free_tour(tour, NULL);
   }
   init_tour_count = curr_sz;

   Print_queue(queue, 0, "Initial queue");
}  /* Build_initial_queue */

/*------------------------------------------------------------------
 * Function:    Get_upper_bd_queue_sz
 * Purpose:     Determine the number of tours needed so that
 *              each thread/process gets at least one and a level
 *              of the tree is fully expanded.  Used as upper
 *              bound when building initial queue and used as
 *              test to see if there are too many threads for
 *              the problem size
 * Globals In:
 *    procs:         number of processes
 *    proc_threads:  number of threads per process
 *    n:             number of cities
 *
 */
int Get_upper_bd_queue_sz(void) {
   int fact = n-1;
   int size = n-1;

   while (size < procs * proc_threads) {
      fact++;
      size *= fact;
   }

   if (size > Factorial(n-1)) {
      fprintf(stderr, "You really shouldn't use so many threads for");
      fprintf(stderr, "such a small problem\n");
      size = 0;
   }
   return size;
}  /* Get_upper_bd_queue_sz */

/*------------------------------------------------------------------
 * Function:    Factorial
 * Purpose:     Compute k!
 * In arg:      k
 * Ret val:     k!
 */
long long Factorial(int k) {
   long long tmp = 1;
   int i;

   for (i = 2; i <= k; i++)
      tmp *= i;
   return tmp;
}  /* Factorial */

/*------------------------------------------------------------------
 * Function:  My_barrier_init
 * Purpose:   Initialize data members of barrier struct
 * In arg:
 *    thr_count:  number of threads that will use this barrier
 * Ret val:
 *    Pointer to initialized barrier struct
 */
my_barrier_t My_barrier_init(int thr_count) {
   my_barrier_t bar = malloc(sizeof(barrier_struct));
   bar->curr_tc = 0;
   bar->max_tc = thr_count;
   pthread_mutex_init(&bar->mutex, NULL);
   pthread_cond_init(&bar->ok_to_go, NULL);

   return bar;
}  /* My_barrier_init */

/*------------------------------------------------------------------
 * Function:  My_barrier_destroy
 * Purpose:   Free barrier struct and its members
 * Out arg:   bar
 */
void My_barrier_destroy(my_barrier_t bar) {
   pthread_mutex_destroy(&bar->mutex);
   pthread_cond_destroy(&bar->ok_to_go);
   free(bar);
}  /* My_barrier_destroy */

/*------------------------------------------------------------------
 * Function:  My_barrier
 * Purpose:   Implement a barrier using a condition variable
 * In/out arg:  bar
 */
void My_barrier(my_barrier_t bar) {
   pthread_mutex_lock(&bar->mutex);
   bar->curr_tc++;
   if (bar->curr_tc == bar->max_tc) {
      bar->curr_tc = 0;
      pthread_cond_broadcast(&bar->ok_to_go);
   } else {
      // Wait unlocks mutex and puts thread to sleep.
      //    Put wait in while loop in case some other
      // event awakens thread.
      while (pthread_cond_wait(&bar->ok_to_go,
             &bar->mutex) != 0);
      // Mutex is relocked at this point.
   }
   pthread_mutex_unlock(&bar->mutex);

}  /* My_barrier */

/*------------------------------------------------------------------
 * Function:  Split_stack
 * Purpose:   Return a pointer to a new stack, nonempty stack
 *            created by taking half the records on the input stack
 * In/out arg:  stack
 * Out arg:     dst_stack
 */
void Split_stack(my_stack_t stack, my_stack_t dst_stack, long my_rank) {
   int new_src, new_dest, old_src, old_dest;

   //Print_stack(stack, my_rank, "Original old stack");
   //Print_stack(dst_stack, my_rank, "Original dst stack");
   if (dst_stack->list_sz != 0) {
      printf("Split stack panic!\n");
      raise(SIGINT);
   }

   new_dest = 0;
   old_dest = 1;
   for (new_src = 1; new_src < stack->list_sz; new_src += 2) {
      old_src = new_src+1;
      dst_stack->list[new_dest++] = stack->list[new_src];
      if (old_src < stack->list_sz)
         stack->list[old_dest++] = stack->list[old_src];
   }

   stack->list_sz = old_dest;
   dst_stack->list_sz = new_dest;

   //Print_stack(stack, my_rank, "Updated old stack");
   //Print_stack(dst_stack, my_rank, "New stack");
}  /* Split_stack */

/*------------------------------------------------------------------
 * Function:    Best_tour
 * Purpose:     Determine whether addition of the hometown to the 
 *              n-city input tour will lead to a best tour.
 * In arg:
 *    tour:     tour visiting all n cities
 * Ret val:
 *    TRUE if best tour, FALSE otherwise
 */
int Best_tour(tour_t tour) {
   cost_t cost_so_far = Tour_cost(tour);
   city_t last_city = Last_city(tour);

   if (cost_so_far + Cost(last_city, home_town) < Tour_cost(best_tour))
      return TRUE;
   else
      return FALSE;
}  /* Best_tour */


/*------------------------------------------------------------------
 * Function:    Update_best_tour_local
 * Purpose:     Replace the existing best tour with the input tour +
 *              hometown
 * In arg:
 *    tour:     tour that's visited all n-cities
 * Global out:
 *    best_tour:  the current best tour
 * Ret:         TRUE if the best tour was updated, FALSE otherwise.
 * Note:
 * 1. The input tour hasn't had the home_town added as the last
 *    city before the call to Update_best_tour_local.  So we call
 *    Add_city(best_tour, hometown) before returning.
 * 2. The call to Best_tour is necessary.  Without, we can get
 *    the following race:
 *
 *       best_tour_cost = 1000
 *       Time       Th 0                 Th 1
 *       ----       ----                 ----
 *        0         tour->cost = 10
 *        1         Best_tour ret true   tour->cost = 20
 *        2                              Best_tour ret true
 *        3         Lock mutex
 *        4         best_tour_cost = 10  Wait on mutex
 *        5         Unlock mutex
 *        6                              Lock mutex
 *        7                              best_tour_cost = 20
 *        8                              Unlock mutex
 *
 */
int Update_best_tour_local(tour_t tour) {
   int ret = FALSE;
   pthread_mutex_lock(&best_tour_mutex);
   if (Best_tour(tour)) {
      Copy_tour(tour, best_tour);
      Add_city(best_tour, home_town);
      ret = TRUE;
   }
   pthread_mutex_unlock(&best_tour_mutex);
   return ret;
}  /* Update_best_tour_local */

/*------------------------------------------------------------------
 * Function:    Update_best_tour_global
 * Purpose:     Replace the existing best tour with the input tour +
 *              hometown and broadcast it to the other processes
 * In arg:
 *    tour:     tour that's visited all n-cities
 * Global out:
 *    best_tour:  the current best tour
 */
void Update_best_tour_global(tour_t tour) {
   if (Update_best_tour_local(tour)) {
      if (proc_id == 0)
         MPI_Bcast(tour->cities, n, MPI_INT, 0, MPI_COMM_WORLD);
      else
         MPI_Send(tour->cities, n, MPI_INT, 0, TAG_BEST_TOUR, MPI_COMM_WORLD);
   }
}

/*------------------------------------------------------------------
 * Function:  Feasible
 * Purpose:   Check whether nbr could possibly lead to a better
 *            solution if it is added to the current tour.  The
 *            function checks whether nbr has already been visited
 *            in the current tour, and, if not, whether adding the
 *            edge from the current city to nbr will result in
 *            a cost less than the current best cost.
 * In args:   All
 * Global in:
 *    best_tour
 * Return:    TRUE if the nbr can be added to the current tour.
 *            FALSE otherwise
 */
int Feasible(tour_t tour, city_t city) {
   city_t last_city = Last_city(tour);

   if (!Visited(tour, city) &&
        Tour_cost(tour) + Cost(last_city,city) < Tour_cost(best_tour))
      return TRUE;
   else
      return FALSE;
}  /* Feasible */


