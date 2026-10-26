// gentle-todo: a guilt-free, terminal-based todo app (C++17).
// No due dates, no overdue warnings, no streaks, no shame.
// Todos are stored in a plain text file: todos.txt
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct Item {
    bool done;
    std::string text;
};

static std::string todoFile() {
    const char* env = std::getenv("TODO_FILE");
    return env ? env : "todos.txt";
}

static const char* HEADER =
    "# gentle-todo list. Edit freely by hand.\n"
    "# [ ] = open, [x] = done. Nothing here is overdue. Ever.\n";

static std::vector<Item> load() {
    std::vector<Item> items;
    std::ifstream in(todoFile());
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("[ ] ", 0) == 0) items.push_back({false, line.substr(4)});
        else if (line.rfind("[x] ", 0) == 0) items.push_back({true, line.substr(4)});
    }
    return items;
}

static void save(const std::vector<Item>& items) {
    std::string path = todoFile(), tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        out << HEADER;
        for (const auto& it : items) out << (it.done ? "[x] " : "[ ] ") << it.text << "\n";
    }
    std::rename(tmp.c_str(), path.c_str());
}

static void show(const std::vector<Item>& items, bool showDone = true) {
    if (items.empty()) {
        std::cout << "Your list is empty. Enjoy the calm, or add something with: add <task>\n";
        return;
    }
    bool anyOpen = false, anyDone = false;
    for (const auto& it : items) (it.done ? anyDone : anyOpen) = true;
    if (anyOpen) {
        std::cout << "Whenever you're ready:\n";
        for (size_t i = 0; i < items.size(); ++i)
            if (!items[i].done) std::cout << "  " << (i + 1) << ". [ ] " << items[i].text << "\n";
    } else {
        std::cout << "Nothing open right now. You're all caught up.\n";
    }
    if (showDone && anyDone) {
        std::cout << "\nAlready done:\n";
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].done) std::cout << "  " << (i + 1) << ". [x] " << items[i].text << "\n";
    }
}

static int getIndex(const std::vector<Item>& items, const std::string& arg) {
    try {
        size_t pos;
        int n = std::stoi(arg, &pos);
        if (pos == arg.size() && n >= 1 && n <= (int)items.size()) return n - 1;
    } catch (...) {}
    std::cout << "No task numbered '" << arg << "'. Try: list\n";
    return -1;
}

static std::string join(const std::vector<std::string>& v, size_t from) {
    std::string s;
    for (size_t i = from; i < v.size(); ++i) {
        if (i > from) s += " ";
        s += v[i];
    }
    return s;
}

static const char* HELP =
    "gentle-todo commands:\n"
    "  add <task>          add a task\n"
    "  list                show everything\n"
    "  open                show only open tasks\n"
    "  done <n>            mark task n done\n"
    "  undo <n>            reopen task n\n"
    "  edit <n> <text>     rewrite task n\n"
    "  later <n>           move task n to the bottom (no pressure)\n"
    "  rm <n>              let a task go\n"
    "  tidy                remove all finished tasks\n"
    "  pick                suggest one random open task\n"
    "  help / quit\n"
    "\nRun with no arguments for interactive mode, or one-shot: ./todo add \"water plants\"\n"
    "Your tasks live in todos.txt (or set TODO_FILE to use another path).\n";

static std::mt19937& rng() {
    static std::mt19937 g{std::random_device{}()};
    return g;
}

// returns false when the user wants to quit
static bool run(std::vector<Item>& items, const std::vector<std::string>& a) {
    if (a.empty()) return true;
    std::string c = a[0];
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);
    static const char* cheers[] = {"Nice one.", "Done. Well done.", "Lovely.", "One less thing.", "That counts."};

    if (c == "add" || c == "a") {
        std::string t = join(a, 1);
        if (t.empty()) std::cout << "Usage: add <task>\n";
        else { items.push_back({false, t}); save(items); std::cout << "Added: " << t << "\n"; }
    } else if (c == "list" || c == "ls" || c == "l") {
        show(items);
    } else if (c == "open" || c == "o") {
        show(items, false);
    } else if (c == "done" || c == "d" || c == "x") {
        if (a.size() < 2) { std::cout << "Usage: done <number>\n"; return true; }
        int i = getIndex(items, a[1]);
        if (i >= 0) {
            items[i].done = true; save(items);
            std::cout << cheers[rng()() % 5] << " (" << items[i].text << ")\n";
        }
    } else if (c == "undo" || c == "u" || c == "reopen") {
        if (a.size() < 2) { std::cout << "Usage: undo <number>\n"; return true; }
        int i = getIndex(items, a[1]);
        if (i >= 0) { items[i].done = false; save(items); std::cout << "Back on the list: " << items[i].text << "\n"; }
    } else if (c == "edit" || c == "e") {
        if (a.size() < 3) { std::cout << "Usage: edit <number> <new text>\n"; return true; }
        int i = getIndex(items, a[1]);
        if (i >= 0) { items[i].text = join(a, 2); save(items); std::cout << "Updated.\n"; }
    } else if (c == "later") {
        if (a.size() < 2) { std::cout << "Usage: later <number>\n"; return true; }
        int i = getIndex(items, a[1]);
        if (i >= 0) {
            Item it = items[i]; items.erase(items.begin() + i); items.push_back(it); save(items);
            std::cout << "Moved to the bottom. It'll wait for you.\n";
        }
    } else if (c == "rm" || c == "remove" || c == "del" || c == "delete") {
        if (a.size() < 2) { std::cout << "Usage: rm <number>\n"; return true; }
        int i = getIndex(items, a[1]);
        if (i >= 0) {
            std::string t = items[i].text; items.erase(items.begin() + i); save(items);
            std::cout << "Let go of: " << t << "  (changed your mind? that's allowed.)\n";
        }
    } else if (c == "tidy" || c == "clean") {
        size_t before = items.size();
        items.erase(std::remove_if(items.begin(), items.end(), [](const Item& it) { return it.done; }), items.end());
        save(items);
        std::cout << "Tidied away " << (before - items.size()) << " finished task(s).\n";
    } else if (c == "pick" || c == "p" || c == "one") {
        std::vector<const Item*> open;
        for (const auto& it : items) if (!it.done) open.push_back(&it);
        if (open.empty()) std::cout << "Nothing open. Rest up.\n";
        else {
            std::cout << "How about this one? -> " << open[rng()() % open.size()]->text << "\n";
            std::cout << "(Totally fine to skip it.)\n";
        }
    } else if (c == "help" || c == "h" || c == "?") {
        std::cout << HELP;
    } else if (c == "quit" || c == "exit" || c == "q") {
        return false;
    } else {
        std::cout << "Not sure what '" << c << "' means. Type: help\n";
    }
    return true;
}

int main(int argc, char** argv) {
    std::vector<Item> items = load();
    if (argc > 1) {
        std::vector<std::string> args(argv + 1, argv + argc);
        run(items, args);
        return 0;
    }
    std::cout << "gentle-todo. No deadlines, no guilt. Type 'help' for commands.\n\n";
    show(items, false);
    std::string line;
    while (true) {
        std::cout << "\n> " << std::flush;
        if (!std::getline(std::cin, line)) { std::cout << "\nTake care.\n"; break; }
        std::istringstream ss(line);
        std::vector<std::string> args;
        std::string w;
        while (ss >> w) args.push_back(w);
        if (args.empty()) continue;
        if (!run(items, args)) { std::cout << "Take care.\n"; break; }
    }
    return 0;
}
