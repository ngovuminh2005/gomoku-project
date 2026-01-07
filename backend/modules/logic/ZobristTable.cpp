#include "ZobristTable.h"

TTEntry TTable[TT_TABLE_SIZE];
uint64_t zobrist[BOARD_SIZE * BOARD_SIZE][2];
uint64_t zobristTurn;
uint64_t currentHash = 0;
int board[BOARD_SIZE * BOARD_SIZE];

void initZobrist() {
    std::mt19937_64 rng(RNG_SEED);
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        zobrist[i][0] = rng();
        zobrist[i][1] = rng();
    }
    zobristTurn = rng();
    std::memset(board, 0, sizeof(board));
    std::memset(TTable, 0, sizeof(TTable));
    currentHash = zobristTurn;
}

void toggleHash(int idx, int player) {
    currentHash ^= zobrist[idx][player - 1];
    currentHash ^= zobristTurn;
}