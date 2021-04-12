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
OBJ=src/louis_chessx/bitboard.o src/louis_chessx/chessboard_eval.o src/louis_chessx/chessboard_search.o src/louis_chessx/chessboard_format.o src/louis_chessx/chessboard_moves.o src/louis_chessx/game_controller_precomputation.o src/louis_chessx/game_controller_commands.o



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

# liblouis_chessx.a
#
# compile into a static library
liblouis_chessx.a: $(OBJ)
	$(AR) $(ARFLAGS) liblouis_chessx.a $(OBJ)

# liblouis_chessx.so
#
# compile into a dynamic library
liblouis_chessx.so: $(OBJ)
	$(CPP) $(OBJ) -shared -o liblouis_chessx.so

# louis_chessx
#
# compile the louis_chessx binary
louis_chessx: liblouis_chessx.so main.o
	$(CPP) $(CPPFLAGS) -llouis_chessx main.o -o louis_chessx

# install
#
# install the binary and includes
install: louis_chessx liblouis_chessx.a liblouis_chessx.so
	# Make directories
	mkdir -p /usr/include/louis_chessx

	# Install headers and libraries
	install -C -m 644 include/louis_chessx/* /usr/include/louis_chessx
	install -C -m 755 liblouis_chessx.a liblouis_chessx.so /usr/lib

	# Install binary
	install -C -m 755 louis_chessx /bin

	# Configure libraries
	ldconfig /usr/lib

# uninstall
#
# uninstall any installation
uninstall:
	# Remove headers
	rm -rf /usr/include/louis_chessx

	# Remove library
	rm -f /usr/lib/liblouis_chessx.a /usr/lib/liblouis_chessx.so

	# Remove binary
	rm -f /bin/louis_chessx

	# Configure libraries
	ldconfig /usr/lib
