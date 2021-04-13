#!/bin/make

# COMMANDS AND FLAGS

# gcc setup
CC=g++
CFLAGS=-std=c++20 -Iinclude -g -O2 -march=native -flto

# g++ setup
CPP=g++
CPPFLAGS=-std=c++20 -Iinclude -L. -O3 -march=native -flto=auto -pedantic -pthread -latomic

# ar setup
AR=ar
ARFLAGS=-rc

# object files
OBJ=src/louischessx/bitboard.o src/louischessx/chessboard_eval.o src/louischessx/chessboard_search.o src/louischessx/chessboard_format.o src/louischessx/chessboard_moves.o src/louischessx/game_controller_precomputation.o src/louischessx/game_controller_commands.o



# USEFUL TARGETS

# all
#
# make libraries and binary
all: liblouischessx.a liblouischessx.so louischessx

# clean
#
# remove all object files and libraries
.PHONY: clean
clean:
	find . -type f -name "*\.o" -delete -print
	find . -type f -name "*\.a" -delete -print
	find . -type f -name "*\.so" -delete -print



# COMPILATION TARGETS

# liblouischessx.a
#
# compile into a static library
liblouischessx.a: $(OBJ)
	$(AR) $(ARFLAGS) liblouischessx.a $(OBJ)

# liblouischessx.so
#
# compile into a dynamic library
liblouischessx.so: $(OBJ)
	$(CPP) $(OBJ) -shared -o liblouischessx.so

# louischessx
#
# compile the louischessx binary
louischessx: liblouischessx.so main.o
	$(CPP) $(CPPFLAGS) -llouischessx main.o -o louischessx

# install
#
# install the binary and includes
install: all
	# Make directories
	mkdir -p /usr/include/louischessx

	# Install headers and libraries
	install -C -m 644 include/louischessx/* /usr/include/louischessx
	install -C -m 755 liblouischessx.a liblouischessx.so /usr/lib

	# Install binary
	install -C -m 755 louischessx /usr/bin

	# Configure libraries
	ldconfig /usr/lib

# uninstall
#
# uninstall any installation
uninstall:
	# Remove headers
	rm -rf /usr/include/louischessx

	# Remove library
	rm -f /usr/lib/liblouischessx.a /usr/lib/liblouischessx.so

	# Remove binary
	rm -f /usr/bin/louischessx

	# Configure libraries
	ldconfig /usr/lib
