#!/bin/make

# COMMANDS AND FLAGS

# gcc setup
CC=g++
CFLAGS=-std=c++20 -Iinclude -g -O2 -march=native -flto

# g++ setup
CPP=g++
CPPFLAGS=-std=c++20 -Iinclude -g -O3 -march=native -flto -pedantic

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
test: test.o src/chess/bitboard.o src/chess/chessboard.o
	$(CPP) $(CPPFLAGS) test.o src/chess/bitboard.o src/chess/chessboard.o -o test.out 
	objdump -d  test.out 2> /dev/null 1> test.dump
	objdump -dS test.out 2> /dev/null 1> test.s.dump
