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
#include "../user_save_system/user_save_system.h"

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
__  __                   _  __      _       _     _   
|  \/  | __ _ _______   | |/ /_ __ (_) __ _| |__ | |_ 
| |\/| |/ _` |_  / _ \  |   /|  _ \| |/ _` | '_ \| __|
| |  | | (_| |/ /  __/  |   \| | | | | (_| | | | | |_ 
|_|  |_|\__,_/___\___|  |_|\_\_| |_|_|\__, |_| |_|\__|
                                      |___/

                    MAZE NIGHT
)";

void show_ATT(int value, int maxVal, string type, int y, int x) {
    const int BAR_LENGTH = 12;

    int filled_count = (int)((double)value / maxVal * BAR_LENGTH);

    std::string hp_bar;
    for (int i = 0; i < filled_count; i++) {
        hp_bar += "|";
    }

    // Print left border (no color)
    mvprintw(y, x, "|");
    
    // Print bar with color based on type
    if (type == "HP") {
        attron(COLOR_PAIR(3) | A_BOLD); 
    } else if (type == "ATK") {
        attron(COLOR_PAIR(2) | A_BOLD); 
    } else if (type == "DEF") {
        attron(COLOR_PAIR(4) | A_BOLD);  
    }
    mvprintw(y, x + 1, "%s", hp_bar.c_str());
    if (type == "HP") {
        attroff(COLOR_PAIR(3) | A_BOLD);
    } else if (type == "ATK") {
        attroff(COLOR_PAIR(2) | A_BOLD);
    } else if (type == "DEF") {
        attroff(COLOR_PAIR(4) | A_BOLD);
    }

    // Print right border (no color) and attribute text
    mvprintw(y, x + 1 + BAR_LENGTH, "|");
    mvprintw(y, x + 1 + BAR_LENGTH + 2, "%s: %d / %d", 
             type == "HP" ? "HP" : type == "ATK" ? "ATK" : "DEF", value, maxVal);
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
   .--OO--.
 /--_    _--\
(___@____@___)
    |    |
    (____)
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
    mvprintw(12, 20, "Press ENTER to continue...");
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

        showButton();
        refresh();

        int ch = wgetch(stdscr);
        if (ch == ERR) {
            continue;
        }

        timeout(-1);
        return ch;
    }
}

bool shouldAdvanceFromWaitKey(int ch) {
    // Accept only ENTER for a concise, explicit continue action.
    if (ch == ERR || ch == KEY_RESIZE || ch == KEY_MOUSE) return false;
    return ch == '\n' || ch == KEY_ENTER;
}

void drawSpaceContinueHint() {
    const string hint = "Press ENTER to continue...";
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int hintY = max(0, maxY - 2);
    move(hintY, 0);
    clrtoeol();
    mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
}

