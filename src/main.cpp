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
#include "state.hpp"

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

/*
 * Function: open_file
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Opens a file specified in argv in visual mode or opens a
 *              new file in visual mode
 */
void open_file(state& program_state)
{
    std::ifstream infile(program_state.file_name);

    if (infile)
    {
        std::string line;

        while (std::getline(infile, line))
            program_state.lines.push_back(line);

        // Display file in the page range
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
    else // Add an empty line to the new file and enter visual mode
    {
        program_state.lines.push_back("");
        visual_mode(program_state);
    }
}

/*
 * Function: refresh_edit_window
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Prints the file in the page range in the edit_window and
 *              refreshes the window
 */
void refresh_edit_window(state& program_state)
{
    werase(program_state.edit_window);

    // Display file in the page range
    for (int32_t i = program_state.page_start; i < program_state.page_end; ++i)
    {
        if (i >= program_state.lines.size())
            break;

        wprintw(program_state.edit_window, "%s\n", program_state.lines[i].c_str());
    }

    wrefresh(program_state.edit_window);
}

/*
 * Function: visual_mode
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Allows user to freely navigate the file without editing
 */
void visual_mode(state& program_state)
{
    program_state.mode = "VISUAL";
    noecho();

    // Move the cursor back if we are over the line size
    if (program_state.column >= program_state.lines[program_state.line].size() && program_state.lines[program_state.line].size() != 0)
    {
        program_state.column = program_state.lines[program_state.line].size() - 1;
        program_state.save_column = program_state.column;
    }
    
    update_position(program_state);
    int32_t c;

    while (true)
    {
        c = getch();

        switch (c)
        {
        case 'h':
            // Do not move cursor left of column 0
            if (program_state.column != 0)
            {
                program_state.column -= 1;
                program_state.save_column = program_state.column;
                update_position(program_state);
            }

            break;
        case 'j':
            // Do not move down passed the end of the file
            if (program_state.line < program_state.lines.size() - 1)
            {
                // Scroll if we are at the end of the "page"
                if (program_state.line + 1 >= program_state.page_end)
                {
                    program_state.page_start += 1;
                    program_state.page_end += 1;
                    refresh_edit_window(program_state);
                }

                // If the column below us is empty, switch the column number to 0
                if (program_state.lines[program_state.line + 1].size() == 0)
                    program_state.column = 0;
                else if (program_state.lines[program_state.line + 1].size() > program_state.save_column)
                    program_state.column = program_state.save_column;
                else if (program_state.lines[program_state.line + 1].size() <= program_state.save_column)
                    program_state.column = program_state.lines[program_state.line + 1].size() - 1;                

                program_state.line += 1;
                update_position(program_state);
            }

            break;
        case 'k':
            if (program_state.line != 0)
            {
                // Scroll if we are at the end of the "page"
                if (program_state.line - 1 < program_state.page_start)
                {
                    program_state.page_start -= 1;
                    program_state.page_end -= 1;
                    refresh_edit_window(program_state);
                }

                // If the column above us is empty, switch the column number to 0
                if (program_state.lines[program_state.line - 1].size() == 0)
                    program_state.column = 0;
                else if (program_state.lines[program_state.line - 1].size() > program_state.save_column)
                    program_state.column = program_state.save_column;
                else if (program_state.lines[program_state.line - 1].size() <= program_state.save_column)
                    program_state.column = program_state.lines[program_state.line - 1].size() - 1;

                program_state.line -= 1;
                update_position(program_state);
            }

            break;
        case 'l':
            // Do not exceed the length of the string on this line
            if ((program_state.column < program_state.lines[program_state.line].size() - 1) && program_state.lines[program_state.line].size() != 0 )
            {
                program_state.column += 1;
                program_state.save_column = program_state.column;
                update_position(program_state);             
            }

            break;
        case 'a':
            // If the size of the line is not empty, move cursor forward
            if (program_state.lines[program_state.line].size() > 0)
            {
                if (program_state.column != program_state.lines[program_state.line].size())
                {
                    program_state.column += 1;
                    program_state.save_column = program_state.column;
                    insert_mode(program_state);
                }
            }
            else // Otherwise, keep the cursor as is
                insert_mode(program_state);

            break;
        case 'i': // Insert mode at current position
            insert_mode(program_state);
            break;
        case '^': // Beginning of line
            program_state.column = 0;
            program_state.save_column = program_state.column;
            update_position(program_state);
            break;
        case '$': // End of line
            program_state.column = program_state.lines[program_state.line].size() - 1;
            program_state.save_column = program_state.column;
            update_position(program_state);
            break;
        case 'A': // End of line and insert
            program_state.column = program_state.lines[program_state.line].size();
            program_state.save_column = program_state.column;
            insert_mode(program_state);
            break;
        case 's': // Save the file
            save_file(program_state);
            break;
        case 'q': // Quit the program
            endwin();
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    getch();
}

/*
 * Function: insert_mode
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Allows user to edit the file interactively
 */
void insert_mode(state& program_state)
{
    program_state.mode = "INSERT";
    update_position(program_state);
    int32_t c;

    while (c = getch())
    {
        if (c == 27) // ESC
            visual_mode(program_state);
        else if (c == 127) // BACKSPACE
        {
            // If the column is not empty, remove a character
            if (program_state.column > 0)
            {
                mvwdelch(program_state.edit_window, program_state.line - program_state.page_start, program_state.column - 1);
                program_state.lines[program_state.line].erase(program_state.lines[program_state.line].begin() + program_state.column - 1);
                program_state.column -= 1;
                program_state.save_column = program_state.column;
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
                program_state.save_column = program_state.column;
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
            program_state.save_column = program_state.column;
            update_position(program_state);
        }
        else // Normal character insertion
        {
            winsch(program_state.edit_window, c);
            char character = char(c);

            program_state.lines[program_state.line].insert(program_state.lines[program_state.line].begin() + program_state.column, character);

            program_state.column += 1;
            program_state.save_column = program_state.column;
            update_position(program_state);
        }
    }
}


/*
 * Function: update_position
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Updates the status bar and moves the cursor to where it
 *              it needs in the edit_window which means a programmer should
 *              use this function after programmatically changing the line or
 *              column value
 */
void update_position(state& program_state)
{
    werase(program_state.status_bar);
    box(program_state.status_bar, 0, 0);
    mvwprintw(program_state.status_bar, 1, 1, "%s    %s    (%d, %d)", program_state.mode.c_str(), program_state.file_name.c_str(), program_state.line + 1, program_state.column + 1);
    wrefresh(program_state.status_bar);
    wmove(program_state.edit_window, program_state.line - program_state.page_start, program_state.column);
    wrefresh(program_state.edit_window);
}

/*
 * Function: save_file
 * Parameters: state& program_state - The state of the editor at the time
 * Description: Write the vector of strings (lines) out to the desired file
 */
void save_file(state& program_state)
{
    std::ofstream outfile(program_state.file_name);

    for (auto line: program_state.lines)
        outfile << line << '\n';

    outfile.close();
}
