#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tour.h"
#include "digraph.h"
#include "debug.h"

static const int FALSE = 0;
static const int TRUE = 1;
static const int NO_CITY = -1;
static const int MAX_STRING = 1000;

/*------------------------------------------------------------------
 * Function:  Init_tour
 * Purpose:   Initialize the data members of allocated tour
 * In args:
 *    cost:   initial cost of tour
 * Out arg:
 *    tour
 */
void Init_tour(tour_t tour, cost_t cost) {
   int i;

   tour->cities[0] = 0;
   for (i = 1; i <= tour->length; i++) {
      tour->cities[i] = NO_CITY;
   }
   tour->cost = cost;
   tour->count = 1;
}  /* Init_tour */

/*------------------------------------------------------------------
 * Function:  Alloc_tour
 * Purpose:   Allocate memory for a tour and its members
 * In/out arg:
 *    avail:  stack storing unused tours
 * In arg:
 *    length: number of cities
 * Ret val:   Pointer to a tour_struct with storage allocated for its
 *            members
 */
tour_t Alloc_tour(my_stack_t avail, int length) {
   tour_t tmp;

   if (avail == NULL || Empty_stack(avail)) {
      tmp = malloc(sizeof(tour_struct));
      tmp->cities = malloc((length+1)*sizeof(city_t));
      tmp->length = length;
      return tmp;
   } else {
      return Pop(avail);
   }
}  /* Alloc_tour */

/*------------------------------------------------------------------
 * Function:  Free_tour
 * Purpose:   Free a tour
 * In/out arg:
 *    avail
 * Out arg:
 *    tour
 */
void Free_tour(tour_t tour, my_stack_t avail) {
   if (avail == NULL) {
      free(tour->cities);
      free(tour);
   } else {
      Push(avail, tour);
   }
}  /* Free_tour */

/*------------------------------------------------------------------
 * Function:  Print_tour
 * Purpose:   Print a tour
 * In args:   All
 * Notes:
 * 1.  Copying the tour to a string makes it less likely that the 
 *     output will be broken up by another process/thread
 * 2.  Passing a negative value for my_rank will cause the rank
 *     to be omitted from the output
 */
void Print_tour(long my_rank, tour_t tour, char* title) {
   int i;
   char string[MAX_STRING];

   if (my_rank >= 0)
      sprintf(string, "Proc %ld > %s %p: ", my_rank, title, tour);
   else
      sprintf(string, "%s: ", title);
   for (i = 0; i < City_count(tour); i++)
      sprintf(string + strlen(string), "%d ", Tour_city(tour,i));
   printf("%s\n\n", string);
}  /* Print_tour */

/*------------------------------------------------------------------
 * Function:   Copy_tour
 * Purpose:    Copy tour1 into tour2
 * In arg:
 *    tour1
 * Out arg:
 *    tour2
 */
void Copy_tour(tour_t tour1, tour_t tour2) {
// int i;

   memcpy(tour2->cities, tour1->cities, (tour1->length+1)*sizeof(city_t));
// for (i = 0; i <= n; i++)
//   tour2->cities[i] =  tour1->cities[i];
   tour2->count = tour1->count;
   tour2->cost = tour1->cost;
}  /* Copy_tour */

/*------------------------------------------------------------------
 * Function:  Add_city
 * Purpose:   Add city to the end of tour
 * In arg:
 *    city
 * In/out arg:
 *    tour
 * Note: This should only be called if tour->count >= 1.
 */
void Add_city(tour_t tour, city_t new_city) {
   city_t old_last_city = Last_city(tour);
   tour->cities[tour->count] = new_city;
   (tour->count)++;
   tour->cost += Cost(old_last_city,new_city);
}  /* Add_city */

/*------------------------------------------------------------------
 * Function:  Remove_last_city
 * Purpose:   Remove last city from end of tour
 * In/out arg:
 *    tour
 * Note:
 *    Function assumes there are at least two cities on the tour --
 *    i.e., the hometown in tour->cities[0] won't be removed.
 */
void Remove_last_city(tour_t tour) {
   city_t old_last_city = Last_city(tour);
   city_t new_last_city;

   tour->cities[tour->count-1] = NO_CITY;
   (tour->count)--;
   new_last_city = Last_city(tour);
   tour->cost -= Cost(new_last_city,old_last_city);
}  /* Remove_last_city */

/*------------------------------------------------------------------
 * Function:   Visited
 * Purpose:    Use linear search to determine whether city has already
 *             been visited on the current tour.
 * In args:    All
 * Return val: TRUE if city has already been visited.
 *             FALSE otherwise
 */
int Visited(tour_t tour, city_t city) {
   int i;

   for (i = 0; i < City_count(tour); i++)
      if ( Tour_city(tour,i) == city ) return TRUE;
   return FALSE;
}  /* Visited */


/******************************* STACK ***************************/


