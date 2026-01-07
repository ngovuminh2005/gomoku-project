#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cmath>
#include <random>
#include <iomanip>
#include "../logic/ZobristTable.cpp"

using namespace std;

int NOISE_MAGNITUDE = 0;
double DEFENSE_SCALE = 1.1;

const int TIME_LIMIT_MS = 1000;
const int VCT_TIME_LIMIT_MS = 200;
const int MAX_SEARCH_DEPTH = 20;
const int VCT_DEPTH = 12;
const int MOVE_GEN_RADIUS = 2;

const long long INF_SCORE = 1e16;
const long long SCORE_WIN = 1e14;
const long long SCORE_LIVE_4 = 1e11;
const long long SCORE_DEAD_4 = 1e7;
const long long SCORE_LIVE_3 = 1e7;
const long long SCORE_DEAD_3 = 2e6;
const long long SCORE_LIVE_2 = 2e6;
const long long SCORE_DEAD_2 = 1e4;

const int TYPE_NONE = 0;
const int TYPE_DEAD_3 = 1;
const int TYPE_OPEN_3 = 2;
const int TYPE_CLOSED_4 = 3;
const int TYPE_OPEN_4 = 4;
const int TYPE_WIN = 5;

int myID = 2;
int opID = 1;

long long history[BOARD_SIZE * BOARD_SIZE];
long long killerMoves[MAX_SEARCH_DEPTH][2];
long long nodesCount = 0;

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

int getMoveStatus(int idx, int p) {
    int cx = getX(idx);
    int cy = getY(idx);
    int maxStatus = TYPE_NONE;
    int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (int d = 0; d < 4; d++) {
        int dx = dirs[d][0];
        int dy = dirs[d][1];

        int count = 1;

        int x = cx + dx, y = cy + dy;
        while (isValid(x, y) && board[getIdx(x, y)] == p) {
            count++;
            x += dx; y += dy;
        }
        bool openPos = isValid(x, y) && board[getIdx(x, y)] == 0;

        x = cx - dx; y = cy - dy;
        while (isValid(x, y) && board[getIdx(x, y)] == p) {
            count++;
            x -= dx; y -= dy;
        }
        bool openNeg = isValid(x, y) && board[getIdx(x, y)] == 0;

        int openEnds = (openPos ? 1 : 0) + (openNeg ? 1 : 0);

        if (count >= 5) return TYPE_WIN;

        if (count == 4) {
            if (openEnds == 2) return TYPE_OPEN_4;
            if (openEnds == 1) maxStatus = max(maxStatus, TYPE_CLOSED_4);
        }

        if (count == 3) {
            if (openEnds == 2) maxStatus = max(maxStatus, TYPE_OPEN_3);
            else if (openEnds == 1) maxStatus = max(maxStatus, TYPE_DEAD_3);
        }
    }
    return maxStatus;
}

long long evaluateLine(const vector<int>& line, int p) {
    long long score = 0;
    int n = (int)line.size();

    for (int i = 0; i < n; ) {
        if (line[i] != p) { i++; continue; }

        int start = i;
        while (i < n && line[i] == p) i++;
        int count = i - start;

        int openEnds = 0;
        if (start - 1 >= 0 && line[start - 1] == 0) openEnds++;
        if (i < n && line[i] == 0) openEnds++;

        if (count >= 5) return INF_SCORE;
        if (count == 4) {
            if (openEnds == 2) score += SCORE_LIVE_4;
            else if (openEnds == 1) score += SCORE_DEAD_4;
        } else if (count == 3) {
            if (openEnds == 2) score += SCORE_LIVE_3;
            else if (openEnds == 1) score += SCORE_DEAD_3;
        } else if (count == 2) {
            if (openEnds == 2) score += SCORE_LIVE_2;
            else if (openEnds == 1) score += SCORE_DEAD_2;
        }
    }

    return score;
}

