# Configuration
-include config/config.local.mk

# Testing configuration
ifndef NUM_INPUTS
	NUM_INPUTS = 1
endif

ifndef FILE_NAME
	FILE_NAME = example
endif

ifndef FILE_EXT
	FILE_EXT = txt
endif

# Compiler
CXX = clang
CXXFLAGS = -g

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OUT_DIR = out
DATA_DIR = data

# Find all .c files in SRC_DIR to compile them into .o files in OBJ_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default goal
.DEFAULT_GOAL := all

# Targets
all: $(OUT_DIR)/raytracer1c

.PHONY: all clean test run

# Linking the executable
$(OUT_DIR)/raytracer1c: $(OBJS)
	$(CXX) -g -lm -o $@ $^

# Compiling source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CXX) -g -I $(INC_DIR) -c $< -o $@

# Create the object files directory
$(OBJ_DIR):
	mkdir -p $@

# Create the output directory
$(OUT_DIR):
	mkdir -p $@

# Test target
test: $(OUT_DIR)/raytracer1c
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./$(OUT_DIR)/raytracer1b $(DATA_DIR)/$(FILE_NAME)$$number.$(FILE_EXT); \
	done

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)/raytracer1* $(DATA_DIR)/*.ppm $(DATA_DIR)/*.jpeg

# Run program with a user-specified input file
# Usage: make run INPUT=filename.txt
run: $(OUT_DIR)/raytracer1c
	@./$(OUT_DIR)/raytracer1c $(DATA_DIR)/$(INPUT)
