#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <ncurses.h>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

WINDOW *header_win, *chat_win, *input_win, *sidebar_win;
int max_y, max_x;

string username = "";
string current_room = "GLOBAL";
vector<string> rooms = {"GLOBAL"};

mutex ui_lock;
bool running = true;

// ================= UI =================

void init_ui() {
  initscr();
  cbreak();
  noecho();
  curs_set(1);
  keypad(stdscr, TRUE);

  start_color();
  use_default_colors();

  // Color pairs
  init_pair(1, COLOR_WHITE, COLOR_BLUE);    // header
  init_pair(2, COLOR_WHITE, COLOR_BLACK);   // normal text
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // mentions
  init_pair(4, COLOR_CYAN, COLOR_BLACK);    // other usernames
  init_pair(5, COLOR_GREEN, COLOR_BLACK);   // system messages
  init_pair(6, COLOR_WHITE, COLOR_BLACK);   // input box
  init_pair(7, COLOR_WHITE, COLOR_BLACK);   // sidebar
  init_pair(8, COLOR_MAGENTA, COLOR_BLACK); // own username

  getmaxyx(stdscr, max_y, max_x);

  // Create windows
  int sidebar_width = max_x / 5;

  header_win = newwin(1, max_x, 0, 0);
  sidebar_win = newwin(max_y - 4, sidebar_width, 1, 0);
  chat_win = newwin(max_y - 4, max_x - sidebar_width, 1, sidebar_width);
  input_win = newwin(3, max_x, max_y - 3, 0);

  scrollok(chat_win, TRUE);
  scrollok(sidebar_win, TRUE);

  wbkgd(header_win, COLOR_PAIR(1));
  wbkgd(sidebar_win, COLOR_PAIR(7));
  wbkgd(chat_win, COLOR_PAIR(2));
  wbkgd(input_win, COLOR_PAIR(6));

  box(input_win, 0, 0);
  box(sidebar_win, 0, 0);
  box(chat_win, 0, 0);

  wrefresh(header_win);
  wrefresh(sidebar_win);
  wrefresh(chat_win);
  wrefresh(input_win);
}

void draw_header() {
  werase(header_win);
  wbkgd(header_win, COLOR_PAIR(1));
  wattron(header_win, A_BOLD);

  string title = " CHAT APPLICATION ";
  mvwprintw(header_win, 0, 2, "%s", title.c_str());

  string room_info = " Room: " + current_room + " ";
  mvwprintw(header_win, 0, max_x / 2 - room_info.length() / 2, "%s",
            room_info.c_str());

  string user_info = " User: " + (username.empty() ? "?" : username) + " ";
  mvwprintw(header_win, 0, max_x - user_info.length() - 2, "%s",
            user_info.c_str());

  wattroff(header_win, A_BOLD);
  wrefresh(header_win);
}

void draw_sidebar() {
  werase(sidebar_win);
  wbkgd(sidebar_win, COLOR_PAIR(7));

  // Title
  wattron(sidebar_win, A_BOLD | COLOR_PAIR(4));
  mvwprintw(sidebar_win, 1, 2, "📁 ROOMS");
  wattroff(sidebar_win, A_BOLD | COLOR_PAIR(4));

  // List rooms
  for (size_t i = 0; i < rooms.size(); i++) {
    int y = i + 3;
    if (y >= max_y - 10)
      break;

    if (rooms[i] == current_room) {
      wattron(sidebar_win, A_REVERSE | A_BOLD);
    }

    string room_display = "  " + rooms[i];
    if (rooms[i] == "GLOBAL") {
      room_display = "🌍 " + rooms[i];
    } else {
      room_display = "📌 " + rooms[i];
    }

    mvwprintw(sidebar_win, y, 2, "%s", room_display.c_str());

    if (rooms[i] == current_room) {
      wattroff(sidebar_win, A_REVERSE | A_BOLD);
    }
  }

  // Commands section
  wattron(sidebar_win, A_BOLD | COLOR_PAIR(5));
  mvwprintw(sidebar_win, max_y - 12, 2, "⚡ COMMANDS");
  wattroff(sidebar_win, A_BOLD | COLOR_PAIR(5));

  mvwprintw(sidebar_win, max_y - 10, 2, "/join [room]");
  mvwprintw(sidebar_win, max_y - 9, 2, "/exit");
  mvwprintw(sidebar_win, max_y - 8, 2, "/online");
  mvwprintw(sidebar_win, max_y - 7, 2, "/clear");
  mvwprintw(sidebar_win, max_y - 6, 2, "/quit");

  box(sidebar_win, 0, 0);
  wrefresh(sidebar_win);
}

