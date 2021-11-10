#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

#include "process.h"
#include "system.h"

#define SYSTEM_WINDOW_HEIGHT 9

namespace NCursesDisplay {
void BoldUnderlineAndColor(WINDOW* window, int color, int row, int col,
                           std::string str, size_t pos = 0);
void AddColorChar(WINDOW* window, int color, chtype c);
void Resize(WINDOW* system_w, WINDOW* process_w, int& n);
int Keypress(void);
void CheckEvents(System& system, WINDOW* system_w, WINDOW* process_w, int& n);
std::string ProgressBar(float percent);
void DisplaySystem(System& system, WINDOW* window);
void DisplayProcesses(System& system, WINDOW* window, int n);
void Display(System& system);
};  // namespace NCursesDisplay

#endif