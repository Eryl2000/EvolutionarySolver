#ifndef TTT_H
#define TTT_H

#include <Eigen/Dense>
using namespace Eigen;

#include "NeuralNet.h"
#include "Player.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::vector;
using std::pair;
using std::make_pair;

enum States { empty = 0, playerX = 1, playerO = 2, invalid = 3 };



template <class T1, class T2>
class TicTacToe {
public:
    TicTacToe(playerContainer<T1>& player1, playerContainer<T2>& player2,
              bool verbose = false);
    void playGame();

    static const int NUM_INPUTS = 9;
    static const int NUM_OUTPUTS = 9;
private:
    bool takeTurn(const States state, const int turns);
    bool isEmpty() const;
    bool isFull() const;

    RowVectorXd toMatrix() const;
    RowVectorXd toPlayerPerspective(const States state) const;

    States getBoardAtPosition(const int position) const;
    void setBoardAtPosition(const int position, const States state);

    inline RowVectorXi bestMoves(const RowVectorXd& input) const;
    void printBoard() const;
    bool hasWon() const;

    uint32_t m_board;

    playerContainer<T1>& m_player1;
    playerContainer<T2>& m_player2;

    bool m_verbose;


};




template <class T1, class T2>
TicTacToe<T1, T2>::TicTacToe(playerContainer<T1>& player1,
                             playerContainer<T2>& player2, bool verbose)
    : m_player1(player1)
    , m_player2(player2)
    , m_verbose(verbose) {
    m_board = (uint32_t)0;
}

/* Plays until a player wins or the board is full.
 * Player's fitness is updated within takeTurn().
 */
template <class T1, class T2>
void TicTacToe<T1, T2>::playGame() {
    int turns = 0;
    while (true) {
        turns++;
        if (takeTurn(States::playerX, turns)) {
            break;
        }
        turns++;
        if (takeTurn(States::playerO, turns)) {
            break;
        }
    }
}

//Helper method to determine if the board is empty
template <class T1, class T2>
bool TicTacToe<T1, T2>::isEmpty() const {
    return m_board == (uint32_t)0;
}

//Helper method to determine if the board is full
template <class T1, class T2>
bool TicTacToe<T1, T2>::isFull() const {
    for (int i = 0; i < 9; ++i) {
        if (getBoardAtPosition(i) == States::empty) {
            return false;
        }
    }
    return true;
}

//Returns a vector of the preferred moves starting with most preferred
template <class T1, class T2>
inline RowVectorXi TicTacToe<T1, T2>::bestMoves(const RowVectorXd& input) const {
    Matrix<int, 1, 9> ret;
    vector< pair<double, unsigned int> > inputPair;
    inputPair.reserve(9);

    //Populate inputPair
    for (unsigned int i = 0; i < NUM_OUTPUTS; ++i) {
        inputPair.emplace_back(make_pair(input(i), i));
    }

    sort(inputPair.begin(), inputPair.end());

    //Populate ret
    for (unsigned int i = 0; i < NUM_OUTPUTS; ++i) {
        ret(8 - i) = inputPair[i].second;
    }

    return ret;
}

//Prints the current board to the console
template <class T1, class T2>
void TicTacToe<T1, T2>::printBoard() const {
    cout << "+---+---+---+" << endl;
    for (int i = 0; i < 3; ++i) {
        cout << "|";
        for (int j = 0; j < 3; ++j) {
            States cur = getBoardAtPosition(3 * i + j);
            if (cur == States::playerX) {
                cout << " X |";
            } else if (cur == States::playerO) {
                cout << " O |";
            } else {
                cout << "   |";
            }
        }
        cout << endl << "+---+---+---+" << endl;
    }
    cout << endl;
}

/* The board is stored as a 3x3 matrix.
 * This helper method converts that into a 1D column matrix
 * representation so it can be passed into other methods.
 */
template <class T1, class T2>
RowVectorXd TicTacToe<T1, T2>::toMatrix() const {
    RowVectorXd temp(NUM_OUTPUTS);

    for (int i = 0; i < 9; ++i) {
        temp(i) = (double)getBoardAtPosition(i);
    }
    return temp;
}


/* Copy of the board with:
    - player's own squares =  1,
    - opponent's squares   = -1,
    - empty squares        =  0
 */
template <class T1, class T2>
RowVectorXd TicTacToe<T1, T2>::toPlayerPerspective(const States state) const {
    RowVectorXd temp = toMatrix();

    for (unsigned int i = 0; i < 9; ++i) {
        States cur = static_cast<States>((int)temp(i));
        if (cur == States::empty) {
            temp(i) = 0.0;
        } else if (cur == state) {
            temp(i) = 1.0;
        } else {
            temp(i) = -1.0;
        }
    }
    return temp;
}

