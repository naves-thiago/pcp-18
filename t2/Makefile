all: quad_bag

# Object output dir
BUILD_DIR:=build

# Libs
FIFO:=$(BUILD_DIR)/llfifo.o
BAG:=$(BUILD_DIR)/bag.o $(FIFO)

# Dependencies
OBJ:=quad_equal.o quad_bag.o quad_bag_equal.o bag.o llfifo.o quadrature.o
OBJ:=$(addprefix $(BUILD_DIR)/, $(OBJ))
DEP:=$(OBJ:.o=.d)

# Flags
LDFLAGS:=-Wall -g -lm
CFLAGS:=-Wall -g -O0 -std=c99 -MMD

.PHONY: clean tests

quad_equal: $(BUILD_DIR)/quad_equal.o $(BUILD_DIR)/quadrature.o
	mpicc $^ -o $@ $(LDFLAGS)

quad_bag_equal: $(BUILD_DIR)/quad_bag_equal.o $(BUILD_DIR)/quadrature.o
	mpicc $^ -o $@ $(LDFLAGS)

quad_bag: $(BUILD_DIR)/quad_bag.o $(BUILD_DIR)/quadrature.o $(FIFO) $(BAG)
	mpicc $^ -o $@ $(LDFLAGS)

tests: test_fifo test_quadrature test_mpi

test_fifo: $(BUILD_DIR)/test_fifo.o $(FIFO)
	gcc $^ -o $@ $(LDFLAGS)

test_bag: $(BUILD_DIR)/test_bag.o $(BAG)
	gcc $^ -o $@ $(LDFLAGS)

test_quadrature: test_quadrature.c $(BUILD_DIR)/quadrature.o
	gcc $^ -o $@ $(LDFLAGS)

test_mpi: test_mpi.c
	mpicc $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o:%.c
	@mkdir -p $(BUILD_DIR)
	mpicc -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(DEP) test_fifo quad_equal quad_bag_equal quad_bag test_quadrature test_mpi test_bag

-include $(DEP)
