CXX=clang
NUM_INPUTS=2

.DEFAULT_GOAL := all

all: raytracer 

.PHONY: all clean test

raytracer: raytracer.c raytracer.h
	$(CXX) raytracer.c -o raytracer

test: raytracer
	for number in $$(seq 1 $(NUM_INPUTS)); do \
		./raytracer input$$number.txt; \
	done

clean:
	rm -f raytracer *.ppm