/* 'position' is an integer between [0,8], but the board is a 3x3 matrix
 * This method returns the board position at that position.
 */
template <class T1, class T2>
States TicTacToe<T1, T2>::getBoardAtPosition(const int position) const {
    if (position < 0 || position > 8) {
        cerr << "Invalid position in getBoardAtPosition" << endl;
        exit(1);
    }
    uint32_t shiftAmount = (uint32_t)(8 - position) << 1;
    uint32_t ret = m_board >> shiftAmount;
    ret &= (uint32_t)3;
    if (ret == (uint32_t)0) {
        return States::empty;
    } else if (ret == (uint32_t)1) {
        return States::playerX;
    } else if (ret == (uint32_t)2) {
        return States::playerO;
    } else {
        return States::invalid;
    }
}

/* This method sets the board square given by 'position' to a given value.
 */
template <class T1, class T2>
void TicTacToe<T1, T2>::setBoardAtPosition(const int position,
                                           const States state) {
    uint32_t shiftAmount = (uint32_t)(8 - position) << 1;

    //Clear the 2-bit-wode field
    m_board &= ~((uint32_t)3 << shiftAmount);

    //Set the new state at the 2-bit-wide field
    uint32_t val = (uint32_t)state;
    m_board |= (val << shiftAmount);
}

//Helper function to determine if a player has won
template <class T1, class T2>
bool TicTacToe<T1, T2>::hasWon() const {
    //PlayerX
    if ((m_board & (uint32_t)65793) == (uint32_t)65793) { return true; }
    if ((m_board & (uint32_t)4368) == (uint32_t)4368) { return true; }
    if ((m_board & (uint32_t)86016) == (uint32_t)86016) { return true; }
    if ((m_board & (uint32_t)1344) == (uint32_t)1344) { return true; }
    if ((m_board & (uint32_t)21) == (uint32_t)21) { return true; }
    if ((m_board & (uint32_t)66576) == (uint32_t)66576) { return true; }
    if ((m_board & (uint32_t)16644) == (uint32_t)16644) { return true; }
    if ((m_board & (uint32_t)4161) == (uint32_t)4161) { return true; }

    //PlayerO
    if ((m_board & (uint32_t)131586) == (uint32_t)131586) { return true; }
    if ((m_board & (uint32_t)8736) == (uint32_t)8736) { return true; }
    if ((m_board & (uint32_t)172032) == (uint32_t)172032) { return true; }
    if ((m_board & (uint32_t)2688) == (uint32_t)2688) { return true; }
    if ((m_board & (uint32_t)42) == (uint32_t)42) { return true; }
    if ((m_board & (uint32_t)133152) == (uint32_t)133152) { return true; }
    if ((m_board & (uint32_t)33288) == (uint32_t)33288) { return true; }
    if ((m_board & (uint32_t)8322) == (uint32_t)8322) { return true; }

    return false;
}

//helper function to handle the steps required to take a turn
template <class T1, class T2>
bool TicTacToe<T1, T2>::takeTurn(const States state, const int turn) {
    //holds the list of desired moves in order of preference
    RowVectorXi moves;

    //Diagnostics
    if (m_verbose) {
        printBoard();
    }

    //player 1 controls 'X' squares, player 2 controls 'O' squares
    RowVectorXd playerPerspective = toPlayerPerspective(state);
    if (state == States::playerX) {
        moves = bestMoves(m_player1.player.getMove(playerPerspective));
    } else {
        moves = bestMoves(m_player2.player.getMove(playerPerspective));
    }

    //Make the best move from available squares
    for (int i = 0; i < NUM_OUTPUTS; ++i) {
        if (getBoardAtPosition(moves(i)) == States::empty) {
            setBoardAtPosition(moves(i), state);
            break;
        }
    }

    //Check if the move played was a winning move
    if (turn >= 5) {
        if (hasWon()) {
            if (state == States::playerX) {
                m_player1.player.addToFitness(1.0 + (10.0 - turn) / 10.0);
            } else {
                m_player2.player.addToFitness(1.0 + (10.0 - turn) / 10.0);
            }

            if (m_verbose) {
                printBoard();

                char symbol;
                if (state == States::playerX) {
                    symbol = 'X';
                } else {
                    symbol = 'O';
                }

                cout << "Player " << symbol << " has won the game!" << endl;
                cout << "=============" << endl;
            }
            return true;
        }
    }


    //Check if the board is now full
    if (turn == 9) {
        if (isFull()) {
            m_player1.player.addToFitness(1.0);
            m_player2.player.addToFitness(1.0);
            if (m_verbose) {
                printBoard();
                cout << "Tie game" << endl;
                cout << "=============" << endl;
            }
            return true;
        }
    }


    //If the game is not over, return false
    return false;
}

#endif
