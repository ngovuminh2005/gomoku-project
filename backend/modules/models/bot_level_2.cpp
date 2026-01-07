#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cmath>
#include <random>
#include <iomanip>

#include "../logic/ZobristTable.cpp"

using namespace std;

const int TIME_LIMIT_MS = 2000;
const int MAX_SEARCH_DEPTH = 20;
const int MOVE_GEN_RADIUS = 2;
const long long INF_SCORE = 1e16;

double DEFENSE_SCALE = 1.3;
int NOISE_MAGNITUDE = 0;
long long nodesCount = 0;

struct Pattern {
    string s;
    int score;
};

const vector<Pattern> PATTERNS = {
    {"XXXXX", 100000},
    {"_XXXX_", 10000},
    {"XXXX_", 5000},
    {"_XXXX", 5000},
    {"_XXX_", 1000},
    {"XXX__", 200},
    {"__XXX", 200},
    {"XX_XX", 800},
    {"_XX_", 50}
};

int myID = 2;
int opID = 1;

long long history[BOARD_SIZE * BOARD_SIZE];
long long killerMoves[MAX_SEARCH_DEPTH][2];

chrono::steady_clock::time_point startTime;
bool timeOut;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

string move_to_str(int move) {
    if (move == -1) return "NULL";
    int x = getX(move);
    int y = getY(move);
    char col = 'A' + x;
    return string(1, col) + to_string(y + 1);
}

int count_occurrences(const string& str, const string& sub) {
    int count = 0;
    size_t pos = 0;
    while ((pos = str.find(sub, pos)) != string::npos) {
        count++;
        pos += sub.length();
    }
    return count;
}

string line_to_string(const vector<int>& line, int my_val, int op_val) {
    string s = "";
    s.reserve(line.size());
    for (int val : line) {
        if (val == my_val) s += 'X';
        else if (val == op_val) s += 'O';
        else s += '_';
    }
    return s;
}

long long pattern_evaluate(int bot_val) {
    int opp_val = (bot_val == 1) ? 2 : 1;
    vector<string> lines;
    lines.reserve(BOARD_SIZE * 2 + BOARD_SIZE * 4);

    for (int y = 0; y < BOARD_SIZE; ++y) {
        vector<int> row;
        for (int x = 0; x < BOARD_SIZE; ++x) row.push_back(board[getIdx(x, y)]);
        lines.push_back(line_to_string(row, bot_val, opp_val));
    }

    for (int x = 0; x < BOARD_SIZE; ++x) {
        vector<int> col;
        for (int y = 0; y < BOARD_SIZE; ++y) col.push_back(board[getIdx(x, y)]);
        lines.push_back(line_to_string(col, bot_val, opp_val));
    }

    for (int d = -(BOARD_SIZE - 1); d < BOARD_SIZE; ++d) {
        vector<int> diag;
        for (int y = 0; y < BOARD_SIZE; ++y) {
            int x = y - d;
            if (isValid(x, y)) diag.push_back(board[getIdx(x, y)]);
        }
        if (diag.size() >= 5) lines.push_back(line_to_string(diag, bot_val, opp_val));
    }

    for (int d = 0; d < BOARD_SIZE * 2; ++d) {
        vector<int> diag;
        for (int y = 0; y < BOARD_SIZE; ++y) {
            int x = d - y;
            if (isValid(x, y)) diag.push_back(board[getIdx(x, y)]);
        }
        if (diag.size() >= 5) lines.push_back(line_to_string(diag, bot_val, opp_val));
    }

    long long total_score = 0;

    for (const string& s : lines) {
        for (const auto& p : PATTERNS) {
            total_score += count_occurrences(s, p.s) * p.score;
            string p_opp = p.s;
            replace(p_opp.begin(), p_opp.end(), 'X', 'K');
            replace(p_opp.begin(), p_opp.end(), 'O', 'X');
            replace(p_opp.begin(), p_opp.end(), 'K', 'O');
            total_score -= count_occurrences(s, p_opp) * p.score * DEFENSE_SCALE;
        }
    }

    if (NOISE_MAGNITUDE > 0) {
        total_score += (long long)(rng() % (NOISE_MAGNITUDE * 2 + 1)) - NOISE_MAGNITUDE;
    }

    return total_score;
}

vector<int> generateMoves() {
    vector<int> moves;
    int minX = BOARD_SIZE, maxX = 0, minY = BOARD_SIZE, maxY = 0;
    bool empty = true;
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        if (board[i] != 0) {
            empty = false;
            int x = getX(i), y = getY(i);
            minX = min(minX, x); maxX = max(maxX, x);
            minY = min(minY, y); maxY = max(maxY, y);
        }
    }
    if (empty) {
        moves.push_back(getIdx(BOARD_SIZE/2, BOARD_SIZE/2));
        return moves;
    }
    minX = max(0, minX - MOVE_GEN_RADIUS);
    maxX = min(BOARD_SIZE - 1, maxX + MOVE_GEN_RADIUS);
    minY = max(0, minY - MOVE_GEN_RADIUS);
    maxY = min(BOARD_SIZE - 1, maxY + MOVE_GEN_RADIUS);

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            int idx = getIdx(x, y);
            if (board[idx] == 0) {
                 bool hasNeighbor = false;
                 for (int dy = -1; dy <= 1; dy++) {
                     for (int dx = -1; dx <= 1; dx++) {
                         if (dx == 0 && dy == 0) continue;
                         int nx = x + dx, ny = y + dy;
                         if (isValid(nx, ny) && board[getIdx(nx, ny)] != 0) {
                             hasNeighbor = true;
                             break;
                         }
                     }
                     if (hasNeighbor) break;
                 }
                 if (hasNeighbor) moves.push_back(idx);
            }
        }
    }
    return moves;
}

