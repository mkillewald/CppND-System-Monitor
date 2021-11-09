#ifndef NCURSES_DISPLAY_H
#define NCURSES_DISPLAY_H

#include <curses.h>

#include "process.h"
#include "system.h"

namespace NCursesDisplay {
void Display(System& system);
void DisplaySystem(System& system, WINDOW* window);
void DisplayProcesses(std::vector<Process>& processes, WINDOW* window, int n);
std::string ProgressBar(float percent);
int Keypress(void);
void Resize(WINDOW* system_w, WINDOW* process_w, int& n);
void CheckEvents(System& system, WINDOW* system_w, WINDOW* process_w, int& n);
};  // namespace NCursesDisplay

#endif