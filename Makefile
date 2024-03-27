# Configuration
-include config/config.default.mk

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

# Zip name
ZIP_NAME = hw1d.sam.martin.zip

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OUT_DIR = out
DATA_DIR = data

DBG_OBJ_DIR = $(OBJ_DIR)/debug

# Find all .c files in SRC_DIR to compile them into .o files in OBJ_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DBGOBJS = $(SRCS:$(SRC_DIR)/%.c=$(DBG_OBJ_DIR)/%.o)

# Default goal
.DEFAULT_GOAL := all

# Targets
all: $(OUT_DIR)/raytracer1d

.PHONY: all clean test run debug zip

# Linking the executable
$(OUT_DIR)/raytracer1d: $(OBJS)
	$(CXX) -lm -o $@ $^

# Compiling source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(INC_DIR)/%.h
	$(CXX) -I $(INC_DIR) -c $< -o $@

# Create the object files directory
$(OBJ_DIR):
	mkdir -p $@

# Create the output directory
$(OUT_DIR):
	mkdir -p $@

# Test target
test: $(OUT_DIR)/raytracer1d
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./$(OUT_DIR)/raytracer1d $(DATA_DIR)/$(FILE_NAME)$$number.$(FILE_EXT); \
	done

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(OUT_DIR)/raytracer1* $(DATA_DIR)/*.ppm 

# Run program with a user-specified input file
# Usage: make run INPUT=filename.txt
run: $(OUT_DIR)/raytracer1d
	@./$(OUT_DIR)/raytracer1d $(DATA_DIR)/$(INPUT)

# Debug target
debug: $(DBGOBJS)
	$(CXX) -g -lm -o $(OUT_DIR)/raytracer1d_debug $^

$(DBG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DBG_OBJ_DIR)
	$(CXX) -g -I $(INC_DIR) -c $< -o $@

# Create the object files directory for debug
$(DBG_OBJ_DIR): $(OBJ_DIR)
	mkdir -p $@

zip:
	zip -r $(ZIP_NAME) $(SRC_DIR) $(INC_DIR) $(DATA_DIR) Makefile config $(OUT_DIR) README.md aloe.jpeg