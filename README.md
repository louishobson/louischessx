# louischessx

A C++ chess engine designed to be an xboard backend.

## Supported OSs and Dependencies

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

## How to Use

Once installed, the binary /usr/bin/louischessx is created, which can communicate with XBoard through stdin and stdout.

To connect to XBoard, either:

- While xboard is open, go to _Engine_ > _Load new 1st engine_, then set the _Engine Directory_ to blank, the _Engine Command_ to 'louischessx', and click _OK_.
- Alternatively, start XBoard with the command `$ xboard -firstChessProgram louischessx`, which will automatically do the above on start.

In both of these cases, the engine is automatically set to play black. To modify which color the engine plays (or even face it off against a second engine) look in the _Mode_ tab.

See https://www.gnu.org/software/xboard/user_guide/UserGuide.html for help on the other aspects of the XBoard interface.

## About the Engine

Chessboard information is stored as bitboards: 64-bit integers serving a bitfield, assigning a boolean value for each square of the chessboard.
For example, a bitboard may be used to mark which squares are occupied, or something more complex such as the paths along which a piece is allowed to travel.
Bitwise operations such as bit shifts or bit rotations allow for the properties in the bitboard to be translated, which forms the basis for calculating piece moves.
Bitboards, although less intuitive to use, have the advantages of having their operations (bit shifts etc.) implemented as single CPU instructions, as well as having the ability to calculate moves of multiple pieces simultaneously (as the whole board is shifted at once).

The search algorithm implemented in _src/louischessx/chessboard_search.cpp_ uses the negamax algorithm to choose a best move for any given state (similar to minimax, but negation to eliminate alternate minimizing and maximizing).
The evaluation function implemented in _src/louischessx/chessboard_eval.cpp_ mostly uses the evaluation points and weights from the Kaissa chess program (see https://www.chessprogramming.org/Kaissa#Evaluation), although the method behind evaluation is my own.
The search tree is optimised in many ways, including:

- Alpha-beta pruning.
- Simple MVV-LVA move ordering.
- Static exchange evaluation.
- Quiescence search with delta pruning.
- Using a transposition table to store previous best values and best moves.
- Iterative deepening with null window pruning. The transposition table is also kept between iterations.
- Killer move heuristic.
- Null move pruning.

All of these techniques allow for the engine to search to an average depth of 9 half-moves (excluding quiescence search) with a max 20 second search time in the opening, and as high as 12 half-moves in the end game. The response time, however, can be significantly reduced since the engine is designed to ponder while it's opponent moves. It runs a shallow search to gain a rough idea of their best choices, then begins to search for its responses before it is even the engine's turn.
If, when it does become the engine's turn, it was able to guess it's opponent's move correctly, the engine will have already begin the search, and the response time will therefore be significantly reduced.
Xboard features such as time controls and board editing are implemented, although live analysis and feedback from the search is not yet available.

## Thanks For Your Support!

Feature requests, bug reports and pull requests are all welcome, as well as any advice or criticism of my programming - I am still very much learning!
Email me on louis-hobson@hotmail.co.uk to get in touch.