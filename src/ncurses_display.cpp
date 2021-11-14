#include "ncurses_display.h"

#include <curses.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "system.h"

using std::string;
using std::to_string;

/*
 * taken from:
 * https://stackoverflow.com/questions/4025891/create-a-function-to-check-for-key-press-in-unix-using-ncurses
 */
int NCursesDisplay::Keypress(void) {
  int ch = getch();

  if (ch != ERR) {
    ungetch(ch);
    return 1;
  } else {
    return 0;
  }
}

void NCursesDisplay::CheckEvents(System& system, WINDOW* system_w,
                                 WINDOW* process_w, int& process_rows) {
  if (Keypress()) {
    switch (getch()) {
      case KEY_RESIZE:
        Resize(system, system_w, process_w, process_rows);
        break;
      case 'h':
      case 'H':
        system.ToggleCores();
        Resize(system, system_w, process_w, process_rows);
        break;
      case 'q':
      case 'Q':
        // Quit program
        endwin();
        exit(0);
        break;
      case 'c':
      case 'C':
        // sort by CPU
        system.SetSort(System::kCpu_);
        break;
      case 'o':
      case 'O':
        // sort by Command
        system.SetSort(System::kCommand_);
        break;
      case 'p':
      case 'P':
        // sort by Pid
        system.SetSort(System::kPid_);
        break;
      case 'r':
      case 'R':
        // sort by Ram
        system.SetSort(System::kRam_);
        break;
      case 's':
      case 'S':
        // sort by State
        system.SetSort(System::kState_);
        break;
      case 't':
      case 'T':
        // sort by UpTime
        system.SetSort(System::kUpTime_);
        break;
      case 'u':
      case 'U':
        // sort by User
        system.SetSort(System::kUser_);
        break;
      case '_':
      case '-': {
        // descending sort
        system.SetDescending(true);
        break;
      }
      case '+':
      case '=': {
        // ascending sort
        system.SetDescending(false);
        break;
      }
      default:;
    }
    // because our main loop sleeps 1 sec every iteration, clearing the
    // input buffer after receiving 1 char seems like a good idea to stop input
    // chars from piling up.
    flushinp();
  }
}

void NCursesDisplay::ClearLine(WINDOW* window, int row) {
  mvwprintw(window, row, 1, (string(window->_maxx - 1, ' ').c_str()));
}

/*
 * Sets color to entire string and applies bold and underline attributes to a
 * single character at given position. Defaults to first character if position
 * not supplied.
 */
void NCursesDisplay::BoldUnderlineAndColor(WINDOW* window, int color, int row,
                                           int col, string str, size_t pos) {
  wattron(window, COLOR_PAIR(color));
  if (pos > 0) {
    mvwprintw(window, row, col, str.substr(0, pos).c_str());
  }
  wattron(window, A_BOLD);
  wattron(window, A_UNDERLINE);
  mvwprintw(window, row, col + pos, str.substr(pos, 1).c_str());
  wattroff(window, A_UNDERLINE);
  wattroff(window, A_BOLD);
  if (str.size() > pos) {
    mvwprintw(window, row, col + pos + 1,
              str.substr(pos + 1, string::npos).c_str());
  }
  wattroff(window, COLOR_PAIR(color));
}

/*
 * Colorizes the input char before adding it to the given window at its current
 * cursor position
 */
void NCursesDisplay::AddColorChar(WINDOW* window, int color, chtype c) {
  wattron(window, COLOR_PAIR(color));
  waddch(window, c);
  wattroff(window, COLOR_PAIR(color));
}

