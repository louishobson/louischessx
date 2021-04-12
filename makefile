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
OBJ=main.o src/chess/bitboard.o src/chess/chessboard_eval.o src/chess/chessboard_search.o src/chess/chessboard_format.o src/chess/chessboard_moves.o src/chess/game_controller_precomputation.o src/chess/game_controller_commands.o



# USEFUL TARGETS

# all
#
# make louis_chessx
all: louis_chessx

# clean
#
# remove all object files and libraries
.PHONY: clean
clean:
	find . -type f -name "*\.o" -delete -print
	find . -type f -name "*\.a" -delete -print
	find . -type f -name "*\.so" -delete -print



# COMPILATION TARGETS

# louis_chessx
#
# compile the louis_chessx binary
louis_chessx: $(OBJ)
	$(CPP) $(CPPFLAGS) $(OBJ) -o louis_chessx
