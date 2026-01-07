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

### 1. Cài đặt thư viện Python
Di chuyển vào thư mục `backend` và cài đặt các thư viện cần thiết:

```bash
cd backend
pip install flask flask-socketio flask-cors eventlet
```

### 2. Biên dịch AI Engine (C++)
Hệ thống cần biên dịch mã nguồn C++ thành các file thực thi (binary) để Python có thể gọi được.

#### 👉 Đối với Linux / MacOS / WSL
Chạy script tự động đã được chuẩn bị sẵn:

```bash
cd backend
chmod +x run.sh
./run.sh
```

#### 👉 Đối với Windows (Command Prompt / PowerShell)
Nếu không dùng WSL, bạn cần biên dịch thủ công bằng lệnh sau (đảm bảo đã cài g++):

```cmd
cd backend
g++ -O3 modules/models/bot_level_1.cpp -o modules/models/bot_level_1.exe
g++ -O3 modules/models/bot_level_2.cpp -o modules/models/bot_level_2.exe
g++ -O3 modules/models/bot_level_3.cpp -o modules/models/bot_level_3.exe
g++ -O3 modules/models/bot_final.cpp -o modules/models/bot_final.exe
g++ -O3 modules/logic/engine.cpp -o modules/logic/engine.exe
```
*Lưu ý: Nếu chạy trên Windows thuần, bạn cần sửa lại đường dẫn trong file `server.py` và `config.py` để trỏ đúng đến file `.exe`.*

---

## ▶️ Hướng dẫn Chạy ứng dụng

### Bước 1: Khởi động Backend Server
Tại thư mục `backend`, chạy lệnh:

```bash
python server.py
```
Nếu thành công, bạn sẽ thấy thông báo server đang chạy tại `http://127.0.0.1:5000`.

### Bước 2: Mở Giao diện (Frontend)
1. Đi tới thư mục `frontend`.
2. Mở file `index.html` bằng trình duyệt web bất kỳ (Chrome, Edge, Firefox...).
3. Chọn chế độ chơi và bắt đầu trải nghiệm!

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
