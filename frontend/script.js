const BOARD_SIZE = 20;
const WIN_LENGTH = 5;
const API_URL = "http://{your_ip_address}:5000"; // Thêm :5000 vào đuôi

const board = document.getElementById("board");
const results = document.querySelector("#results");
const playAgainBtn = document.querySelector("#play-again");
const turnBg = document.querySelector(".bg");
const menu = document.querySelector(".menu");
const gameContainer = document.querySelector(".game-container");
const logWrapper = document.getElementById("log-wrapper");
const logToggle = document.getElementById("log-toggle");
const logContent = document.getElementById("log-content");
const logIcon = document.getElementById("log-icon");
const logTerminal = document.getElementById("log-terminal");

const pvpBtn = document.getElementById("pvp-btn");
const pvcXBtn = document.getElementById("pvc-x-btn");
const pvcOBtn = document.getElementById("pvc-o-btn");

let gameId = null;
let isLocked = false;
let gameMode = "pvp"; 
let turn = "X";
let playerRole = "X";
let boardState = [];
let socket = null;

function createBoard() {
    board.innerHTML = "";
    board.style.gridTemplateColumns = `repeat(${BOARD_SIZE}, 1fr)`;
    board.style.gridTemplateRows = `repeat(${BOARD_SIZE}, 1fr)`;
    boardState = Array(BOARD_SIZE * BOARD_SIZE).fill(null);
    for (let i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        const cell = document.createElement("div");
        cell.classList.add("box");
        cell.dataset.index = i;
        cell.addEventListener("click", handleBoxClick);
        board.appendChild(cell);
    }
}

function updateTurnIndicator(activeTurn) {
    turnBg.style.left = (activeTurn === "X") ? "0" : "85px";
}

function getRealWinner(serverWinner) {
    if (gameMode === "pvp" || playerRole === "X") return serverWinner;
    return serverWinner === "X" ? "O" : "X";
}

async function startGame(mode, role = null) {
    gameMode = mode;
    menu.classList.add("hide");
    gameContainer.classList.remove("hide");
    createBoard();
    resetUI();

    if (gameMode === "pvc") {
        playerRole = role;
        updateTurnIndicator("X"); 
        logWrapper.classList.remove("hide");

        try {
            const res = await fetch(`${API_URL}/start`, { method: "POST" });
            const data = await res.json();
            gameId = data.game_id;
            
            connectSocket(gameId);
            
            if (role === "O") {
                isLocked = true;
                board.classList.add("board-locked");
                await makeAiMove(-1); 
            }
        } catch (e) { console.error(e); }
    } else {
        logWrapper.classList.add("hide");
    }
}

function connectSocket(gid) {
    if (socket) socket.disconnect();
    socket = io(API_URL, {
        transports: ['websocket', 'polling']
    });
    
    socket.on('connect', () => {
        socket.emit('join_game', { game_id: gid });
        logTerminal.innerHTML = "";
        addLog("System: Connected to Bot Brain...");
    });

    socket.on('bot_log', (data) => {
        addLog(data.log);
    });
}

function addLog(text) {
    const div = document.createElement("div");
    div.classList.add("log-line");
    if (text.startsWith("info")) div.classList.add("info");
    else if (text.startsWith("bestmove")) div.classList.add("bestmove");
    else div.classList.add("system");
    div.innerText = text;
    logTerminal.appendChild(div);
    logTerminal.scrollTop = logTerminal.scrollHeight;
}

logToggle.addEventListener("click", () => {
    logContent.classList.toggle("open");
    logIcon.innerText = logContent.classList.contains("open") ? "▲" : "▼";
});

function resetUI() {
    results.innerHTML = "";
    playAgainBtn.style.display = "none";
    isLocked = false;
    turn = "X";
    updateTurnIndicator("X");
    board.classList.remove("board-locked");
    logTerminal.innerHTML = "";
}

async function handleBoxClick(e) {
    const index = parseInt(e.target.dataset.index);
    if (isLocked || boardState[index] !== null || e.target.innerHTML !== "") return;

    if (gameMode === "pvp") {
        handlePvpMove(index, e.target);
    } else {
        await handlePvcMove(index, e.target);
    }
}

