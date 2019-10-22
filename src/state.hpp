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

#ifndef STATE_HPP
#define STATE_HPP

#include <string>
#include <vector>
#include <ncurses.h>

class state
{
public:
    state();

    WINDOW* edit_window; // The editing window
    WINDOW* status_bar; // The top status bar
    const int32_t status_bar_height = 3;
    std::string file_name;
    std::string mode; // The current mode. VISUAL and INSERT are the only modes
    std::vector<std::string> lines;
    int32_t line; // Indexed at 0 internally. The current line number in edit_window
                  // Note that the user will see the line as the current line number + 1
    int32_t column; // Indexed at 0 internally. The column in edit_window
                    // Note that the user will see the column as the current column number + 1
    int32_t page_start; // The line number at the top of the current display
    int32_t page_end; // The line number at the bottom of the current display
    int32_t save_column; // The column the user has navigated too
    
    //Returns true if the file was opened sucessfully
    bool open_file(std::string& filename);
    //Returns true if the file was saved sucessfully
    bool save_file();
};

#endif