void NCursesDisplay::Resize(System& system, WINDOW* system_w, WINDOW* process_w,
                            int& rows) {
  int new_x, new_y;
  getmaxyx(stdscr, new_y, new_x);
  int system_window_height;
  if (system.ShowCores()) {
    system_window_height = SYSTEM_SHOW_CORE_STATIC_ROWS + system.TotalCpus();
  } else {
    system_window_height = SYSTEM_HIDE_CORE_STATIC_ROWS;
  }
  wresize(system_w, system_window_height, new_x);
  if (system.ShowCores()) {
    wresize(process_w, new_y - system_window_height, new_x);
    mvwin(process_w, system_window_height, 0);
  } else {
    mvwin(process_w, system_window_height, 0);
    wresize(process_w, new_y - system_window_height, new_x);
  }
  rows = new_y - system_w->_maxy - 4;
  wclear(stdscr);
  wclear(system_w);
  wclear(process_w);
  wrefresh(process_w);
  wrefresh(system_w);
  refresh();
}

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}

void NCursesDisplay::SystemMenu(System& sys, WINDOW* win, int& row, int col) {
  mvwprintw(win, row, col, "[ ");
  col += 2;
  if (sys.ShowCores()) {
    BoldUnderlineAndColor(win, 3, row, col, kHideCores);
    col += kHideCores.size();
  } else {
    BoldUnderlineAndColor(win, 3, row, col, kShowCores, 1);
    col += kShowCores.size();
  }
  mvwprintw(win, row, col, " ]");
  col += 4;
  mvwprintw(win, row, col, "[ ");
  col += 2;
  BoldUnderlineAndColor(win, 2, row, col, kQuit.c_str());
  col += kQuit.size();
  mvwprintw(win, row, col, " ]");
}

void NCursesDisplay::SystemInfo(System& sys, WINDOW* win, int& row, int col) {
  std::string os = kOs + sys.OperatingSystem();
  std::string kernel = kKernel + sys.Kernel();
  std::string uptime = kUpTime + Format::ElapsedTime(sys.UpTime());
  if (sys.ShowCores()) {
    mvwprintw(win, row, col, os.c_str());
    mvwprintw(win, row, (win->_maxx - kernel.size()) / 2, kernel.c_str());
    mvwprintw(win, row, win->_maxx - uptime.size() - 1, uptime.c_str());
  } else {
    mvwprintw(win, row, col, os.c_str());
    mvwprintw(win, row, win->_maxx - uptime.size() - 1, uptime.c_str());
    mvwprintw(win, ++row, col, kernel.c_str());
  }
}

void NCursesDisplay::CpuBars(System& sys, WINDOW* win, int& row, int col) {
  mvwprintw(win, row, col, (kCpuCore + ":").c_str());
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, row, col + 8, "");
  wprintw(win, ProgressBar(sys.Cpu().Utilization()).c_str());
  wattroff(win, COLOR_PAIR(1));

  if (sys.ShowCores()) {
    std::vector<Processor> cpus = sys.Cpus();
    for (auto& cpu : cpus) {
      mvwprintw(win, ++row, col,
                (kCpuCore + to_string(cpu.Id()) + ":").c_str());
      wattron(win, COLOR_PAIR(1));
      mvwprintw(win, row, col + 8, "");
      wprintw(win, ProgressBar(cpu.Utilization()).c_str());
      wattroff(win, COLOR_PAIR(1));
    }
  }
}

void NCursesDisplay::MemoryBar(System& sys, WINDOW* win, int& row, int col) {
  mvwprintw(win, row, col, kMemory.c_str());
  wattron(win, COLOR_PAIR(1));
  mvwprintw(win, row, col + 8, "");  // Tab
  wprintw(win, ProgressBar(sys.MemoryUtilization()).c_str());
  wattroff(win, COLOR_PAIR(1));
}

void NCursesDisplay::ProcessMenu(System& sys, WINDOW* win, int& row, int col) {
  string sort_order = "[ " + kSortOrder;
  mvwprintw(win, row, col, sort_order.c_str());
  bool descending = sys.Descending();
  int key_color = descending ? 4 : 3;
  AddColorChar(win, key_color, '-');
  AddColorChar(win, 3, '/');
  key_color = descending ? 3 : 4;
  AddColorChar(win, key_color, '+');
  mvwprintw(win, row, col + sort_order.size() + 3, " ]");
}

