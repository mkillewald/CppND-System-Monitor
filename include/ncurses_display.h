#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

#include "process.h"
#include "system.h"

#define SYSTEM_WINDOW_ROWS 6

namespace NCursesDisplay {
// system info
const std::string kOs{"Operating System: "};
const std::string kKernel{"Kernel: "};
const std::string kUpTime{"Up Time: "};
const std::string kCpuCore{"CPU"};  // trailing colon ":" will be added
const std::string kMemory{"Memory:"};
const std::string kTotal{"Total Processes: "};
const std::string kRunning{"Running Processes: "};
const std::string kAlive{"Alive Processes: "};

// processes
const std::string kPid{"PID"};
const std::string kUser{"USER"};
const std::string kState{"S"};
const std::string kCpu{"CPU%%"};
const std::string kRam{"RAM[MB]"};
const std::string kTime{"TIME+"};
const std::string kCommand{"COMMAND"};

// menu
const std::string kHideCores{"Hide Cores"};
const std::string kShowCores{"Show Cores"};
const std::string kSortOrder{"Sort Order: "};
const std::string kQuit{"Quit"};

int Keypress(void);
void CheckEvents(System& system, WINDOW* system_w, WINDOW* process_w, int& n);
void ClearLine(WINDOW* window, int row);
void BoldUnderlineAndColor(WINDOW* window, int color, int row, int col,
                           std::string str, size_t pos = 0);
void AddColorChar(WINDOW* window, int color, chtype c);
void Resize(System& system, WINDOW* system_w, WINDOW* process_w, int& n);
std::string ProgressBar(float percent);
void SystemMenu(System& system, WINDOW* window, int row, int col);
void SystemInfo(System& system, WINDOW* window, int row, int col);
void CpuBars(System& sys, WINDOW* win, int& row, int col);
void MemoryBar(System& system, WINDOW* window, int row, int col);
void ProcessMenu(System& system, WINDOW* window, int row, int col);
void ProcessInfo(System& system, WINDOW* window, int row, int col);
void DisplaySystem(System& system, WINDOW* window);
void DisplayProcesses(System& system, WINDOW* window, int n);
void Display(System& system);
};  // namespace NCursesDisplay

#endif