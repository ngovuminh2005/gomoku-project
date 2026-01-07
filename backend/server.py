import eventlet
eventlet.monkey_patch()

import subprocess, uuid, os, threading
from flask import Flask, request, jsonify
from flask_cors import CORS
from flask_socketio import SocketIO, emit
from config import *

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet', ping_timeout=10, ping_interval=5)

sessions = {}

class Manager:
    def __init__(self, model_name, game_id):
        self.game_id = game_id
        self.engine = subprocess.Popen([PATH_LOGIC], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
        
        model_exec = os.path.join(PATH_MODELS, model_name)
        self.ai = subprocess.Popen(
            [model_exec], 
            stdin=subprocess.PIPE, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            text=True, 
            bufsize=1 
        )
        
        self.log_thread = threading.Thread(target=self.read_logs, daemon=True)
        self.log_thread.start()

    def read_logs(self):
        try:
            for line in iter(self.ai.stderr.readline, ''):
                if line:
                    clean_line = line.strip()
                    if clean_line:
                        socketio.emit('bot_log', {'log': clean_line}, room=self.game_id)
        except Exception:
            pass

    def send_engine(self, idx, p_val):
        try:
            self.engine.stdin.write(f"{idx} {p_val}\n")
            return int(self.engine.stdout.readline().strip())
        except: return -1

    def get_ai(self, op_idx):
        try:
            self.ai.stdin.write(f"{op_idx}\n")
            return int(self.ai.stdout.readline().strip())
        except: return -1

    def close(self):
        try:
            self.engine.terminate()
            self.ai.terminate()
        except: pass

@app.route('/start', methods=['POST'])
def start():
    gid = str(uuid.uuid4())
    sessions[gid] = Manager(CURRENT_MODEL, gid)
    return jsonify({"game_id": gid})

@app.route('/move', methods=['POST'])
def move():
    data = request.json
    gid, idx = data.get('game_id'), data.get('index')
    if gid not in sessions: return jsonify({"error": "No session"}), 404
    mgr = sessions[gid]
    
    if idx != -1:
        st = mgr.send_engine(idx, 1)
        if st == -1: return jsonify({"error": "Invalid"}), 400
        if st == 1: return jsonify({"win": True, "winner": "X", "move": idx})
    
    ai_idx = mgr.get_ai(idx)
    st_ai = mgr.send_engine(ai_idx, 2)
    if st_ai == 1: return jsonify({"win": True, "winner": "O", "move": ai_idx})
    
    return jsonify({"win": False, "move": ai_idx})

@app.route('/reset', methods=['POST'])
def reset():
    gid = request.json.get('game_id')
    if gid in sessions:
        sessions[gid].close(); del sessions[gid]
    return start()

@socketio.on('join_game')
def handle_join(data):
    from flask_socketio import join_room
    join_room(data.get('game_id'))

if __name__ == '__main__':
    socketio.run(app, host=HOST, port=PORT, debug=True)