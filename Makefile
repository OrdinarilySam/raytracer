-include config.local.mk
ifndef CXX
    CXX=clang
endif
ifndef NUM_INPUTS
    NUM_INPUTS=1
endif

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