#ifndef UI_UX_H
#define UI_UX_H

#include <string>
#include <vector>
#include <ncurses.h>

/** @brief Custom key code representing the Home button action. */
static const int KET_HOME_BUTTON = -2;

/**
 * @brief Structure representing monster data.
 * Stores name, ASCII frames for animation, combat moves, and lore.
 */
struct Monster {
    std::string name;
    std::string appearance1;
    std::string appearance2;
    std::string specialattack1;
    std::string specialattack2;
    std::string Introduction;
};

// ===== External Global Variables =====
extern Monster ghost;
extern Monster chestnut;
extern Monster owl;
extern Monster blob;

extern std::string tree;
extern std::string ground;
extern std::string title;

/** @brief Enum for top navigation bar actions. */
enum class TopButtonAction {
    None,
    Home,
    Help,
    Quit
};

/** @brief Renders the title screen; returns true if user starts game. */
bool showTitle();

/** @brief Displays the HP bar or related status. */
void show_HP();

/** @brief Runs the side-scrolling animation demo. */
int runSideScrollDemo();

/** @brief Runs the monster selection menu demo. */
int runMonsterMenuDemo();

// ===== Centering & Alignment Helpers =====

/** @brief Calculates the X coordinate to center text in the current window. */
int getCenteredX(const std::string &text);

/** @brief Calculates the starting Y coordinate to center a block of lines vertically. */
int getCenteredStartY(int totalLines);

/** @brief Prints the given text centered at row y. */
void centerPrint(int y, const std::string &text);

// ===== UI Navigation & Feedback =====

/** @brief Pauses execution and waits for user input (Enter). */
void ncWait();

/** @brief Displays a highlighted "Press ENTER to continue" hint at the bottom. */
void showEnterToContinueHint();

/** @brief Shows the game introduction sequence. */
void showIntro();

/** @brief Displays the help/instructions menu. */
void showHelp();

/** @brief Draws the persistent UI buttons at the top of the screen. */
void showButton();

/** @brief Displays the current difficulty level at the top-right corner. */
void displayDifficultyLevel(int difficulty);

/** @brief Global difficulty level variable. */
extern int currentDifficulty;

/** @brief Determines which UI button was clicked based on mouse event. */
TopButtonAction getTopButtonActionFromMouse(const MEVENT &event);

// ===== Window Management =====

/** @brief Checks if the terminal window meets the minimum size requirements. */
bool isWindowLargeEnough();

/** @brief Blocks execution until the window is resized to a valid dimension. */
void enforceWindowSizeGate();

/** @brief Reads a key while verifying window size stability. */
int readKeyWithWindowGuard();

/** @brief Non-blocking read for animation frames; returns ERR if no input. */
int readKeyAnimFrame(int timeoutMs);

/** @brief Renders an attribute bar (e.g., HP/ATK) at specific coordinates. */
void show_ATT(int value, int maxVal, std::string type, int y, int x);

// ===== Player Stats =====

/**
 * @brief Structure for player statistics used in UI panels.
 */
struct PlayerStats {
    int hp, maxHP;
    int atk;
    int def;
    int gold;
    int exp;
    int level;
    bool hasKey;
};

/** @brief Displays the player statistics panel. */
void displayPlayerStats(const PlayerStats &stats);

// ===== User Input and Authentication =====

/**
 * @brief Displays a prompt and captures a line of user input.
 * @param maskInput If true, displays asterisks instead of characters.
 */
std::string promptInputLine(int y,
                            const std::string &label,
                            bool maskInput,
                            const std::vector<std::string> *contextLines = nullptr,
                            int contextStartY = 0);

/** @brief Handles the user authentication (login/registration) process. */
bool authenticateUser(std::string &username);

// ===== Monster Interactions =====

/** @brief Returns a pointer to a random Monster from the global list. */
Monster* getRandomMonster();

/** @brief Renders the monster encounter screen at the given Y position. */
void displayMonsterEncounter(int &y);

// ===== Visual Effects =====

/** @brief Animates a fire effect at the specified coordinates. */
void fireEffect(int startX, int startY, int duration = 1500);

/** @brief Draws a mushroom ASCII at the given coordinates with custom colors. */
void drawMushroom(int startY, int startX, int colorPair, char pattern);

/** @brief Global list containing all available monster types. */
extern std::vector<Monster> monsters;

#endif // UI_UX_H
