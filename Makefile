all: pcp

pcp: main.o fifo.o
	gcc -Wall -g -lpthread $^ -o $@

test_sem: test_sem.o sem.o
	gcc -Wall -g -lpthread $^ -o $@

%.o:%.c
	gcc -Wall -g -c -std=c99 $< -o $@
