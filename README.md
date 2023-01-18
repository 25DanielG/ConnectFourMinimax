# ConnectFourMinimax
This program uses
- Minimax
- Iterative Deepening
- Quiescence Search
- Alpha Beta Pruning
- pthread.h multithreading
- Transposition table helped by boost's serialization
- Questionable SDL graphics

Heuristic Algorithm:
+2.5 each center piece
+5 each connected two that is not contained
+20 each connected three that is not contained
+1000 possible win next move (stacks with double traps) (opponent win)
+10000 possible win next move for computer

Running The Program:
- Have Boost and SDL2 installed using Homebrew
- Run the exec.sh script