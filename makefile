#!/bin/make

# COMMANDS AND FLAGS

# gcc setup
CC=g++
CFLAGS=-std=c++20 -Iinclude -g -O2 -march=native -flto

# g++ setup
CPP=g++
CPPFLAGS=-std=c++20 -Iinclude -O3 -march=native -flto=auto -pedantic -pthread -latomic

# ar setup
AR=ar
ARFLAGS=-rc

# object files
OBJ=test.o src/chess/bitboard.o src/chess/chessboard_eval.o src/chess/chessboard_search.o src/chess/chessboard_format.o src/chess/chessboard_moves.o src/chess/game_controller_precomputation.o



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
test: $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o test.out 
	#objdump -d  --no-show-raw-insn test.out 2> /dev/null 1> test.dump
	#objdump -dS --no-show-raw-insn test.out 2> /dev/null 1> test.s.dump
	#objdump -d  --no-show-raw-insn --visualize-jumps test.out 2> /dev/null 1> test.v.dump
	#objdump -dS --no-show-raw-insn --visualize-jumps test.out 2> /dev/null 1> test.vs.dump
