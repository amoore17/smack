/*
Copyright (C) 2019, Austin Moore, Zachary Wells

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

#include "state.hpp"
#include <fstream>

state::state()
{
    int32_t max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    status_bar = newwin(status_bar_height, max_x, 0, 0);
    box(status_bar, 0, 0);
    edit_window = newwin(max_y - status_bar_height, max_x, status_bar_height, 0);

    line = 0;
    column = 0;
    save_column = 0;
    mode = "VISUAL";

    page_start = 0;
    page_end = max_y - status_bar_height;
}

bool state::open_file(std::string& filename)
{
    file_name = filename;
    std::ifstream infile(filename);

    if (infile)
    {
        std::string line;

        while (std::getline(infile, line))
            lines.push_back(line);

        // Display file in the page range
        for (int32_t i = page_start; i < page_end; ++i)
        {
            if (i >= lines.size())
                break;

            wprintw(edit_window, "%s\n", lines[i].c_str());
            wrefresh(edit_window);
        }

        infile.close();
        return true;
    }
    else // Add an empty line to the new file and enter visual mode
    {
        lines.push_back("");
        return false;
    }
}
bool state::save_file()
{
    std::ofstream outfile(file_name);

    if (outfile)
    {
        for (auto line: lines)
            outfile << line << '\n';

        outfile.close();
        return true;
    }

    return false;
}
