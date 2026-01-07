#include <iostream>
#include <vector>
using namespace std;
const int SIZE = 20;
const int WIN_LEN = 5;
int board[SIZE * SIZE] = {0};
bool is_valid(int r, int c) { return r >= 0 && r < SIZE && c >= 0 && c < SIZE; }
bool check_win(int idx, int player) {
    int r = idx / SIZE, c = idx % SIZE;
    int dr[] = {1, 0, 1, 1}, dc[] = {0, 1, 1, -1};
    for (int i = 0; i < 4; i++) {
        int count = 1;
        for (int s = 1; s < WIN_LEN; s++) {
            if (!is_valid(r + s*dr[i], c + s*dc[i]) || board[(r + s*dr[i])*SIZE + c + s*dc[i]] != player) break;
            count++;
        }
        for (int s = 1; s < WIN_LEN; s++) {
            if (!is_valid(r - s*dr[i], c - s*dc[i]) || board[(r - s*dr[i])*SIZE + c - s*dc[i]] != player) break;
            count++;
        }
        if (count >= WIN_LEN) return true;
    }
    return false;
}
int main() {
    int idx, player;
    while (cin >> idx >> player) {
        if (idx < 0 || idx >= SIZE * SIZE || board[idx] != 0) { cout << -1 << endl; continue; }
        board[idx] = player;
        cout << (check_win(idx, player) ? 1 : 0) << endl;
    }
    return 0;
}