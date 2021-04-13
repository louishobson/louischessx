# louischessx

A C++ chess engine designed to be an xboard backend.

## Supported OSs and Dependancies

Requires GCC 10 on GNU/Linux-based systems (due to C++20 dependant features in the source).
However, since the makefile is very simple, feel free to create a new one for compilation using Clang or even on Windows.

Although the program is functional as standalone software, it is best used with XBoard or WinBoard.

## How to Install

To download, build and install:

```
$ cd ~/Downloads
$ git clone https://github.com/louishobson/louischessx.git
$ cd louischessx
$ make
$ make install
```

To uninstall:

```
$ cd louischessx
$ make uninstall
```

Note that the install and uninstall targets require write access to /usr/lib, /usr/include and /usr/bin.

## How to use

Once installed, the binary /usr/bin/louischessx is created, which can communicate with XBoard through stdin and stdout.

To connect to xboard, either:

- While xboard is open, go to _Engine_ > _Load new 1st engine_, then set the _Engine Directory_ to blank, the _Engine Command_ to 'louischessx', and click _OK_.
- Alternatively start XBoard with the command `$ xboard -firstChessProgram louischessx`, which will automatically do the above on start.

In both of these cases, the engine is automatically set to play black. To modify which color the engine plays (or even face it off against a second engine) look in the _Mode_ tab.

See https://www.gnu.org/software/xboard/user_guide/UserGuide.html for help on the other aspects of the XBoard interface.