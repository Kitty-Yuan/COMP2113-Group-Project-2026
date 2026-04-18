/*
This file contains classes and functions related to the ncurses(visualization) and user interaction in the game.
*/

#include <ncurses.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include "monsters.h"

using namespace std;

class user_interaction {
private:
    int selected;
    vector<Monster*> monsters_list;
    int max_y, max_x;
    bool quit_flag;

public:
    user_interaction() : selected(0), quit_flag(false) {
        monsters_list = {&ghost, &mushroom, &owl};
    }

    void display_menu() {
        clear();
        getmaxyx(stdscr, max_y, max_x);

        // 标题
        mvprintw(0, (max_x - 8) / 2, "MONSTERS");
        mvprintw(1, 0, "=================================================================================");

        int y = 3;
        for (size_t i = 0; i < monsters_list.size(); i++) {
            if (i == (size_t)selected) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(y, 0, "> %s", monsters_list[i]->name.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(y, 0, "  %s", monsters_list[i]->name.c_str());
            }

            // 显示怪兽图案
            int pattern_y = y + 1;
            istringstream iss(monsters_list[i]->appearance);
            string line;
            while (getline(iss, line) && pattern_y < max_y - 8) {
                if (i == (size_t)selected) {
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

        // 攻击/防御/特殊能力信息
        if ((size_t)selected < monsters_list.size()) {
            Monster* m = monsters_list[selected];
            mvprintw(max_y - 6, 0, "Attack:  %s", m->attack.c_str());
            mvprintw(max_y - 5, 0, "Defense: %s", m->defense.c_str());
            mvprintw(max_y - 4, 0, "Special: %s", m->special_ability.c_str());
        }

        // 帮助信息
        mvprintw(max_y - 2, 0, "Use UP/DOWN to select, ENTER to choose, Q to quit");
        refresh();
    }

    void handle_input(int ch) {
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + monsters_list.size()) % monsters_list.size();
                break;
            case KEY_DOWN:
                selected = (selected + 1) % monsters_list.size();
                break;
            case KEY_ENTER:
            case '\n':
                // 选择处理
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

int main() {
    // 初始化 ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    
    // 启用颜色
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    
    // 创建用户交互对象并运行
    user_interaction ui;
    ui.run();
    
    // 结束 ncurses
    endwin();
    return 0;
}

