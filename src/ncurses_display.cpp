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

void NCursesDisplay::Resize(WINDOW* system_w, WINDOW* process_w, int& rows) {
  int new_x, new_y;
  getmaxyx(stdscr, new_y, new_x);
  wresize(system_w, SYSTEM_WINDOW_HEIGHT, new_x - 1);
  wresize(process_w, new_y - system_w->_maxy - 1, new_x - 1);
  rows = new_y - system_w->_maxy - 4;
  wclear(stdscr);
  wclear(system_w);
  wclear(process_w);
}

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
        Resize(system_w, process_w, process_rows);
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

void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  int x_max = getmaxx(window);
  mvwprintw(window, row, x_max - 10, "[ ");
  BoldUnderlineAndColor(window, 2, row, x_max - 8, "Quit");
  mvwprintw(window, row, x_max - 4, " ]");
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, ++row, 2, ("Kernel: " + system.Kernel()).c_str());
  mvwprintw(window, ++row, 2, "CPU: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.Cpu().Utilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.MemoryUtilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(system.RunningProcesses()) + "   ")
          .c_str());
  mvwprintw(window, row, 25,
            ("Alive Processes: " + to_string(system.Processes().size()) + "   ")
                .c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
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

  mvwprintw(window, row, max_x - 21, "[ Sort Order: ");
  bool descending = system.Descending();
  int key_color = descending ? 4 : 3;
  AddColorChar(window, key_color, '-');
  AddColorChar(window, 3, '/');
  key_color = descending ? 3 : 4;
  AddColorChar(window, key_color, '+');
  mvwprintw(window, row, max_x - 4, " ]");

  int color = system.Sort() == System::kPid_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, ++row, pid_column, "PID");
  color = system.Sort() == System::kUser_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, user_column, "USER");
  color = system.Sort() == System::kState_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, state_column, "S");
  color = system.Sort() == System::kCpu_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, cpu_column, "CPU%%");
  color = system.Sort() == System::kRam_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, ram_column, "RAM[MB]");
  color = system.Sort() == System::kUpTime_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, time_column, "TIME+");
  color = system.Sort() == System::kCommand_ ? 4 : 3;
  BoldUnderlineAndColor(window, color, row, command_column, "COMMAND", 1);

  std::vector<Process> processes = system.Processes();
  for (int i = 0; i < n; ++i) {
    //  Clear the line
    mvwprintw(window, ++row, pid_column,
              (string(window->_maxx - 2, ' ').c_str()));

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
              processes[i]
                  .Command()
                  .substr(0, window->_maxx - command_column)
                  .c_str());
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
  WINDOW* system_window = newwin(SYSTEM_WINDOW_HEIGHT, x_max - 1, 0, 0);
  WINDOW* process_window = newwin(y_max - system_window->_maxy - 1, x_max - 1,
                                  system_window->_maxy + 1, 0);

  int process_rows = y_max - system_window->_maxy - 4;

  while (1) {
    CheckEvents(system, system_window, process_window, process_rows);
    box(process_window, 0, 0);
    box(system_window, 0, 0);
    system.UpdateProcesses();
    DisplayProcesses(system, process_window, process_rows);
    DisplaySystem(system, system_window);
    wrefresh(process_window);
    wrefresh(system_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}