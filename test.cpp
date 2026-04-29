#include <iostream>
#include <vector>
#include <string>
#include <ncurses.h>
#include <locale.h>

using namespace std;

void drawPrincess(const vector<string>& asset, int startY, int startX) {
    for (int y = 0; y < (int)asset.size(); ++y) {
        const string& line = asset[y];
        move(startY + y, startX);
        int i = 0;
        while (i < (int)line.length()) {
            if (line[i] == ' ') {
                addch(' ');
                i++;
                continue;
            }
            char colorKey = line[i];
            i++; 
            int colorPair = 1; 
            switch (colorKey) {
                case 'W': colorPair = 1; break; 
                case 'Y': colorPair = 2; break; 
                case 'R': colorPair = 3; break; 
                default:  colorPair = 1; break;
            }
            int charLen = 1;
            unsigned char c = (unsigned char)line[i];
            if (c >= 0xf0) charLen = 4;
            else if (c >= 0xe0) charLen = 3;
            else if (c >= 0xc0) charLen = 2;
            string actualChar = line.substr(i, charLen);
            i += charLen;
            attron(COLOR_PAIR(colorPair));
            printw("%s", actualChar.c_str());
            attroff(COLOR_PAIR(colorPair));
        }
    }
}

int main() {
    setlocale(LC_ALL, ""); 
    initscr();
    start_color();
    noecho();
    curs_set(0);

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    vector<string> princessUI = {
        "      Y⡖Y⠞Y⠳Y⢲",
        "    Y⣠Y⠊Y⠉Y⠉Y⠉Y⠉Y⠑Y⣄",
        "  Y⡖Y⠃Y⢀Y⡼Y⢆Y⡤Y⠧Y⡀Y⠘Y⢲",
        "  Y⢸Y⢻W⣇W⠘      W⠃W⣸Y⡟Y⡇",
        "Y⢤Y⠜Y⠘W⣚W⣒W⡤W⢤W⣒W⣓Y⠃Y⠣Y⡤",
        "Y⢠Y⠃Y⣞Y⡀R⡏R⣬R⣡R⢹Y⢀Y⣳Y⠘Y⡅",
        "Y⠈W⢢W⠋W⣱R⠷R⣈R⣁R⠾W⣎W⠙W⡔⠁",
        "W⡖W⣁W⠞W⠁R⢀R⠜R⠣R⡀W⠈W⠳W⣈W⢲",
        "W⠉W⡽R⠒R⠊R⠁    R⠈R⠑R⠒W⢫W⠉",
        "R⣰R⠁            R⠈R⢇",
        "R⢀R⠇              R⠸R⡀",
        "R⠘R⠢R⠤R⠤R⠤R⢄R⣀R⣀R⡀R⠤R⠤R⠤R⠔R⠃"
    };

    clear();
    drawPrincess(princessUI, (LINES - 12) / 2, (COLS - 40) / 2);
    mvprintw(LINES - 1, 0, "Press any key to exit...");
    refresh();
    getch();
    endwin();
    return 0;
}