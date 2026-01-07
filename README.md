# Gomoku AI Web App (Cờ Caro)

Ứng dụng Cờ Caro (Gomoku) kết hợp giữa **Python (Flask)** làm Backend điều phối và **C++** làm Engine tính toán AI hiệu năng cao. Giao diện Frontend sử dụng HTML/CSS/JS thuần kết nối qua WebSocket.

## 📋 Yêu cầu hệ thống

Để chạy được dự án, máy tính cần cài đặt:
1. **Python 3.7+**
2. **Trình biên dịch G++** (GCC) để biên dịch mã nguồn C++.
   - *Linux/MacOS:* Thường đã có sẵn hoặc cài qua terminal (`sudo apt install g++`).
   - *Windows:* Cần cài MinGW hoặc sử dụng WSL (Windows Subsystem for Linux).

---

## 🚀 Hướng dẫn Cài đặt

###  Cài đặt thư viện Python
Di chuyển vào thư mục `backend` và cài đặt các thư viện cần thiết:

```bash
cd backend
pip install flask flask-socketio flask-cors eventlet
```

---

## 📂 Cấu trúc thư mục

```text
project/
├── backend/
│   ├── modules/
│   │   ├── logic/          # Logic game cơ bản (check win)
│   │   └── models/         # Các thuật toán AI (Level 1, 2, 3, Final)
│   ├── server.py           # Server Flask & SocketIO
│   ├── config.py           # Cấu hình port, đường dẫn
│   └── run.sh              # Script biên dịch C++ (Linux)
└── frontend/
    ├── index.html          # Giao diện chính
    ├── style.css           # Định dạng giao diện
    └── script.js           # Logic Frontend & kết nối Socket
```
