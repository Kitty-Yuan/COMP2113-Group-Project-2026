#ifndef UI_UX_H
#define UI_UX_H

#include <string>

struct Monster {
    std::string name;
    std::string appearance;
    std::string attack;
    std::string defense;
    std::string special_ability;
};

extern Monster ghost;
extern Monster mushroom;
extern Monster owl;
extern Monster blob;

extern std::string tree;
extern std::string ground;
extern std::string title;

void showTitle();
void show_HP();

int runSideScrollDemo();
int runMonsterMenuDemo();

int getCenteredX(const std::string &text);
int getCenteredStartY(int totalLines);
void centerPrint(int y, const std::string &text);
void ncWait();
void showIntro();
void showHelp();

bool isWindowLargeEnough();
void enforceWindowSizeGate();
int readKeyWithWindowGuard();
void show_ATT(int value, int maxVal, std::string type, int y, int x);

#endif
