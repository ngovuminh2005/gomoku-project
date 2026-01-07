#define LIB_MODE
#include "bot_level_3.cpp"

int main() {
    NOISE_MAGNITUDE = 20000; 

    DEFENSE_SCALE = 1.2; 

    initZobrist();
    int move;
    while (cin >> move) {
        if (move != -1) {
            board[move] = opID;
            toggleHash(move, opID);
        } else {
            myID = 1; opID = 2;
        }
        int best = solve();
        board[best] = myID;
        toggleHash(best, myID);
        cout << best << endl;
    }
    return 0;
}