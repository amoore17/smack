/*
Copyright (C) 2019, Austin Moore

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef STATE_HH
#define STATE_HH

#include <string>
#include <vector>
#include <ncurses.h>

struct state
{
    state();

    WINDOW* edit_window;
    WINDOW* status_bar;
    std::string file_name;
    int32_t line; // Indexed at 0 internally. The line in edit_window
    int32_t column; // Indexed at 0 internally. The column in edit_window
    std::string mode;
    std::vector<std::string> lines;
};

state::state()
{
    int32_t max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    status_bar = newwin(3, max_x, 0, 0);
    box(status_bar, 0, 0);
    edit_window = newwin(max_y - 3, max_x, 3, 0);

    line = 0;
    column = 0;
    mode = "VISUAL";
}

#endif
