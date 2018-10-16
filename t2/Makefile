all: pcp

# Object output dir
BUILD_DIR:=build

# Libs
FIFO:=$(BUILD_DIR)/llfifo.o

# Dependencies
OBJ:=equal.o bag.o bag_equal.o llfifo.o quadrature.o
OBJ:=$(addprefix $(BUILD_DIR)/, $(OBJ))
DEP:=$(OBJ:.o=.d)

# Flags
LDFLAGS:=-Wall -g -lm

.PHONY: clean tests

equal: $(BUILD_DIR)/equal.o $(BUILD_DIR)/quadrature.o
	mpicc $^ -o $@ $(LDFLAGS)

bag_equal: $(BUILD_DIR)/bag_equal.o $(BUILD_DIR)/quadrature.o
	mpicc $^ -o $@ $(LDFLAGS)

bag: $(BUILD_DIR)/bag.o $(FIFO) $(BUILD_DIR)/quadrature.o
	mpicc $^ -o $@ $(LDFLAGS)

tests: test_fifo test_quadrature test_mpi

test_fifo: $(BUILD_DIR)/test_fifo.o $(FIFO)
	gcc $^ -o $@ $(LDFLAGS)

test_quadrature: test_quadrature.c $(BUILD_DIR)/quadrature.o
	gcc $^ -o $@ $(LDFLAGS)

test_mpi: test_mpi.c
	mpicc $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o:%.c
	@mkdir -p $(BUILD_DIR)
	mpicc -Wall -g -c -O2 -std=c99 -MMD $< -o $@

clean:
	rm -f $(OBJ) $(DEP) pcp test_fifo equal bag_equal test_quadrature test_mpi

-include $(DEP)
