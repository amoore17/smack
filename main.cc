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

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <ncurses.h>
#include "state.hh"

void open_file(state& program_state);
void refresh_edit_window(state& program_state);
void visual_mode(state& program_state);
void insert_mode(state& program_state);
void update_position(state& program_state);
void save_file(state& program_state);

int32_t main(int32_t argc, char** argv)
{
    if (argc == 2)
    {
        initscr();
        state program_state;

        refresh();
        wrefresh(program_state.status_bar);
        wrefresh(program_state.edit_window);

        std::string file_name(argv[1]);
        program_state.file_name = file_name;
        open_file(program_state);
        endwin();
        exit(EXIT_SUCCESS);
    }
    else
    {
        std::cerr << "Invalid number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}

void open_file(state& program_state)
{
    std::ifstream infile(program_state.file_name);

    if (infile)
    {
        std::string line;

        while (std::getline(infile, line))
            program_state.lines.push_back(line);

        for (int32_t i = program_state.page_start; i < program_state.page_end; ++i)
        {
            if (i >= program_state.lines.size())
                break;

            wprintw(program_state.edit_window, "%s\n", program_state.lines[i].c_str());
            wrefresh(program_state.edit_window);
        }

        infile.close();
        visual_mode(program_state);
    }
    else
    {
        program_state.lines.push_back("");
        visual_mode(program_state);
    }
}

void refresh_edit_window(state& program_state)
{
    werase(program_state.edit_window);

    for (int32_t i = program_state.page_start; i < program_state.page_end; ++i)
    {
        if (i >= program_state.lines.size())
            break;

        wprintw(program_state.edit_window, "%s\n", program_state.lines[i].c_str());
    }

    wrefresh(program_state.edit_window);
}

void visual_mode(state& program_state)
{
    program_state.mode = "VISUAL";
    noecho();

    if (program_state.column >= program_state.lines[program_state.line].size())
        program_state.column = program_state.lines[program_state.line].size() - 1;
    
    update_position(program_state);
    int32_t c;

    while (true)
    {
        c = getch();

        switch (c)
        {
        case 'h':
            if (program_state.column != 0)
            {
                program_state.column -= 1;
                update_position(program_state);
            }

            break;
        case 'j':
            if (program_state.line < program_state.lines.size() - 1)
            {
                if (program_state.line + 1 >= program_state.page_end)
                {
                    program_state.page_start += 1;
                    program_state.page_end += 1;
                    refresh_edit_window(program_state);
                }

                if (program_state.lines[program_state.line + 1].size() == 0)
                    program_state.column = 0;
                else if (program_state.column > program_state.lines[program_state.line + 1].size() - 1)
                    program_state.column = program_state.lines[program_state.line + 1].size() - 1;

                program_state.line += 1;
                update_position(program_state);
            }

            break;
        case 'k':
            if (program_state.line != 0)
            {
                if (program_state.line - 1 < program_state.page_start)
                {
                    program_state.page_start -= 1;
                    program_state.page_end -= 1;
                    refresh_edit_window(program_state);
                }

                if (program_state.lines[program_state.line - 1].size() == 0)
                    program_state.column = 0;
                else if (program_state.column > program_state.lines[program_state.line - 1].size() - 1)
                    program_state.column = program_state.lines[program_state.line - 1].size() - 1;

                program_state.line -= 1;
                update_position(program_state);
            }

            break;
        case 'l':
            if ((program_state.column < program_state.lines[program_state.line].size() - 1) && program_state.lines[program_state.line].size() != 0 )
            {
                program_state.column += 1;
                update_position(program_state);             
            }

            break;
        case 'a':
            if (program_state.lines[program_state.line].size() > 0)
            {
                if (program_state.column != program_state.lines[program_state.line].size())
                {
                    program_state.column += 1;
                    insert_mode(program_state);
                }
            }
            else
                insert_mode(program_state);

            break;
        case 'i':
            insert_mode(program_state);
            break;
        case '^':
            program_state.column = 0;
            update_position(program_state);
            break;
        case '$':
            program_state.column = program_state.lines[program_state.line].size() - 1;
            update_position(program_state);
            break;
        case 's':
            save_file(program_state);
            break;
        case 'q':
            endwin();
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    getch();
}

void insert_mode(state& program_state)
{
    //echo();
    program_state.mode = "INSERT";
    update_position(program_state);
    int32_t c;

    while (c = getch())
    {
        if (c == 27) // ESC
            visual_mode(program_state);
        else if (c == 127) // BACKSPACE
        {
            if (program_state.column > 0)
            {
                mvwdelch(program_state.edit_window, program_state.line - program_state.page_start, program_state.column - 1);
                program_state.lines[program_state.line].erase(program_state.lines[program_state.line].begin() + program_state.column - 1);
                program_state.column -= 1;
                update_position(program_state);
            }
            else if (program_state.line > 0 && program_state.column == 0) // If we need to move back a line
            {
                std::string new_line = program_state.lines[program_state.line].substr(program_state.column, program_state.lines[program_state.line].size());
                int32_t last_line_size = program_state.lines[program_state.line - 1].size();
                program_state.lines[program_state.line - 1] += new_line;
                program_state.lines.erase(program_state.lines.begin() + program_state.line);
                program_state.line -= 1;
                program_state.column = last_line_size;
                refresh_edit_window(program_state);
                update_position(program_state);
            }
        }
        else if (c == 10) // ENTER
        {
            std::string new_line = program_state.lines[program_state.line].substr(program_state.column, program_state.lines[program_state.line].size());
            program_state.lines[program_state.line].erase(program_state.column, new_line.size());
            program_state.lines.insert(program_state.lines.begin() + program_state.line + 1, new_line);
            refresh_edit_window(program_state);
            program_state.line += 1;
            program_state.column = 0;
            update_position(program_state);
        }
        else
        {
            winsch(program_state.edit_window, c);
            char character = char(c);

            program_state.lines[program_state.line].insert(program_state.lines[program_state.line].begin() + program_state.column, character);

            program_state.column += 1;
            update_position(program_state);
        }
    }
}


void update_position(state& program_state)
{
    werase(program_state.status_bar);
    box(program_state.status_bar, 0, 0);
    mvwprintw(program_state.status_bar, 1, 1, "%s    %s    (%d, %d)", program_state.mode.c_str(), program_state.file_name.c_str(), program_state.line + 1, program_state.column + 1);
    wrefresh(program_state.status_bar);
    wmove(program_state.edit_window, program_state.line - program_state.page_start, program_state.column);
    wrefresh(program_state.edit_window);
}

void save_file(state& program_state)
{
    std::ofstream outfile(program_state.file_name);

    for (auto line: program_state.lines)
        outfile << line << '\n';

    outfile.close();
}