long long alphaBeta(int depth, long long alpha, long long beta, int p) {
    nodesCount++;
    if ((depth % 4 == 0) && chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() > TIME_LIMIT_MS) {
        timeOut = true;
        return 0;
    }
    int idx = currentHash & (TT_TABLE_SIZE - 1);
    if (TTable[idx].key == currentHash && TTable[idx].depth >= depth) {
        if (TTable[idx].flag == FLAG_EXACT) return TTable[idx].score;
        if (TTable[idx].flag == FLAG_LOWERBOUND && TTable[idx].score >= beta) return beta;
        if (TTable[idx].flag == FLAG_UPPERBOUND && TTable[idx].score <= alpha) return alpha;
    }
    if (depth == 0) return pattern_evaluate(p);

    vector<int> moves = generateMoves();
    if (moves.empty()) return 0;

    int bestMoveCache = -1;
    if (TTable[idx].key == currentHash) bestMoveCache = TTable[idx].bestMove;

    for (int& m : moves) {
        long long score = 0;
        if (m == bestMoveCache) score = 1e18;
        else score = history[m];
        if (m == killerMoves[depth][0]) score += 10000;
        else if (m == killerMoves[depth][1]) score += 5000;
        history[m] = score;
    }
    sort(moves.begin(), moves.end(), [](int a, int b) { return history[a] > history[b]; });

    long long bestVal = -INF_SCORE * 2;
    int flag = FLAG_UPPERBOUND;
    int moveIdx = -1;
    int movesSearched = 0;

    for (int m : moves) {
        board[m] = p;
        toggleHash(m, p);
        long long val;
        if (movesSearched == 0) val = -alphaBeta(depth - 1, -beta, -alpha, (p == 1) ? 2 : 1);
        else {
            val = -alphaBeta(depth - 1, -alpha - 1, -alpha, (p == 1) ? 2 : 1);
            if (val > alpha && val < beta) val = -alphaBeta(depth - 1, -beta, -alpha, (p == 1) ? 2 : 1);
        }
        toggleHash(m, p);
        board[m] = 0;
        if (timeOut) return 0;
        movesSearched++;
        if (val > bestVal) {
            bestVal = val;
            moveIdx = m;
        }
        if (bestVal > alpha) {
            alpha = bestVal;
            flag = FLAG_EXACT;
            history[m] += depth * depth;
            if (history[m] > 1e15) history[m] /= 2;
            if (m != killerMoves[depth][0]) {
                killerMoves[depth][1] = killerMoves[depth][0];
                killerMoves[depth][0] = m;
            }
        }
        if (alpha >= beta) {
            flag = FLAG_LOWERBOUND;
            break;
        }
    }
    if (!timeOut) TTable[idx] = {currentHash, depth, bestVal, flag, moveIdx};
    return bestVal;
}

int solve() {
    startTime = chrono::steady_clock::now();
    timeOut = false;
    nodesCount = 0;
    memset(history, 0, sizeof(history));
    memset(killerMoves, 0, sizeof(killerMoves));
    vector<int> moves = generateMoves();
    if (moves.size() == 1) return moves[0];
    int bestMove = moves[0];
    
    for (int d = 1; d <= MAX_SEARCH_DEPTH; d++) {
        long long bestVal = -INF_SCORE * 2;
        int curMove = -1;
        long long alpha = -INF_SCORE * 2;
        long long beta = INF_SCORE * 2;
        int idx = currentHash & (TT_TABLE_SIZE - 1);
        if (TTable[idx].key == currentHash && TTable[idx].bestMove != -1) bestMove = TTable[idx].bestMove;
        
        sort(moves.begin(), moves.end(), [&](int a, int b) {
            if (a == bestMove) return true;
            if (b == bestMove) return false;
            return history[a] > history[b];
        });

        for (int m : moves) {
            board[m] = myID;
            toggleHash(m, myID);
            long long val = -alphaBeta(d - 1, -beta, -alpha, opID);
            toggleHash(m, myID);
            board[m] = 0;
            if (timeOut) break;
            if (val > bestVal) {
                bestVal = val;
                curMove = m;
            }
            alpha = max(alpha, bestVal);
        }

        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - startTime).count();


        if (!timeOut && curMove != -1) {
            bestMove = curMove;

            cerr << "depth:" << d
                << ",  eval:" << bestVal
                << ",  nodes:" << nodesCount
                << ",  time:" << elapsed << "ms"
                << ",  best:" << move_to_str(bestMove)
                << endl;

            if (bestVal >= 90000) break;
        } else {
            cerr << "depth:" << d
                << ",  eval:" << bestVal
                << ",  nodes:" << nodesCount
                << ",  time:" << elapsed << "ms"
                << ",  best:" << move_to_str(bestMove)
                << "  [TIMEOUT]"
                << endl;
            break;
        }



            }
            cerr << "bestmove " << move_to_str(bestMove) << endl;
            return bestMove;
        }

#ifndef LIB_MODE
int main() {
    setbuf(stderr, NULL); 
    cerr << "system: Bot Level 2 Initialized..." << endl;
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
#endif