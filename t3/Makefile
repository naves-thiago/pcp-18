all: tsp

# Object output dir
BUILD_DIR:=build

# Dependencies
OBJ:=tsp.o queue.o tour.o digraph.o
OBJ:=$(addprefix $(BUILD_DIR)/, $(OBJ))
DEP:=$(OBJ:.o=.d)

# Flags
LDFLAGS:=-Wall -g -lpthread
CFLAGS:= -Wall -g -O2 -std=c99 -MMD

#MPICC:=/home/thiago/dev/pcp/t2/mpi/bin/mpicc -Wl,-rpath -Wl,/usr/lib64/openmpi/lib
MPICC:=mpicc
#MPICC:=gcc
.PHONY: clean

tsp: $(OBJ)
	$(MPICC) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(BUILD_DIR)
	$(MPICC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(DEP) tsp

-include $(DEP)
