/*
Combined UI implementation file.
Merged from: monsters.cpp, scene.cpp, user_interaction.cpp
*/

#include <ncurses.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ui_ux.h"

using namespace std;

// ===== Scene assets =====
string tree = R"(
    /\
   /**\
  /****\
 /******\
/********\
   ||||
   ||||
)";

string ground = R"(
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
)";

string title = R"(
   ____  _   _ ____  ____  _____ ____
    / ___|| | | |  _ \|  _ \| ____|  _ \
    \___ \| | | | |_) | |_) |  _| | |_) |
   ___) | |_| |  __/|  __/| |___|  _ <
    |____/ \___/|_|   |_|   |_____|_| \_\
)";

void show_HP() {
}

// ===== Monster assets =====
Monster ghost = {
    "Ghost",
    R"(
 .------.
/  #   # \
|        |
~` ~` ~ `~
)",
    "Ethereal Strike: Deals damage that ignores armor.",
    "Incorporeal Form: Reduces physical damage taken by 50%.",
    "Haunting Presence: Chance to frighten enemies, reducing their attack power."
};

Monster mushroom = {
    "Mushroom",
    R"(
 .-o-OO-o-.
(__________)
 |  *   * |
 |________|
)",
    "Spore Burst: Releases spores that damage and poison enemies.",
    "Fungal Shield: Creates a temporary shield that absorbs damage.",
    "Regeneration: Heals over time when not taking damage."
};

Monster owl = {
    "Owl",
    R"(
  /\ /\
((@ v @))
() ::: ()
  VV VV
)",
    "Silent Flight: Allows the owl to move without making noise, avoiding detection.",
    "Keen Vision: Increases accuracy and critical hit chance.",
    "Night Hunter: Gains increased damage and evasion during nighttime."
};

Monster blob = {
    "Blob",
    R"(
    .----.
   ( @  @ )
   (      )
   `------`
)",
    "Acidic Touch: Deals damage over time and reduces enemy armor.",
    "Amorphous Body: Can squeeze through tight spaces and is immune to being grappled.",
    "Split: When reduced to low health, splits into two smaller blobs with half health."
};

string player = R"(
        ┌───────┐
        │--o--o-│
        │       │
        │       │
        │       │
        └──├──├─┘
           │  │
)";

string cloud = R"(
      .--.
   .-(    ).
  (___.__)__)
)";

struct Cloud {
    int x;
    int y;
    string art;
    int width;
    int height;

    Cloud(int startX, int startY, string cloudArt) : x(startX), y(startY), art(cloudArt) {
        height = 0;
        width = 0;
        for (char c : art) {
            if (c == '\n') {
                height++;
            }
        }

        size_t pos = 0;
        while (pos < art.length()) {
            size_t end = art.find('\n', pos);
            if (end == string::npos) {
                end = art.length();
            }
            int lineWidth = static_cast<int>(end - pos);
            if (lineWidth > width) {
                width = lineWidth;
            }
            pos = end + 1;
        }
    }

    void draw() const {
        int lineY = y;
        string line;
        for (char c : art) {
            if (c == '\n') {
                mvprintw(lineY++, x, "%s", line.c_str());
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            mvprintw(lineY, x, "%s", line.c_str());
        }
    }

    void clear() const {
        for (int i = 0; i < height; i++) {
            mvprintw(y + i, x, "%s", string(width, ' ').c_str());
        }
    }

    void moveLeft() {
        clear();
        x--;
        draw();
    }
};

class SideScrollPlayer {
private:
    int screenY;
    int screenX;
    string art;
    int width;
    int height;

public:
    SideScrollPlayer(int centerY, int centerX) : screenY(centerY), screenX(centerX), art(player) {
        height = 0;
        width = 0;
        for (char c : art) {
            if (c == '\n') {
                height++;
            }
        }
        size_t pos = 0;
        while (pos < art.length()) {
            size_t end = art.find('\n', pos);
            if (end == string::npos) {
                end = art.length();
            }
            int lineWidth = static_cast<int>(end - pos);
            if (lineWidth > width) {
                width = lineWidth;
            }
            pos = end + 1;
        }
    }

    void draw() const {
        int lineY = screenY;
        string line;
        for (char c : art) {
            if (c == '\n') {
                mvprintw(lineY++, screenX, "%s", line.c_str());
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            mvprintw(lineY, screenX, "%s", line.c_str());
        }
    }
};

class SideScrollGame {
private:
    vector<Cloud> clouds;
    int screenWidth;
    int screenHeight;
    int cloudSpawnCounter;
    int cloudSpawnDelay;
    int score;
    bool running;

public:
    SideScrollGame() : cloudSpawnCounter(0), cloudSpawnDelay(20), score(0), running(true) {
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);

        getmaxyx(stdscr, screenHeight, screenWidth);

        srand(time(0));
        for (int i = 0; i < 3; i++) {
            spawnCloud(screenWidth + (i * 30));
        }
    }

    ~SideScrollGame() {
        endwin();
    }

    void spawnCloud(int x = -1) {
        if (x == -1) {
            x = screenWidth;
        }
        int y = rand() % (screenHeight / 3);
        if (y < 1) {
            y = 1;
        }

        clouds.emplace_back(x, y, cloud);
        cloudSpawnCounter = 0;
    }

    void updateClouds() {
        for (auto &item : clouds) {
            item.moveLeft();
        }

        clouds.erase(
            remove_if(clouds.begin(), clouds.end(), [this](const Cloud &c) { return c.x + c.width < 0; }),
            clouds.end()
        );

        cloudSpawnCounter++;
        if (cloudSpawnCounter >= cloudSpawnDelay) {
            spawnCloud();
            cloudSpawnDelay = 15 + rand() % 15;
        }
    }

    void handleInput() {
        int ch = wgetch(stdscr);
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }
    }

    void drawUI() {
        mvprintw(0, 0, "Score: %d", score);
        mvprintw(1, 0, "Clouds: %zu", clouds.size());
        mvprintw(2, 0, "Press Q to quit");
        mvprintw(3, 0, "<- Clouds move left | Player stays in center ->");

        for (int i = 0; i < screenWidth; i++) {
            mvprintw(screenHeight - 5, i, "=");
        }
    }

    void updateScore() {
        score++;
    }

    void render(SideScrollPlayer &playerObj) {
        clear();

        for (auto &item : clouds) {
            item.draw();
        }

        playerObj.draw();
        drawUI();
        refresh();
    }

    void run() {
        SideScrollPlayer playerObj(screenHeight - 12, screenWidth / 2 - 10);
        while (running) {
            handleInput();
            updateClouds();
            updateScore();
            render(playerObj);
            napms(50);
        }
    }
};

// ===== User interaction UI =====
class user_interaction {
private:
    int selected;
    vector<Monster *> monsters_list;
    int max_y, max_x;
    bool quit_flag;

public:
    user_interaction() : selected(0), quit_flag(false) {
        monsters_list = {&ghost, &mushroom, &owl};
    }

    void display_menu() {
        clear();
        getmaxyx(stdscr, max_y, max_x);

        mvprintw(0, (max_x - 8) / 2, "MONSTERS");
        mvprintw(1, 0, "=================================================================================");

        int y = 3;
        for (size_t i = 0; i < monsters_list.size(); i++) {
            if (i == static_cast<size_t>(selected)) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(y, 0, "> %s", monsters_list[i]->name.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(y, 0, "  %s", monsters_list[i]->name.c_str());
            }

            int pattern_y = y + 1;
            istringstream iss(monsters_list[i]->appearance);
            string line;
            while (getline(iss, line) && pattern_y < max_y - 8) {
                if (i == static_cast<size_t>(selected)) {
                    attron(COLOR_PAIR(1));
                    mvprintw(pattern_y, 2, "%s", line.c_str());
                    attroff(COLOR_PAIR(1));
                } else {
                    mvprintw(pattern_y, 2, "%s", line.c_str());
                }
                pattern_y++;
            }

            y = pattern_y + 2;
        }

        if (static_cast<size_t>(selected) < monsters_list.size()) {
            Monster *m = monsters_list[selected];
            mvprintw(max_y - 6, 0, "Attack:  %s", m->attack.c_str());
            mvprintw(max_y - 5, 0, "Defense: %s", m->defense.c_str());
            mvprintw(max_y - 4, 0, "Special: %s", m->special_ability.c_str());
        }

        mvprintw(max_y - 2, 0, "Use UP/DOWN to select, ENTER to choose, Q to quit");
        refresh();
    }

    void handle_input(int ch) {
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + static_cast<int>(monsters_list.size())) % static_cast<int>(monsters_list.size());
                break;
            case KEY_DOWN:
                selected = (selected + 1) % static_cast<int>(monsters_list.size());
                break;
            case KEY_ENTER:
            case '\n':
                mvprintw(max_y - 1, 0, "You selected: %s", monsters_list[selected]->name.c_str());
                refresh();
                getch();
                break;
            case 'q':
            case 'Q':
                quit_flag = true;
                break;
        }
    }

    void run() {
        while (!quit_flag) {
            display_menu();
            int ch = getch();
            handle_input(ch);
        }
    }
};

int runSideScrollDemo() {
    SideScrollGame game;
    game.run();

    clear();
    mvprintw(10, 20, "Game Over!");
    mvprintw(12, 20, "Press any key to exit...");
    refresh();
    getch();
    return 0;
}

int runMonsterMenuDemo() {
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);

    user_interaction ui;
    ui.run();

    endwin();
    return 0;
}

int getCenteredX(const string &text) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    return max(0, (maxX - static_cast<int>(text.size())) / 2);
}

int getCenteredStartY(int totalLines) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    return max(0, (maxY - totalLines) / 2);
}

void centerPrint(int y, const string &text) {
    mvprintw(y, getCenteredX(text), "%s", text.c_str());
}

bool isWindowLargeEnough() {
    const int requiredRows = 32;
    const int requiredCols = 120;
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    return maxY >= requiredRows && maxX >= requiredCols;
}

void enforceWindowSizeGate() {
    const int requiredRows = 32;
    const int requiredCols = 120;

    timeout(120);
    while (!isWindowLargeEnough()) {
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);

        clear();
        vector<string> lines = {
            "Please maximize the terminal window to continue.",
            "Current size: " + to_string(maxX) + "x" + to_string(maxY),
            "Required size: at least " + to_string(requiredCols) + "x" + to_string(requiredRows),
            "Game is paused until the window is large enough."
        };

        int startY = max(0, (maxY - static_cast<int>(lines.size())) / 2);
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            int startX = max(0, (maxX - static_cast<int>(lines[i].size())) / 2);
            mvprintw(startY + i, startX, "%s", lines[i].c_str());
        }
        refresh();

        int ch = wgetch(stdscr);
        if (ch == KEY_RESIZE || ch == ERR) {
            continue;
        }
    }
    timeout(-1);
}

int readKeyWithWindowGuard() {
    timeout(120);
    while (true) {
        if (!isWindowLargeEnough()) {
            enforceWindowSizeGate();
            clear();
            refresh();
        }

        int ch = wgetch(stdscr);
        if (ch == ERR) {
            continue;
        }

        timeout(-1);
        return ch;
    }
}

bool shouldAdvanceFromWaitKey(int ch) {
    // Accept only Space for a concise, explicit continue action.
    if (ch == ERR || ch == KEY_RESIZE || ch == KEY_MOUSE) return false;
    return ch == ' ';
}

void drawSpaceContinueHint() {
    const string hint = "Press SPACE to continue...";
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int hintY = max(0, maxY - 2);
    move(hintY, 0);
    clrtoeol();
    mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
}

void ncWait() {
    string hint = "Press SPACE to continue...";

    while (true) {
        enforceWindowSizeGate();

        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);

        int hintY = max(0, maxY - 2);
        move(hintY, 0);
        clrtoeol();
        mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
        refresh();

        int ch = readKeyWithWindowGuard();
        if (!shouldAdvanceFromWaitKey(ch)) {
            continue;
        }
        break;
    }
}


void showTitle() {
    // Use a dedicated color pair for the title. If the terminal supports custom colors,
    // define a softer pink; otherwise fall back to magenta.
    if (has_colors()) {
        if (can_change_color() && COLORS > 16) {
            const short kPinkColor = 10;
            init_color(2, 1000, 500, 800); // bright pink
            init_pair(2, kPinkColor, COLOR_BLACK);
        } else {
            init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
        }
    }

    while (true) {
        clear();
        int startY = getCenteredStartY(7);

        if (has_colors()) {
            attron(COLOR_PAIR(2) | A_BOLD);
        }
        centerPrint(startY, title);
        if (has_colors()) {
            attroff(COLOR_PAIR(2) | A_BOLD);
        }

        drawSpaceContinueHint();

        refresh();

        int ch = readKeyWithWindowGuard();
        if (!shouldAdvanceFromWaitKey(ch)) {
            continue;
        }
        break;
    }
}   

void showIntro() {
    vector<string> lines = {
        "===== Knight Maze RPG =====",
        "",
        "You are a brave knight, sent into a dangerous maze to rescue the princess.",
        "",
        "In the maze, you will encounter traps, enemies, merchants, and a Boss.",
        "",
        "You must find the key and rescue the princess!"
    };

    while (true) {
        clear();
        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            centerPrint(startY + i, lines[i]);
        }

        drawSpaceContinueHint();
        refresh();

        int ch = readKeyWithWindowGuard();
        if (!shouldAdvanceFromWaitKey(ch)) {
            continue;
        }
        break;
    }
}

void showHelp() {
    vector<string> lines = {
        "===== HELP =====",
        "",
        "- Movement: Enter w a s d or arrow keys to move.",
        "- Battle: Choose 1 Normal Attack, 2 Strong Attack, 3 Defend.",
        "- Merchant: Buy or rob (risk).",
        "- Chest: May contain gold, potion, or monster.",
        "- Level up: EXP reaches 100 = auto level up.",
        "- Victory: Find the key and rescue the princess."
    };

    while (true) {
        clear();
        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            centerPrint(startY + i, lines[i]);
        }

        drawSpaceContinueHint();
        refresh();

        int ch = readKeyWithWindowGuard();
        if (!shouldAdvanceFromWaitKey(ch)) {
            continue;
        }
        break;
    }
}