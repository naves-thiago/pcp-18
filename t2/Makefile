all: pcp

# Object output dir
BUILD_DIR:=build

# Libs
FIFO:=$(BUILD_DIR)/fifo.o

# Dependencies
OBJ:=main.o fifo.o
OBJ:=$(addprefix $(BUILD_DIR)/, $(OBJ))
DEP:=$(OBJ:.o=.d)

.PHONY: clean tests

pcp: $(BUILD_DIR)/main.o $(FIFO)
	gcc -Wall -g -lpthread $^ -o $@

tests: test_fifo test_quadrature test_mpi

test_fifo: $(BUILD_DIR)/test_fifo.o $(FIFO)
	gcc -Wall -g $^ -o $@

test_quadrature: test_quadrature.c
	gcc -Wall -g -O0 $^ -o $@ -lm

test_mpi: test_mpi.c
	mpicc -Wall $^ -o $@

$(BUILD_DIR)/%.o:%.c
	@mkdir -p $(BUILD_DIR)
	gcc -Wall -g -c -std=c99 -MMD $< -o $@

clean:
	rm -f $(OBJ) $(DEP) pcp test_fifo

-include $(DEP)
