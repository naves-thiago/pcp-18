all: quad_bag quad_equal quad_bag_equal

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
CFLAGS:= -Wall -g -O2 -std=c99 -MMD

#MPICC:=/home/thiago/dev/pcp/t2/mpi/bin/mpicc -Wl,-rpath -Wl,/usr/lib64/openmpi/lib
MPICC:=mpicc
.PHONY: clean tests

quad_equal: $(BUILD_DIR)/quad_equal.o $(BUILD_DIR)/quadrature.o
	$(MPICC) $^ -o $@ $(LDFLAGS)

quad_bag_equal: $(BUILD_DIR)/quad_bag_equal.o $(BUILD_DIR)/quadrature.o
	$(MPICC) $^ -o $@ $(LDFLAGS)

quad_bag: $(BUILD_DIR)/quad_bag.o $(BUILD_DIR)/quadrature.o $(FIFO) $(BAG)
	$(MPICC) $^ -o $@ $(LDFLAGS)

tests: test_fifo test_quadrature test_mpi

test_fifo: $(BUILD_DIR)/test_fifo.o $(FIFO)
	gcc $^ -o $@ $(LDFLAGS)

test_bag: $(BUILD_DIR)/test_bag.o $(BAG)
	gcc $^ -o $@ $(LDFLAGS)

test_quadrature: test_quadrature.c $(BUILD_DIR)/quadrature.o
	gcc $^ -o $@ $(LDFLAGS)

test_mpi: test_mpi.c
	$(MPICC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(BUILD_DIR)
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(DEP) test_fifo quad_equal quad_bag_equal quad_bag test_quadrature test_mpi test_bag

-include $(DEP)
