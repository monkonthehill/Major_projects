# TermChat 🚀

A feature-rich terminal-based chat application with multi-room support, built with C++ and ncurses. Connect with others in real-time through a beautiful TUI (Terminal User Interface).

![TermChat Demo](/screenshots/main-chat.png)
*Main chat interface*

## ✨ Features

- **🎨 Beautiful TUI** - Dark-themed interface with color-coded messages
- **💬 Multi-room Chat** - Create and join different chat rooms
- **👤 User Mentions** - Get notified when someone mentions you (@username)
- **📋 Online Users** - See who's online in your current room
- **⚡ Real-time Updates** - Instant message delivery
- **🎯 Room Management** - Seamlessly switch between rooms
- **🔔 System Notifications** - Join/leave messages for room activity
- **📱 Responsive Design** - Adapts to terminal window size

## 📸 Screenshots

<div align="center">
  <img src="/screenshots/first.png" alt="Main Chat Interface" width="80%">
  <p><em>Main chat interface with sidebar and message area</em></p>
  
  <img src="/screenshots/second.png" alt="Room Switching" width="80%">
  <p><em>Switching between different chat rooms</em></p>
  
  <img src="/screenshots/third.png" alt="User Mentions" width="80%">
  <p><em>User mentions highlighted in yellow</em></p>
  
  <img src="/screenshots/fourth.png" alt="Online Users List" width="80%">
  <p><em>Viewing online users in current room</em></p>
</div>

## 🛠️ Technologies Used

- **C++17** - Core language
- **ncurses** - Terminal UI library
- **Socket Programming** - TCP/IP communication
- **POSIX Threads** - Concurrent message handling
- **poll()** - I/O multiplexing (server)

## 📋 Prerequisites

- Linux/Unix-based OS (macOS works too)
- C++ compiler with C++17 support
- ncurses library
- Make (optional)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install g++ libncurses5-dev make
---

## 🛠️ Tech Stack

* **C++17**
* **ncurses**
* **TCP sockets**
* **poll()**
* **pthread**

---

## ⚙️ Setup

### Install dependencies

**Ubuntu/Debian**

```bash
sudo apt install g++ libncurses5-dev
```

**macOS**

```bash
brew install ncurses
```

---

### Compile

```bash
g++ server.cpp -o server -pthread
g++ client.cpp -o client -lncurses -pthread
```

---

### Run

Start server:

```bash
./server
```

Run client(s):

```bash
./client
```

---

## 🎮 Commands

| Command      | Description      |
| ------------ | ---------------- |
| `/join room` | Join/create room |
| `/exit`      | Back to GLOBAL   |
| `/online`    | List users       |
| `/clear`     | Clear chat       |
| `/quit`      | Exit             |

---

## 🧠 How it Works

* Server uses `poll()` to handle multiple clients
* Each client is mapped as:

```
fd → username + room
```

* Messages are broadcast **only within the same room**

---

## ⚠️ Limitations

* No message persistence
* No encryption
* Basic TCP (no message framing)

---

## 🚀 Future Improvements

* Private messaging
* Message timestamps
* Better input handling
* WebSocket version

---

## 👨‍💻 Author

**Nitish**

---

## ⭐ Note

This project focuses on **low-level networking + terminal UI design**.
If you understand this, you're already beyond beginner level.

---

**Star the repo if you found it useful.**