void clear_chat() {
  werase(chat_win);
  box(chat_win, 0, 0);
  wmove(chat_win, 1, 1);
  wrefresh(chat_win);
}

void print_message(const string &msg, bool is_own = false) {
  // Handle system messages
  if (msg.find("System:") == 0 || msg.find("Error:") == 0) {
    wattron(chat_win, COLOR_PAIR(5) | A_BOLD);
    wprintw(chat_win, "%s\n", msg.c_str());
    wattroff(chat_win, COLOR_PAIR(5) | A_BOLD);
  }
  // Handle user messages
  else {
    size_t colon_pos = msg.find(':');
    if (colon_pos != string::npos) {
      string name = msg.substr(0, colon_pos);
      string content = msg.substr(colon_pos + 1);

      // Print username with appropriate color
      if (is_own) {
        wattron(chat_win, COLOR_PAIR(8) | A_BOLD);
      } else {
        wattron(chat_win, COLOR_PAIR(4) | A_BOLD);
      }
      wprintw(chat_win, "%s:", name.c_str());
      wattroff(chat_win, A_BOLD | COLOR_PAIR(4) | COLOR_PAIR(8));

      // Check for mentions
      if (!username.empty() && content.find("@" + username) != string::npos) {
        wattron(chat_win, COLOR_PAIR(3) | A_BOLD);
        wprintw(chat_win, "%s\n", content.c_str());
        wattroff(chat_win, COLOR_PAIR(3) | A_BOLD);
      } else {
        wprintw(chat_win, "%s\n", content.c_str());
      }
    } else {
      wprintw(chat_win, "%s\n", msg.c_str());
    }
  }
  wrefresh(chat_win);
}

void print_own_message(const string &msg) {
  wattron(chat_win, COLOR_PAIR(8) | A_BOLD);
  wprintw(chat_win, "%s:", username.c_str());
  wattroff(chat_win, COLOR_PAIR(8) | A_BOLD);

  // Dim effect for own messages
  wattron(chat_win, A_DIM);
  wprintw(chat_win, "%s\n", msg.c_str());
  wattroff(chat_win, A_DIM);

  wrefresh(chat_win);
}

