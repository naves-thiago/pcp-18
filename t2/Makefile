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

tests: test_fifo

test_fifo: $(BUILD_DIR)/test_fifo.o $(FIFO)
	gcc -Wall -g $^ -o $@

$(BUILD_DIR)/%.o:%.c
	@mkdir -p $(BUILD_DIR)
	gcc -Wall -g -c -std=c99 -MMD $< -o $@

clean:
	rm -f $(OBJ) $(DEP) pcp test_fifo

-include $(DEP)
