CXX=clang
NUM_INPUTS=2
FILE_NAME = example
FILE_EXT = txt

.DEFAULT_GOAL := all

all: raytracer 

.PHONY: all clean test

raytracer: raytracer.c raytracer.h
	$(CXX) raytracer.c -o raytracer

test: raytracer
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./raytracer $(FILE_NAME)$$number.$(FILE_EXT); \
	done

clean:
	rm -f raytracer *.ppm