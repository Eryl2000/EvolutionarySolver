# TicTacToeMachineLearning
Created a genetic algorithm which uses a population of neural networks to learn to play games. Written from scratch in C++. Currently Tic-tac-toe and Ultimate tic-tac-toe are implemented as games. This has been a long-term passion project of mine which I began after first learning about genetic algorithms.

While tic-tac-toe is fully solvable using algorithms such as minimax, a player’s move in Ultimate tic-tac-toe affects the available moves an opponent has, meaning the game cannot easily be solved using DFS. This is partly because it is hard to find a good heuristic for the strength of a board position. My approach therefore does not attempt to fully solve the game. Instead it learns to find acceptable moves through a random exploration of the game space.