void ncWait() {
    string hint = "Press ENTER to continue...";

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


bool showTitle() {
    // Use a dedicated color pair for the title. If the terminal supports custom colors,
    // define a softer pink; otherwise fall back to magenta.

    while (true) {
        clear();
        vector<string> titleLines;
        {
            istringstream iss(title);
            string line;
            while (getline(iss, line)) {
                titleLines.push_back(line);
            }
        }

        while (!titleLines.empty() && titleLines.front().empty()) {
            titleLines.erase(titleLines.begin());
        }
        while (!titleLines.empty() && titleLines.back().empty()) {
            titleLines.pop_back();
        }

        int startY = getCenteredStartY(static_cast<int>(titleLines.size()));

        if (has_colors()) {
            attron(COLOR_PAIR(2) | A_BOLD);
        }
        for (int i = 0; i < static_cast<int>(titleLines.size()); i++) {
            centerPrint(startY + i, titleLines[i]);
        }
        if (has_colors()) {
            attroff(COLOR_PAIR(2) | A_BOLD);
        }

        const int buttonWidth = 14;
        const int buttonHeight = 3;
        const int buttonGap = 4;
        const int buttonsTopY = startY + static_cast<int>(titleLines.size()) + 2;
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        const int totalButtonsWidth = buttonWidth * 2 + buttonGap;
        const int buttonsStartX = max(0, (maxX - totalButtonsWidth) / 2);

        const int manualX = buttonsStartX;
        const int quitX = buttonsStartX + buttonWidth + buttonGap;

        mvprintw(buttonsTopY, manualX, "+------------+");
        mvprintw(buttonsTopY + 1, manualX, "|   MANUAL   |");
        mvprintw(buttonsTopY + 2, manualX, "+------------+");

        mvprintw(buttonsTopY, quitX, "+------------+");
        mvprintw(buttonsTopY + 1, quitX, "|    QUIT    |");
        mvprintw(buttonsTopY + 2, quitX, "+------------+");

        drawSpaceContinueHint();

        const string hint = "Click MANUAL / QUIT, or press ENTER to start";
        mvprintw(min(maxY - 4, buttonsTopY + buttonHeight + 1),
                 max(0, (maxX - static_cast<int>(hint.size())) / 2),
                 "%s",
                 hint.c_str());

        refresh();

        int ch = readKeyWithWindowGuard();
        if (ch == KEY_MOUSE) {
            MEVENT event;
            if (getmouse(&event) == OK) {
                mmask_t clickMask = BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED |
                                    BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED;
                if ((event.bstate & clickMask) != 0) {
                    bool hitManual =
                        event.x >= manualX && event.x < manualX + buttonWidth &&
                        event.y >= buttonsTopY && event.y < buttonsTopY + buttonHeight;
                    bool hitQuit =
                        event.x >= quitX && event.x < quitX + buttonWidth &&
                        event.y >= buttonsTopY && event.y < buttonsTopY + buttonHeight;

                    if (hitManual) {
                        showHelp();
                        continue;
                    }
                    if (hitQuit) {
                        return false;
                    }
                }
            }
            continue;
        }

        if (!shouldAdvanceFromWaitKey(ch)) {
            continue;
        }
        return true;
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
    int contentWidth = 0;
    for (const string &line : lines) {
        contentWidth = max(contentWidth, static_cast<int>(line.size()));
    }

    const int popupWidth = max(66, contentWidth + 4);
    const int popupHeight = static_cast<int>(lines.size()) + 6;

    while (true) {
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        int startY = max(0, (maxY - popupHeight) / 2);
        int startX = max(0, (maxX - popupWidth) / 2);

        WINDOW *popup = newwin(popupHeight, popupWidth, startY, startX);
        keypad(popup, TRUE);
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
        werase(popup);
        box(popup, 0, 0);
        mvwprintw(popup, 0, 2, " HELP ");
        mvwprintw(popup, 0, popupWidth - 4, "[X]");

        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            mvwprintw(popup, 2 + i, 2, "%s", lines[i].c_str());
        }

        mvwprintw(popup, popupHeight - 2, 2, "Click [X] or press ESC to close.");
        wrefresh(popup);

        const int closeX1 = startX + popupWidth - 4;
        const int closeX2 = startX + popupWidth - 2;
        const int closeY = startY;

        while (true) {
            int ch = wgetch(popup);
            if (ch == KEY_RESIZE) {
                delwin(popup);
                touchwin(stdscr);
                refresh();
                break;
            }
            if (ch == 27) {
                delwin(popup);
                touchwin(stdscr);
                refresh();
                return;
            }
            if (ch != KEY_MOUSE) {
                continue;
            }

            MEVENT event;
            if (getmouse(&event) != OK) {
                continue;
            }

            mmask_t clickMask = BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED |
                                BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED;
            if ((event.bstate & clickMask) == 0) {
                continue;
            }

            if (event.y >= closeY && event.y <= closeY + 1 && event.x >= closeX1 - 1 && event.x <= closeX2 + 1) {
                delwin(popup);
                touchwin(stdscr);
                refresh();
                return;
            }
        }
    }
}

namespace {
struct ButtonRect {
    int x;
    int y;
    int width;
    int height;
    TopButtonAction action;
};

vector<ButtonRect> getTopButtonRects() {
    const int buttonWidth = 14;
    const int buttonHeight = 3;
    const int buttonGap = 1;

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int startX = max(0, maxX - buttonWidth - 1);
    int startY = 0;

    return {
        {startX, startY + 0 * (buttonHeight + buttonGap), buttonWidth, buttonHeight, TopButtonAction::Home},
        {startX, startY + 1 * (buttonHeight + buttonGap), buttonWidth, buttonHeight, TopButtonAction::Quit},
        {startX, startY + 2 * (buttonHeight + buttonGap), buttonWidth, buttonHeight, TopButtonAction::Help},
    };
}
}  // namespace


void showButton(){
    vector<string> labels = {"HOME", "QUIT", "MANUAL"};

    if (has_colors()) {
        attron(COLOR_PAIR(1) | A_BOLD);
    }

    vector<ButtonRect> rects = getTopButtonRects();
    for (int i = 0; i < static_cast<int>(labels.size()) && i < static_cast<int>(rects.size()); i++) {
        const ButtonRect &rect = rects[i];
        if (rect.y + rect.height > getmaxy(stdscr)) {
            break;
        }

        mvprintw(rect.y, rect.x, "+------------+");
        mvprintw(rect.y + 1, rect.x, "|            |");
        mvprintw(rect.y + 2, rect.x, "+------------+");

        int labelX = rect.x + max(0, (rect.width - static_cast<int>(labels[i].size())) / 2);
        mvprintw(rect.y + 1, labelX, "%s", labels[i].c_str());
    }

    if (has_colors()) {
        attroff(COLOR_PAIR(1) | A_BOLD);
    }
}

TopButtonAction getTopButtonActionFromMouse(const MEVENT &event) {
    for (const ButtonRect &rect : getTopButtonRects()) {
        if (event.x >= rect.x - 1 && event.x < rect.x + rect.width + 1 &&
            event.y >= rect.y - 1 && event.y < rect.y + rect.height + 1) {
            return rect.action;
        }
    }

    return TopButtonAction::None;
}

string promptInputLine(int y,
                       const string &label,
                       bool maskInput,
                       const vector<string> *contextLines,
                       int contextStartY) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    int startX = max(0, (maxX - 50) / 2);

    string input;
    while (true) {
        // Clear and redraw the input line
        move(y, 0);
        clrtoeol();
        
        // Display label + input content
        mvprintw(y, startX, "%s", label.c_str());
        
        // Display input content (masked or plain)
        string displayContent;
        for (char c : input) {
            displayContent += maskInput ? '*' : c;
        }
        mvprintw(y, startX + static_cast<int>(label.size()), "%s", displayContent.c_str());
        
        // Display hint at bottom
        string hint = "Press ENTER to continue...";
        int hintY = max(0, maxY - 2);
        move(hintY, 0);
        clrtoeol();
        mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
        
        // Move cursor after the input
        move(y, startX + static_cast<int>(label.size()) + static_cast<int>(displayContent.size()));
        refresh();
        
        int ch = readKeyWithWindowGuard();
        if (ch == KEY_RESIZE) {
            getmaxyx(stdscr, maxY, maxX);
            startX = max(0, (maxX - 50) / 2);

            if (contextLines != nullptr) {
                clear();
                for (int i = 0; i < static_cast<int>(contextLines->size()); i++) {
                    centerPrint(contextStartY + i, (*contextLines)[i]);
                }
            }
            continue;
        }

        if (ch == '\n' || ch == KEY_ENTER) {
            break;
        }

        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (!input.empty()) {
                input.pop_back();
            }
            continue;
        }

        if (!isprint(ch) || input.size() >= 32) {
            continue;
        }

        input.push_back(static_cast<char>(ch));
    }

    return input;
}

