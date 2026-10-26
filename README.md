# gentle-todo (C++)

A guilt-free, terminal-based todo app written in C++17. No dependencies.

## Philosophy
- No due dates, no "overdue", no red warnings
- No streaks or completion percentages
- Moving a task to the bottom (`later`) or removing it (`rm`) is always okay
- Gentle wording everywhere

## Build
    make
(or: g++ -std=c++17 -O2 -o todo todo.cpp; on Windows: g++ -std=c++17 -o todo.exe todo.cpp)

## Use
    ./todo                       # interactive mode
    ./todo add "water plants"    # one-shot commands
    ./todo list
    ./todo done 2
    ./todo pick                  # suggests one task
    ./todo help                  # all commands

## Your data
All todos are stored in `todos.txt` in the folder you run the program from:

    [ ] open task
    [x] finished task

You can edit it by hand. To use another file:
    TODO_FILE=/path/to/my.txt ./todo list
