#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
const int SIZE = 20;
int board[SIZE * SIZE] = {0};
int find_move() {
    vector<int> empty;
    for(int i=0; i<SIZE*SIZE; ++i) if(board[i] == 0) empty.push_back(i);
    return empty.empty() ? -1 : empty[rand() % empty.size()];
}
int main() {
    srand(time(0));
    int op_move;
    while (cin >> op_move) {
        if (op_move != -1) board[op_move] = 1;
        int my_move = find_move();
        if (my_move != -1) board[my_move] = 2;
        cout << my_move << endl;
    }
    return 0;
}