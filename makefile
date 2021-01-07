#!/bin/make

# COMMANDS AND FLAGS

# gcc setup
CC=g++
CFLAGS=-std=c++20 -Iinclude -g -O2 -march=corei7 -flto

# g++ setup
CPP=g++
CPPFLAGS=-std=c++20 -Iinclude -g -O2 -march=corei7 -flto

# ar setup
AR=ar
ARFLAGS=-rc



# USEFUL TARGETS

# all
#
# make test
all: test

# clean
#
# remove all object files and libraries
.PHONY: clean
clean:
	find . -type f -name "*\.o" -delete -print
	find . -type f -name "*\.a" -delete -print
	find . -type f -name "*\.so" -delete -print



# COMPILATION TARGETS

# test
#
# compile the test source
test: test.o src/chess/bitboard.o
	$(CPP) $(CPPFLAGS) test.o src/chess/bitboard.o -o test.out