bool authenticateUser(string &username) {
    for (int attempt = 0; attempt < 3; ++attempt) {
        clear();
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        
        vector<string> tips = {
            "===== USER LOGIN =====",
            "Use only letters, digits, _ or - (max 32 chars).",
            "Existing user: enter correct password to restore progress.",
            "New user: account will be created automatically.",
            ""
        };
        int startY = getCenteredStartY(static_cast<int>(tips.size()) + 4);
        for (int i = 0; i < static_cast<int>(tips.size()); i++) {
            centerPrint(startY + i, tips[i]);
        }

        // Input username with validation
        string inputName;
        bool validUsername = false;
        while (!validUsername) {
            inputName = promptInputLine(
                startY + static_cast<int>(tips.size()),
                "Username: ",
                false,
                &tips,
                startY
            );
            
            // Validate username: only letters, digits, _, -
            bool isValid = true;
            for (char c : inputName) {
                if (!isalnum(c) && c != '_' && c != '-') {
                    isValid = false;
                    break;
                }
                // Check if it's a non-ASCII character
                if (static_cast<unsigned char>(c) > 127) {
                    isValid = false;
                    break;
                }
            }
            
            if (!isValid && !inputName.empty()) {
                // Show error and reset
                clear();
                for (int i = 0; i < static_cast<int>(tips.size()); i++) {
                    centerPrint(startY + i, tips[i]);
                }
                centerPrint(startY + static_cast<int>(tips.size()) + 2, "Invalid input! Use only letters, digits, _ or -");
                
                string hint = "Press ENTER to continue...";
                int hintY = max(0, maxY - 2);
                mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
                refresh();
                ncWait();
                continue;
            }
            validUsername = true;
        }

        string inputPassword = promptInputLine(
            startY + static_cast<int>(tips.size()) + 1,
            "Password: ",
            false,
            &tips,
            startY
        );

        user_save_system::AuthResult result = user_save_system::loginOrRegister(inputName, inputPassword);
        clear();
        centerPrint(getCenteredStartY(2), result.message);
        if (result.authenticated) {
            username = inputName;
            centerPrint(getCenteredStartY(2) + 1, "Authentication success.");
            
            string hint = "Press ENTER to continue...";
            int hintY = max(0, maxY - 2);
            mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
            refresh();
            napms(700);
            return true;
        }

        centerPrint(getCenteredStartY(2) + 1, "Press ENTER to continue...");
        string hint = "Press ENTER to continue...";
        int hintY = max(0, maxY - 2);
        mvprintw(hintY, max(0, (maxX - static_cast<int>(hint.size())) / 2), "%s", hint.c_str());
        refresh();
        ncWait();
    }

    return false;
}