long long evaluateBoard(int p) {
    long long totalScore = 0;
    int op = (p == 1) ? 2 : 1;

    for (int y = 0; y < BOARD_SIZE; y++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int x = 0; x < BOARD_SIZE; x++) line.push_back(board[getIdx(x, y)]);
        long long sp = evaluateLine(line, p);
        if (sp >= SCORE_WIN) return INF_SCORE;
        long long so = evaluateLine(line, op);
        if (so >= SCORE_WIN) return -INF_SCORE;
        totalScore += sp;
        totalScore -= so * DEFENSE_SCALE;
    }

    for (int x = 0; x < BOARD_SIZE; x++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int y = 0; y < BOARD_SIZE; y++) line.push_back(board[getIdx(x, y)]);
        long long sp = evaluateLine(line, p);
        if (sp >= SCORE_WIN) return INF_SCORE;
        long long so = evaluateLine(line, op);
        if (so >= SCORE_WIN) return -INF_SCORE;
        totalScore += sp;
        totalScore -= so * DEFENSE_SCALE;
    }

    for (int k = 0; k < BOARD_SIZE; k++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int x = k, y = 0; x < BOARD_SIZE && y < BOARD_SIZE; x++, y++)
            line.push_back(board[getIdx(x, y)]);
        if (line.size() >= 5) {
            long long sp = evaluateLine(line, p);
            if (sp >= SCORE_WIN) return INF_SCORE;
            long long so = evaluateLine(line, op);
            if (so >= SCORE_WIN) return -INF_SCORE;
            totalScore += sp;
            totalScore -= so * DEFENSE_SCALE;
        }
    }
    for (int k = 1; k < BOARD_SIZE; k++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int x = 0, y = k; x < BOARD_SIZE && y < BOARD_SIZE; x++, y++)
            line.push_back(board[getIdx(x, y)]);
        if (line.size() >= 5) {
            long long sp = evaluateLine(line, p);
            if (sp >= SCORE_WIN) return INF_SCORE;
            long long so = evaluateLine(line, op);
            if (so >= SCORE_WIN) return -INF_SCORE;
            totalScore += sp;
            totalScore -= so * DEFENSE_SCALE;
        }
    }

    for (int k = 0; k < BOARD_SIZE; k++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int x = k, y = 0; x >= 0 && y < BOARD_SIZE; x--, y++)
            line.push_back(board[getIdx(x, y)]);
        if (line.size() >= 5) {
            long long sp = evaluateLine(line, p);
            if (sp >= SCORE_WIN) return INF_SCORE;
            long long so = evaluateLine(line, op);
            if (so >= SCORE_WIN) return -INF_SCORE;
            totalScore += sp;
            totalScore -= so * DEFENSE_SCALE;
        }
    }
    for (int k = 1; k < BOARD_SIZE; k++) {
        vector<int> line;
        line.reserve(BOARD_SIZE);
        for (int x = BOARD_SIZE - 1, y = k; x >= 0 && y < BOARD_SIZE; x--, y++)
            line.push_back(board[getIdx(x, y)]);
        if (line.size() >= 5) {
            long long sp = evaluateLine(line, p);
            if (sp >= SCORE_WIN) return INF_SCORE;
            long long so = evaluateLine(line, op);
            if (so >= SCORE_WIN) return -INF_SCORE;
            totalScore += sp;
            totalScore -= so * DEFENSE_SCALE;
        }
    }

    if (NOISE_MAGNITUDE > 0 && abs(totalScore) < SCORE_DEAD_3) {
        totalScore += (long long)(rng() % (NOISE_MAGNITUDE * 2 + 1)) - NOISE_MAGNITUDE;
    }

    return totalScore;
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
        moves.push_back(getIdx(BOARD_SIZE / 2, BOARD_SIZE / 2));
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

bool opponentHasDangerousThreat(int op) {
    vector<int> moves = generateMoves();
    for (int m : moves) {
        board[m] = op;
        int status = getMoveStatus(m, op);
        board[m] = 0;

        if (status == TYPE_WIN) return true;
        if (status == TYPE_OPEN_4) return true;
        if (status == TYPE_CLOSED_4) return true;
    }
    return false;
}

bool solveVCF(int depth, int p, int& winMove) {
    if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() > VCT_TIME_LIMIT_MS) {
        timeOut = true;
        return false;
    }
    if (depth == 0) return false;

    int op = (p == 1) ? 2 : 1;
    vector<int> moves = generateMoves();
    vector<int> candidates;

    for (int m : moves) {
        board[m] = p;
        int status = getMoveStatus(m, p);
        board[m] = 0;
        if (status == TYPE_WIN) {
            winMove = m;
            return true;
        }
        if (status >= TYPE_CLOSED_4) {
            candidates.push_back(m);
        }
    }

    if (candidates.empty()) return false;

    for (int m : candidates) {
        board[m] = p;

        if (opponentHasDangerousThreat(op)) {
            board[m] = 0;
            continue;
        }

        vector<int> forcedReplies;
        vector<int> nextMoves = generateMoves();
        for (int nm : nextMoves) {
            board[nm] = p;
            if (getMoveStatus(nm, p) == TYPE_WIN) {
                forcedReplies.push_back(nm);
            }
            board[nm] = 0;
        }

        bool defended = false;
        if (forcedReplies.size() > 1) {
            winMove = m;
            board[m] = 0;
            return true;
        } else if (forcedReplies.size() == 1) {
            int reply = forcedReplies[0];
            board[reply] = op;
            int tempMove;
            if (!solveVCF(depth - 1, p, tempMove)) {
                defended = true;
            }
            board[reply] = 0;
        } else {
            defended = true;
        }

        board[m] = 0;
        if (!defended) {
            winMove = m;
            return true;
        }
    }
    return false;
}