void redraw_input() {
  werase(input_win);
  box(input_win, 0, 0);

  if (username.empty()) {
    wattron(input_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(input_win, 1, 2, "👤 Enter your username: ");
    wattroff(input_win, A_BOLD | COLOR_PAIR(4));
    wmove(input_win, 1, 24);
  } else {
    wattron(input_win, COLOR_PAIR(4));
    mvwprintw(input_win, 1, 2, "[%s]", current_room.c_str());
    wattroff(input_win, COLOR_PAIR(4));
    wprintw(input_win, " > ");
    wmove(input_win, 1, getcurx(input_win));
  }

  wrefresh(input_win);
}

// ================= RECEIVE =================

void receive_messages(int sock) {
  char buffer[2048];

  while (running) {
    int bytes = read(sock, buffer, sizeof(buffer) - 1);

    if (bytes <= 0) {
      lock_guard<mutex> lock(ui_lock);
      wattron(chat_win, COLOR_PAIR(5) | A_BOLD);
      wprintw(chat_win, "\n⚠️ Disconnected from server\n");
      wprintw(chat_win, "Press any key to exit...\n");
      wattroff(chat_win, COLOR_PAIR(5) | A_BOLD);
      wrefresh(chat_win);
      running = false;
      break;
    }

    buffer[bytes] = '\0';
    string msg(buffer);

    lock_guard<mutex> lock(ui_lock);

    // Don't print if it's our own message (server might echo)
    // But if server doesn't echo, this won't filter anything
    size_t colon_pos = msg.find(':');
    if (colon_pos != string::npos) {
      string msg_sender = msg.substr(0, colon_pos);
      if (msg_sender != username) {
        print_message(msg);
      }
    } else {
      print_message(msg);
    }

    redraw_input();
  }
}

// ================= MAIN =================

int main() {
  int sock;
  sockaddr_in serv_addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket creation failed");
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

  if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connect failed");
    return 1;
  }

  init_ui();
  draw_header();
  draw_sidebar();

  thread receiver(receive_messages, sock);

  char input[512];

  // Welcome message
  {
    lock_guard<mutex> lock(ui_lock);
    wattron(chat_win, COLOR_PAIR(5) | A_BOLD);
    wprintw(chat_win, "╔════════════════════════════════════╗\n");
    wprintw(chat_win, "║     WELCOME TO THE CHAT ROOM      ║\n");
    wprintw(chat_win, "╚════════════════════════════════════╝\n\n");
    wattroff(chat_win, COLOR_PAIR(5) | A_BOLD);

    wprintw(chat_win, "Enter your username to begin\n");
    wprintw(chat_win, "Available commands:\n");
    wprintw(chat_win, "  • /join [room] - Join/Create room\n");
    wprintw(chat_win, "  • /exit - Return to GLOBAL\n");
    wprintw(chat_win, "  • /online - List online users\n");
    wprintw(chat_win, "  • /clear - Clear chat window\n");
    wprintw(chat_win, "  • /quit - Exit application\n\n");
    wrefresh(chat_win);

    redraw_input();
  }

  while (running) {
    redraw_input();

    // Get input
    echo();
    wgetnstr(input_win, input, sizeof(input) - 1);
    noecho();

    string msg(input);

    // Trim whitespace
    msg.erase(0, msg.find_first_not_of(" \n\r\t"));
    msg.erase(msg.find_last_not_of(" \n\r\t") + 1);

    if (msg.empty() || !running)
      continue;

    // Handle quit
    if (msg == "/quit") {
      send(sock, msg.c_str(), msg.size(), 0);
      break;
    }

    // First message = username
    if (username.empty()) {
      username = msg;
      send(sock, msg.c_str(), msg.size(), 0);

      lock_guard<mutex> lock(ui_lock);
      draw_header();

      wattron(chat_win, COLOR_PAIR(5));
      wprintw(chat_win, "✅ You joined as: %s\n", username.c_str());
      wattroff(chat_win, COLOR_PAIR(5));
      wrefresh(chat_win);
      continue;
    }

    // Handle local commands
    if (msg.rfind("/join", 0) == 0 && msg.length() > 6) {
      string room = msg.substr(6);

      // Trim room name
      room.erase(0, room.find_first_not_of(" \n\r\t"));
      room.erase(room.find_last_not_of(" \n\r\t") + 1);

      if (!room.empty()) {
        lock_guard<mutex> lock(ui_lock);

        // Add to rooms list if new
        bool exists = false;
        for (const auto &r : rooms) {
          if (r == room)
            exists = true;
        }
        if (!exists) {
          rooms.push_back(room);
        }

        current_room = room;
        draw_header();
        draw_sidebar();

        wattron(chat_win, COLOR_PAIR(5));
        wprintw(chat_win, "📌 Switched to room: %s\n", room.c_str());
        wattroff(chat_win, COLOR_PAIR(5));
        wrefresh(chat_win);
      }
    } else if (msg == "/exit") {
      lock_guard<mutex> lock(ui_lock);
      current_room = "GLOBAL";
      draw_header();
      draw_sidebar();

      wattron(chat_win, COLOR_PAIR(5));
      wprintw(chat_win, "📌 Returned to GLOBAL room\n");
      wattroff(chat_win, COLOR_PAIR(5));
      wrefresh(chat_win);
      continue;
    } else if (msg == "/clear") {
      lock_guard<mutex> lock(ui_lock);
      clear_chat();
      continue;
    }

    // Send message to server
    if (send(sock, msg.c_str(), msg.size(), 0) < 0) {
      lock_guard<mutex> lock(ui_lock);
      wattron(chat_win, COLOR_PAIR(5) | A_BOLD);
      wprintw(chat_win, "❌ Failed to send message\n");
      wattroff(chat_win, COLOR_PAIR(5) | A_BOLD);
      wrefresh(chat_win);
    }
    // Local echo for non-command messages
    else if (msg[0] != '/') {
      lock_guard<mutex> lock(ui_lock);
      print_own_message(msg);
    }
  }

  running = false;
  receiver.join();
  endwin();
  close(sock);

  cout << "Goodbye!" << endl;
  return 0;
}
