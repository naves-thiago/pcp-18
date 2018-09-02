all: pcp

pcp: main.o fifo.o
	gcc -Wall -g -lpthread $< -o $@

%.o:%.c
	gcc -Wall -g -c -std=c99 $< -o $@
