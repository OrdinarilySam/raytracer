-include config.local.mk
ifndef CXX
    CXX=clang
endif

.DEFAULT_GOAL := all

all: raytracer1b

.PHONY: all clean test

raytracer1b: raytracer.o vec3.o
	$(CXX) -o raytracer1b raytracer.o vec3.o

raytracer.o: raytracer.c raytracer.h
	$(CXX) -c raytracer.c

vec3.o: vec3.c vec3.h
	$(CXX) -c vec3.c

test: raytracer1a
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./raytracer1a $(FILE_NAME)$$number.$(FILE_EXT); \
	done

clean:
	rm -f raytracer1* *.ppm *.o