void NCursesDisplay::ProcessInfo(System& sys, WINDOW* win, int& row, int col) {
  std::string total = kTotal + to_string(sys.TotalProcesses());
  std::string running = kRunning + to_string(sys.RunningProcesses());
  std::string alive = kAlive + to_string(sys.Processes().size());
  if (sys.ShowCores()) {
    ClearLine(win, row);
    mvwprintw(win, row, col, running.c_str());
    mvwprintw(win, row, (win->_maxx - alive.size()) / 2, alive.c_str());
    mvwprintw(win, row, win->_maxx - total.size() - 1, total.c_str());
  } else {
    ClearLine(win, row);
    mvwprintw(win, row, col, running.c_str());
    mvwprintw(win, row, col + 2 + running.size(), alive.c_str());
    ClearLine(win, ++row);
    mvwprintw(win, row, col, total.c_str());
  }
}

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  int x_max = getmaxx(window);
  SystemMenu(system, window, row, x_max - 26);
  SystemInfo(system, window, ++row, 2);
  CpuBars(system, window, ++row, 2);
  MemoryBar(system, window, ++row, 2);
  ProcessInfo(system, window, ++row, 2);
}

void NCursesDisplay::DisplayProcesses(System& system, WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{10};
  int const state_column{18};
  int const cpu_column{21};
  int const ram_column{27};
  int const time_column{36};
  int const command_column{47};
  int max_x = getmaxx(window);

  ProcessMenu(system, window, row, max_x - 21);

  // Column headings
  int color = system.Sort() == System::kPid_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, ++row, pid_column, kPid);
  color = system.Sort() == System::kUser_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, user_column, kUser);
  color = system.Sort() == System::kState_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, state_column, kState);
  color = system.Sort() == System::kCpu_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, cpu_column, kCpu);
  color = system.Sort() == System::kRam_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, ram_column, kRam);
  color = system.Sort() == System::kUpTime_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, time_column, kTime);
  color = system.Sort() == System::kCommand_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, command_column, kCommand, 1);

  // Processes
  std::vector<Process> processes = system.Processes();
  for (int i = 0; i < n; ++i) {
    ClearLine(window, ++row);

    if ((size_t)i > processes.size() - 1) {
      continue;
    }

    mvwprintw(window, row, pid_column, to_string(processes[i].Pid()).c_str());
    mvwprintw(
        window, row, user_column,
        processes[i].User().substr(0, state_column - user_column - 2).c_str());
    mvwprintw(window, row, state_column, processes[i].State().c_str());
    float cpu = processes[i].CpuUtilization() * 100;
    mvwprintw(window, row, cpu_column, to_string(cpu).substr(0, 4).c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().substr(0, 7).c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    mvwprintw(window, row, command_column,
              processes[i].Command(window->_maxx - command_column - 1).c_str());
  }
}

void NCursesDisplay::Display(System& system) {
  initscr();              // start ncurses
  noecho();               // do not print input values
  cbreak();               // terminate ncurses on ctrl + c
  start_color();          // enable color
  curs_set(0);            // hide cursor
  nodelay(stdscr, TRUE);  // make getch() non-blocking

  init_pair(1, COLOR_BLUE, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_MAGENTA, COLOR_BLACK);

  int x_max, y_max;
  getmaxyx(stdscr, y_max, x_max);
  int system_window_height = SYSTEM_SHOW_CORE_STATIC_ROWS + system.TotalCpus();
  WINDOW* system_window = newwin(system_window_height, x_max, 0, 0);
  WINDOW* process_window = newwin(y_max - system_window->_maxy - 1, x_max,
                                  system_window->_maxy + 1, 0);

  int process_rows = y_max - system_window->_maxy - 4;

  while (1) {
    CheckEvents(system, system_window, process_window, process_rows);
    box(process_window, 0, 0);
    box(system_window, 0, 0);
    system.UpdateProcesses();
    system.UpdateProcessors();
    DisplayProcesses(system, process_window, process_rows);
    DisplaySystem(system, system_window);
    wrefresh(process_window);
    wrefresh(system_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}