/*------------------------------------------------------------------
 * Function: Init_stack
 * Purpose:  Allocate storage for a new stack and initialize members
 * Out arg:  stack_p
 */
my_stack_t Init_stack(int size) {
   int i;

   my_stack_t stack = malloc(sizeof(stack_struct));
   stack->list = malloc(size*size*sizeof(tour_t));
   for (i = 0; i < size*size; i++)
      stack->list[i] = NULL;
   stack->list_sz = 0;
   stack->list_alloc = size*size;

   return stack;
}  /* Init_stack */


/*------------------------------------------------------------------
 * Function:    Push
 * Purpose:     Push a tour pointer onto the stack
 * In arg:      tour
 * In/out arg:  stack
 */
void Push(my_stack_t stack, tour_t tour) {
   if (stack->list_sz == stack->list_alloc) {
      dbg_printf(stderr, "Stack overflow in Push!\n");
      free(tour->cities);
      free(tour);
   } else {
      dbg_printf("In Push, list_sz = %d, pushing %p and %p\n",
            stack->list_sz, tour, tour->cities);
      //Print_tour(-1, tour, "About to be pushed onto stack");
      dbg_printf("\n");

      stack->list[stack->list_sz] = tour;
      (stack->list_sz)++;
   }
}  /* Push */

/*------------------------------------------------------------------
 * Function:    Push_copy
 * Purpose:     Push a copy of tour onto the top of the stack
 * In arg:      tour
 * In/out arg:
 *    stack
 *    avail
 * Error:       If the stack is full, print an error and exit
 */
void Push_copy(my_stack_t stack, tour_t tour, my_stack_t avail) {
   tour_t tmp;

   if (stack->list_sz == stack->list_alloc) {
      dbg_printf(stderr, "Stack overflow!\n");
      exit(-1);
   }
   tmp = Alloc_tour(avail, tour->length);
   Copy_tour(tour, tmp);
   stack->list[stack->list_sz] = tmp;
   (stack->list_sz)++;
}  /* Push_copy */


/*------------------------------------------------------------------
 * Function:  Pop
 * Purpose:   Reduce the size of the stack by returning the top
 * In arg:    stack
 * Ret val:   The tour on the top of the stack
 * Error:     If the stack is empty, print a message and exit
 */
tour_t Pop(my_stack_t stack) {
   tour_t tmp;

   if (stack->list_sz == 0) {
      return NULL;
   }
   tmp = stack->list[stack->list_sz-1];
   stack->list[stack->list_sz-1] = NULL;
   (stack->list_sz)--;
   return tmp;
}  /* Pop */


/*------------------------------------------------------------------
 * Function:  Empty_stack
 * Purpose:   Determine whether the stack is empty
 * In arg:    stack
 * Ret val:   TRUE if empty, FALSE otherwise
 *
 * Note:      As long as min_split_sz is >= 2, splitting the
 *            stack can't empty the stack.  So the member
 *            list_sz will only be set to 0 by the thread
 *            that owns the stack.
 */
int  Empty_stack(my_stack_t stack) {
   if (stack->list_sz == 0)
      return TRUE;
   else
      return FALSE;
}  /* Empty_stack */


/*------------------------------------------------------------------
 * Function:  Free_stack
 * Purpose:   Free a stack and its members
 * Out arg:   stack
 */
void Free_stack(my_stack_t stack) {
   int i;

   for (i = 0; i < stack->list_sz; i++) {
      free(stack->list[i]->cities);
      free(stack->list[i]);
   }
   free(stack->list);
   free(stack);
}  /* Free_stack */

/*------------------------------------------------------------------
 * Function:  Print_stack
 * Purpose:   Print contents of stack for debugging
 * In args:   all
 */
void Print_stack(my_stack_t stack, long my_rank, char title[]) {
   char string[MAX_STRING];
   int i, j;

   printf("Th %ld > %s\n", my_rank, title);
   for (i = 0; i < stack->list_sz; i++) {
      sprintf(string, "Th %ld > ", my_rank);
      for (j = 0; j < stack->list[i]->count; j++)
         sprintf(string + strlen(string), "%d ", stack->list[i]->cities[j]);
      printf("%s\n", string);
   }
}  /* Print_stack */

/*------------------------------------------------------------------
 * Function:   Make_tour_from_msg
 * Purpose:    Fills a tour_t struct with the cities from msg.
 * msg format: msg[0] = count, msg[1..] cities
 * In args:    msg
 * Out args:   tour
 */
void Make_tour_from_msg(tour_t tour, int * msg) {
   tour->count = msg[0];
   tour->cost = 0;
   msg ++;
   tour->cities[0] = msg[0];
   for (int i=1; i<tour->count; i++) {
      tour->cities[i] = msg[i];
      tour->cost += Cost(msg[i-1], msg[i]);
   }
}
