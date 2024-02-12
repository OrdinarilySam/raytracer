-include config.local.mk
ifndef CXX
    CXX=clang
endif

.DEFAULT_GOAL := all

all: raytracer1b

.PHONY: all clean test

raytracer1b: main.o vec3.o readscene.o shaderay.o tracerays.o 
	$(CXX) -o raytracer1b main.o vec3.o readscene.o shaderay.o tracerays.o 

main.o: main.c main.h typedefs.h vec3.o
	$(CXX) -c main.c

vec3.o: vec3.c vec3.h
	$(CXX) -c vec3.c

readscene.o: readscene.c readscene.h typedefs.h vec3.o
	$(CXX) -c readscene.c

shaderay.o: shaderay.c shaderay.h typedefs.h vec3.o
	$(CXX) -c shaderay.c

tracerays.o: tracerays.c tracerays.h typedefs.h vec3.o shaderay.o
	$(CXX) -c tracerays.c

test: raytracer1b
	@for number in $$(seq 1 $(NUM_INPUTS)); do \
		./raytracer1b $(FILE_NAME)$$number.$(FILE_EXT); \
	done

clean:
	rm -f raytracer1* *.ppm *.o