#ifndef ZOBRIST_TABLE_H
#define ZOBRIST_TABLE_H

#include <cstdint>
#include <random>
#include <cstring>

const int BOARD_SIZE = 20;
const int TT_TABLE_SIZE = 1 << 22;
const int RNG_SEED = 12345;

const int FLAG_EXACT = 0;
const int FLAG_LOWERBOUND = 1;
const int FLAG_UPPERBOUND = 2;

struct TTEntry {
    uint64_t key;
    int depth;
    long long score;
    int flag;
    int bestMove;
};

extern TTEntry TTable[TT_TABLE_SIZE];
extern uint64_t zobrist[BOARD_SIZE * BOARD_SIZE][2];
extern uint64_t zobristTurn;
extern uint64_t currentHash;
extern int board[BOARD_SIZE * BOARD_SIZE];

void initZobrist();
void toggleHash(int idx, int player);

inline int getIdx(int x, int y) { return y * BOARD_SIZE + x; }
inline int getX(int idx) { return idx % BOARD_SIZE; }
inline int getY(int idx) { return idx / BOARD_SIZE; }
inline bool isValid(int x, int y) { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }

#endif