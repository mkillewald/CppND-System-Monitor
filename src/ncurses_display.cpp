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
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime())).c_str());
  wrefresh(window);
}

void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes,
                                      WINDOW* window, int n) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const state_column{17};
  int const cpu_column{20};
  int const ram_column{26};
  int const time_column{35};
  int const command_column{46};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, state_column, "S");
  mvwprintw(window, row, cpu_column, "CPU%%");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
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

  int x_max, y_max;
  getmaxyx(stdscr, y_max, x_max);
  WINDOW* system_window = newwin(9, x_max - 1, 0, 0);
  WINDOW* process_window =
      newwin(y_max - 9, x_max - 1, system_window->_maxy + 1, 0);

  int process_rows = y_max - system_window->_maxy - 4;

  while (1) {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    CheckEvents(system, system_window, process_window, process_rows);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplayProcesses(system.Processes(), process_window, process_rows);
    DisplaySystem(system, system_window);
    wrefresh(process_window);
    wrefresh(system_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  endwin();
}

void NCursesDisplay::Resize(WINDOW* system_w, WINDOW* process_w, int& rows) {
  int new_x, new_y;
  getmaxyx(stdscr, new_y, new_x);
  wresize(system_w, 9, new_x - 1);
  wresize(process_w, new_y - 9, new_x - 1);
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
        // sort by max CPU
        system.SetSort(System::Sort::kMaxCpu_);
        break;
      case 'r':
      case 'R':
        // sort by max Ram
        system.SetSort(System::Sort::kMaxRam_);
        break;
      case 'p':
      case 'P':
        // sort by max Pid
        system.SetSort(System::Sort::kMaxPid_);
        break;
      case 's':
      case 'S':
        // sort by max State
        system.SetSort(System::Sort::kMaxState_);
        break;
      case '-': {
        // descending sort
        int sort = system.GetSort();
        if (sort % 2 != 0) {
          system.SetSort(sort - 1);
        }
        break;
      }
      case '=': {
        // ascending sort
        int sort = system.GetSort();
        if (sort % 2 == 0) {
          system.SetSort(sort + 1);
        }
        break;
      }
      default:;
    }
    flushinp();
  }
}