-include config.default.mk
ifndef CXX
    CXX=clang
endif

.DEFAULT_GOAL := all

all: raytracer1a

.PHONY: all clean test

raytracer1a: raytracer.c raytracer.h
	$(CXX) raytracer.c -o raytracer1a

test: raytracer1a
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./raytracer1a $(FILE_NAME)$$number.$(FILE_EXT); \
	done

clean:
	rm -f raytracer1a *.ppm