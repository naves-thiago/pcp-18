#pragma once

typedef int city_t;
typedef int cost_t;
typedef struct {
   city_t* cities; /* Cities in partial tour           */
   int count;      /* Number of cities in partial tour */
   cost_t cost;    /* Cost of partial tour             */
   int length;     /* Max number of cities             */
} tour_struct;
typedef tour_struct* tour_t;

typedef struct {
   tour_t* list;
   int list_sz;
   int list_alloc;
}  stack_struct;
typedef stack_struct* my_stack_t;

#define City_count(tour) (tour->count)
#define Tour_cost(tour) (tour->cost)
#define Last_city(tour) (tour->cities[(tour->count)-1])
#define Tour_city(tour,i) (tour->cities[(i)])
#define Tour_length(tour) (tour->length)

void Init_tour(tour_t tour, cost_t cost);
tour_t Alloc_tour(my_stack_t avail, int length);
void Free_tour(tour_t tour, my_stack_t avail);
void Print_tour(long my_rank, tour_t tour, char* title);
void Copy_tour(tour_t tour1, tour_t tour2);
void Add_city(tour_t tour, city_t);
void Remove_last_city(tour_t tour);
int  Visited(tour_t tour, city_t city);


my_stack_t Init_stack(int size);
void Push(my_stack_t stack, tour_t tour);  // Push pointer
void Push_copy(my_stack_t stack, tour_t tour, my_stack_t avail); 
tour_t Pop(my_stack_t stack);
int  Empty_stack(my_stack_t stack);
void Free_stack(my_stack_t stack);
void Print_stack(my_stack_t stack, long my_rank, char title[]);