function handlePvpMove(index, cell) {
    boardState[index] = turn;
    cell.innerHTML = turn;
    if (checkWinLocal(index, turn)) {
        endGame(turn);
    } else {
        turn = turn === "X" ? "O" : "X";
        updateTurnIndicator(turn);
    }
}

async function handlePvcMove(index, cell) {
    cell.innerHTML = playerRole;
    boardState[index] = playerRole;
    const aiRole = playerRole === "X" ? "O" : "X";
    updateTurnIndicator(aiRole);

    isLocked = true;
    board.classList.add("board-locked");

    try {
        const res = await fetch(`${API_URL}/move`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ game_id: gameId, index: index })
        });
        const data = await res.json();

        if (data.win) {
            const realWinner = getRealWinner(data.winner);
            if (data.winner === "X") {
                checkWinLocal(index, realWinner);
                endGame(realWinner);
                return;
            }
            if (data.move !== undefined) {
                updateAiMoveUI(data.move, aiRole);
                checkWinLocal(data.move, realWinner);
                endGame(realWinner);
                return;
            }
        }
        if (data.move !== undefined) {
            updateAiMoveUI(data.move, aiRole);
            updateTurnIndicator(playerRole);
            isLocked = false;
            board.classList.remove("board-locked");
        }
    } catch (err) {
        console.error(err);
        isLocked = false;
        board.classList.remove("board-locked");
    }
}

async function makeAiMove(lastIdx) {
    try {
        const res = await fetch(`${API_URL}/move`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ game_id: gameId, index: lastIdx }) 
        });
        const data = await res.json();
        
        if (data.move !== undefined) {
            const aiRole = playerRole === "X" ? "O" : "X";
            updateAiMoveUI(data.move, aiRole);
            updateTurnIndicator(playerRole);

            if (data.win) {
                const realWinner = getRealWinner(data.winner);
                checkWinLocal(data.move, realWinner);
                endGame(realWinner);
            } else {
                isLocked = false;
                board.classList.remove("board-locked");
            }
        }
    } catch (e) { console.error(e); }
}

function updateAiMoveUI(index, role) {
    const aiCell = document.querySelector(`div.box[data-index='${index}']`);
    if (aiCell) {
        aiCell.innerHTML = role;
        boardState[index] = role;
    }
}

function checkWinLocal(index, symbol) {
    const col = index % BOARD_SIZE;
    const row = Math.floor(index / BOARD_SIZE);
    const directions = [{x:1, y:0}, {x:0, y:1}, {x:1, y:1}, {x:1, y:-1}];
    for (const dir of directions) {
        let count = 1;
        let winCells = [index];
        for (let i = 1; i < WIN_LENGTH; i++) {
            const r = row + i * dir.y, c = col + i * dir.x;
            if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE || boardState[r * BOARD_SIZE + c] !== symbol) break;
            count++; winCells.push(r * BOARD_SIZE + c);
        }
        for (let i = 1; i < WIN_LENGTH; i++) {
            const r = row - i * dir.y, c = col - i * dir.x;
            if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE || boardState[r * BOARD_SIZE + c] !== symbol) break;
            count++; winCells.push(r * BOARD_SIZE + c);
        }
        if (count >= WIN_LENGTH) {
            winCells.forEach(i => {
                if (board.children[i]) board.children[i].classList.add("winning-cell");
            });
            return true;
        }
    }
    return false;
}

function endGame(winner) {
    isLocked = true;
    results.innerHTML = `${winner} Wins!`;
    playAgainBtn.style.display = "inline";
    board.classList.add("board-locked");
}

pvpBtn.addEventListener("click", () => startGame("pvp"));
pvcXBtn.addEventListener("click", () => startGame("pvc", "X"));
pvcOBtn.addEventListener("click", () => startGame("pvc", "O"));

playAgainBtn.addEventListener("click", async () => {
    if (gameMode === "pvc" && gameId) {
        await fetch(`${API_URL}/reset`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ game_id: gameId })
        });
        startGame("pvc", playerRole); 
    } else {
        startGame("pvp");
    }
});
