#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <climits>
#include <set>
#include <chrono>
#include <iomanip>

using namespace std;

const int SIZE = 20;
const int MAX_DEPTH = 3;
const int NEIGHBOR_RADIUS = 2;

const int EMPTY = 0;
const int OPPONENT = 1;
const int BOT = 2;

int board[SIZE * SIZE] = {0};
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

inline int get_idx(int x, int y) {
    return y * SIZE + x;
}

inline bool is_valid(int x, int y) {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

string move_to_str(int move) {
    if (move == -1) return "NULL";
    int x = move % SIZE;
    int y = move / SIZE;
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

int quick_eval(int bot_val, int opp_val) {
    int score = 0;
    for (int i = 0; i < SIZE * SIZE; ++i) {
        if (board[i] == bot_val) score += 3;
        else if (board[i] == opp_val) score -= 4;
    }
    return score;
}

int pattern_evaluate(int bot_val, int opp_val) {
    vector<string> lines;
    lines.reserve(SIZE * 2 + SIZE * 4);

    for (int y = 0; y < SIZE; ++y) {
        vector<int> row;
        for (int x = 0; x < SIZE; ++x) row.push_back(board[get_idx(x, y)]);
        lines.push_back(line_to_string(row, bot_val, opp_val));
    }

    for (int x = 0; x < SIZE; ++x) {
        vector<int> col;
        for (int y = 0; y < SIZE; ++y) col.push_back(board[get_idx(x, y)]);
        lines.push_back(line_to_string(col, bot_val, opp_val));
    }

    for (int d = -(SIZE - 1); d < SIZE; ++d) {
        vector<int> diag;
        for (int y = 0; y < SIZE; ++y) {
            int x = y - d;
            if (is_valid(x, y)) diag.push_back(board[get_idx(x, y)]);
        }
        if (diag.size() >= 5) lines.push_back(line_to_string(diag, bot_val, opp_val));
    }

    for (int d = 0; d < SIZE * 2; ++d) {
        vector<int> diag;
        for (int y = 0; y < SIZE; ++y) {
            int x = d - y;
            if (is_valid(x, y)) diag.push_back(board[get_idx(x, y)]);
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

            total_score -= count_occurrences(s, p_opp) * p.score * 1.3;
        }
    }

    return (int)total_score;
}

vector<int> get_neighboring_cells() {
    set<int> candidates;
    vector<int> occupied;
    
    for(int i=0; i<SIZE*SIZE; ++i) {
        if(board[i] != EMPTY) occupied.push_back(i);
    }

    if (occupied.empty()) return {};

    for (int idx : occupied) {
        int ox = idx % SIZE;
        int oy = idx / SIZE;
        
        for (int dy = -NEIGHBOR_RADIUS; dy <= NEIGHBOR_RADIUS; ++dy) {
            for (int dx = -NEIGHBOR_RADIUS; dx <= NEIGHBOR_RADIUS; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = ox + dx;
                int ny = oy + dy;
                if (is_valid(nx, ny)) {
                    int n_idx = get_idx(nx, ny);
                    if (board[n_idx] == EMPTY) {
                        candidates.insert(n_idx);
                    }
                }
            }
        }
    }
    return vector<int>(candidates.begin(), candidates.end());
}

vector<int> move_ordering(const vector<int>& candidates, int player) {
    int opp = (player == BOT) ? OPPONENT : BOT;
    vector<pair<int, int>> scored_moves;
    
    for (int idx : candidates) {
        board[idx] = player;
        int s = quick_eval(player, opp);
        board[idx] = EMPTY;
        scored_moves.push_back({s, idx});
    }

    sort(scored_moves.begin(), scored_moves.end(), [](const pair<int,int>& a, const pair<int,int>& b) {
        return a.first > b.first;
    });

    vector<int> result;
    for (auto& p : scored_moves) result.push_back(p.second);
    return result;
}

int minimax(int depth, int alpha, int beta, bool maximizing, int bot_val, int opp_val) {
    nodesCount++;
    int score = pattern_evaluate(bot_val, opp_val);

    if (abs(score) >= 10000 || depth == 0) {
        return score;
    }

    vector<int> candidates = get_neighboring_cells();
    if (candidates.empty()) return score;

    if (maximizing) {
        vector<int> moves = move_ordering(candidates, bot_val);
        int max_eval = -INT_MAX;
        for (int idx : moves) {
            board[idx] = bot_val;
            int eval = minimax(depth - 1, alpha, beta, false, bot_val, opp_val);
            board[idx] = EMPTY;
            
            max_eval = max(max_eval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        vector<int> moves = move_ordering(candidates, opp_val);
        int min_eval = INT_MAX;
        for (int idx : moves) {
            board[idx] = opp_val;
            int eval = minimax(depth - 1, alpha, beta, true, bot_val, opp_val);
            board[idx] = EMPTY;
            
            min_eval = min(min_eval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

int find_best_move() {
    auto startTime = chrono::steady_clock::now();
    nodesCount = 0;

    vector<int> candidates = get_neighboring_cells();
    
    if (candidates.empty()) {
        int center = SIZE / 2;
        if (board[get_idx(center, center)] == EMPTY) return get_idx(center, center);
        return -1; 
    }

    vector<int> ordered_moves = move_ordering(candidates, BOT);
    int best_move = ordered_moves[0];
    int best_score = -INT_MAX;
    int alpha = -INT_MAX;
    int beta = INT_MAX;

    for (int idx : ordered_moves) {
        board[idx] = BOT;
        int score = minimax(MAX_DEPTH - 1, alpha, beta, false, BOT, OPPONENT);
        board[idx] = EMPTY;

        if (score > best_score) {
            best_score = score;
            best_move = idx;
        }
        
        alpha = max(alpha, score);
        if (beta <= alpha) break;
    }

    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(now - startTime).count();
    
    int d = MAX_DEPTH;
    int bestVal = best_score;
    int bestMove = best_move;
    bool timeOut = false;
    int curMove = bestMove;

    if (!timeOut && curMove != -1) {
        bestMove = curMove;
        cerr << "depth:" << d
            << ",  eval:" << bestVal
            << ",  nodes:" << nodesCount
            << ",  time:" << elapsed << "ms"
            << ",  best:" << move_to_str(bestMove)
            << endl;
    } else {
        cerr << "depth:" << d
            << ",  eval:" << bestVal
            << ",  nodes:" << nodesCount
            << ",  time:" << elapsed << "ms"
            << ",  best:" << move_to_str(bestMove)
            << "  [TIMEOUT]"
            << endl;
    }

    cerr << "bestmove " << move_to_str(bestMove) << endl;

    return best_move;
}

int main() {
    setbuf(stderr, NULL);
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int op_move;
    while (cin >> op_move) {
        if (op_move != -1) {
            board[op_move] = OPPONENT;
        }

        int my_move = find_best_move();
        
        if (my_move != -1) {
            board[my_move] = BOT;
        }
        
        cout << my_move << endl;
    }
    return 0;
}