bool solveVCT(int depth, int p, int& winMove) {
    if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() > VCT_TIME_LIMIT_MS) {
        timeOut = true;
        return false;
    }

    if (solveVCF(depth, p, winMove)) return true;
    if (depth == 0) return false;

    int op = (p == 1) ? 2 : 1;
    vector<int> moves = generateMoves();
    vector<int> candidates;

    for (int m : moves) {
        board[m] = p;
        int status = getMoveStatus(m, p);
        board[m] = 0;
        if (status == TYPE_OPEN_3 || status == TYPE_DEAD_3) {
            candidates.push_back(m);
        }
    }

    for (int m : candidates) {
        board[m] = p;

        if (opponentHasDangerousThreat(op)) {
            board[m] = 0;
            continue;
        }

        bool moveFailed = false;
        vector<int> defMoves = generateMoves();

        for (int d : defMoves) {
            board[d] = op;

            int opStatus = getMoveStatus(d, op);
            if (opStatus == TYPE_WIN) {
                board[d] = 0;
                moveFailed = true;
                break;
            }

            int tempMove;
            bool pWins = solveVCT(depth - 1, p, tempMove);
            board[d] = 0;

            if (!pWins) {
                moveFailed = true;
                break;
            }
        }

        board[m] = 0;
        if (!moveFailed) {
            winMove = m;
            return true;
        }
    }
    return false;
}

long long alphaBeta(int depth, long long alpha, long long beta, int p) {
    nodesCount++;
    if ((nodesCount & 1023) == 0) {
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() > TIME_LIMIT_MS) {
            timeOut = true;
        }
    }
    if (timeOut) return 0;

    int idx = currentHash & (TT_TABLE_SIZE - 1);
    if (TTable[idx].key == currentHash && TTable[idx].depth >= depth) {
        if (TTable[idx].flag == FLAG_EXACT) return TTable[idx].score;
        if (TTable[idx].flag == FLAG_LOWERBOUND && TTable[idx].score >= beta) return beta;
        if (TTable[idx].flag == FLAG_UPPERBOUND && TTable[idx].score <= alpha) return alpha;
    }
    if (depth == 0) return evaluateBoard(p);

    vector<int> moves = generateMoves();
    if (moves.empty()) return 0;
    int bestMove = -1;
    if (TTable[idx].key == currentHash) bestMove = TTable[idx].bestMove;

    vector<pair<long long, int>> orderedMoves;
    orderedMoves.reserve(moves.size());

    for (int m : moves) {
        long long score = 0;
        if (m == bestMove) score = 1e18;
        else {
            board[m] = p;
            int stat = getMoveStatus(m, p);
            board[m] = 0;
            if (stat >= TYPE_CLOSED_4) score = 1e17;
            else if (stat >= TYPE_OPEN_3) score = 1e16;
            else {
                board[m] = (p == 1 ? 2 : 1);
                int opStat = getMoveStatus(m, (p == 1 ? 2 : 1));
                board[m] = 0;
                if (opStat >= TYPE_CLOSED_4) score = 1e16;
                else score = history[m];
            }
        }
        if (m == killerMoves[depth][0]) score += 10000;
        else if (m == killerMoves[depth][1]) score += 5000;
        orderedMoves.push_back({score, m});
    }
    sort(orderedMoves.begin(), orderedMoves.end(), [](const pair<long long, int>& a, const pair<long long, int>& b) {
        return a.first > b.first;
    });

    long long bestVal = -INF_SCORE * 2;
    int flag = FLAG_UPPERBOUND;
    int moveIdx = -1;
    int movesSearched = 0;

    for (auto& pair : orderedMoves) {
        int m = pair.second;
        board[m] = p;
        if (getMoveStatus(m, p) == TYPE_WIN) {
            board[m] = 0;
            return INF_SCORE;
        }
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
    for (int m : moves) {
        board[m] = myID;
        if (getMoveStatus(m, myID) == TYPE_WIN) { board[m] = 0; return m; }
        board[m] = 0;
    }
    for (int m : moves) {
        board[m] = opID;
        if (getMoveStatus(m, opID) == TYPE_WIN) { board[m] = 0; return m; }
        board[m] = 0;
    }

    int vctMove = -1;
    if (solveVCT(VCT_DEPTH, myID, vctMove)) {
        return vctMove;
    }

    if (timeOut) {
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() < TIME_LIMIT_MS) {
            timeOut = false;
        } else {
            return moves[0];
        }
    }

    if (NOISE_MAGNITUDE > 0) shuffle(moves.begin(), moves.end(), rng);

    int bestMove = moves[0];
    if (moves.size() == 1) return bestMove;

    for (int d = 1; d <= MAX_SEARCH_DEPTH; d++) {
        long long bestVal = -INF_SCORE * 2;
        int curMove = -1;
        long long alpha = -INF_SCORE * 2;
        long long beta = INF_SCORE * 2;
        int idx = currentHash & (TT_TABLE_SIZE - 1);
        if (TTable[idx].key == currentHash && TTable[idx].bestMove != -1) bestMove = TTable[idx].bestMove;

        vector<pair<long long, int>> rootMoves;
        for (int m : moves) {
            long long s = history[m];
            if (m == bestMove) s += 1e18;
            rootMoves.push_back({s, m});
        }
        sort(rootMoves.begin(), rootMoves.end(), [](auto& a, auto& b) { return a.first > b.first; });

        for (auto& pair : rootMoves) {
            int m = pair.second;
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

            if (bestVal >= SCORE_WIN) break;
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
