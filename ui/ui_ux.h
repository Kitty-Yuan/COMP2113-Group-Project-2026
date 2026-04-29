#ifndef UI_UX_H
#define UI_UX_H

#include <string>
#include <ncurses.h>

static const int KET_HOME_BUTTON = -2;

struct Monster {
    std::string name;
    std::string appearance1;
    std::string appearance2;
    std::string specialattack1;
    std::string specialattack2;
    std::string Introduction;
};

extern Monster ghost;
extern Monster chestnut;
extern Monster owl;
extern Monster blob;

extern std::string tree;
extern std::string ground;
extern std::string title;

enum class TopButtonAction {
    None,
    Home,
    Help,
    Quit
};

bool showTitle();
void show_HP();

int runSideScrollDemo();
int runMonsterMenuDemo();

int getCenteredX(const std::string &text);
int getCenteredStartY(int totalLines);
void centerPrint(int y, const std::string &text);
void ncWait();
void showIntro();
void showHelp();
void showButton();
TopButtonAction getTopButtonActionFromMouse(const MEVENT &event);

bool isWindowLargeEnough();
void enforceWindowSizeGate();
int readKeyWithWindowGuard();
void show_ATT(int value, int maxVal, std::string type, int y, int x);

// Player stats display
struct PlayerStats {
    int hp, maxHP;
    int atk;
    int def;
    int gold;
    int exp;
    int level;
    bool hasKey;
};
void displayPlayerStats(const PlayerStats &stats);

// User input and authentication functions
std::string promptInputLine(int y,
                            const std::string &label,
                            bool maskInput,
                            const std::vector<std::string> *contextLines = nullptr,
                            int contextStartY = 0);
bool authenticateUser(std::string &username);

// Monster encounter function
Monster* getRandomMonster();
void displayMonsterEncounter(int &y);

// Fire effect animation
void fireEffect(int startX, int startY, int duration = 1500);

extern std::vector<Monster> monsters;

#endif
