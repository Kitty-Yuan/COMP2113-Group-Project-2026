#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <limits>
#include <cctype>
#include <ncurses.h>
#include <sstream>
#include "user_save_system/user_save_system.h"
#include "ui/ui_ux.h"
using namespace std;

int SIZE; 
char grid[50][50]; 
bool visited[50][50];
bool discovered[50][50];
int currentDifficulty = 0; // 1=Easy, 2=Normal, 3=Hard, 4=Hell

/**
 * @struct Player
 * @brief Main structure holding player statistics and state.
 */
struct Player {
    int hp = 100;      /**< Health points */
    int atk = 8;       /**< Attack power */
    int def = 5;       /**< Defense power */
    int gold = 10;     /**< Currency balance */
    int exp = 0;       /**< Experience points */
    int level = 1;     /**< Character level */
    bool hasKey = false; /**< Progression flag for winning the game */
};

user_save_system::SaveData buildSaveData(const Player &p, int monsterMin, int monsterMax, int bossMin, int bossMax);
bool applySaveData(const user_save_system::SaveData &data, Player &p, int &monsterMin, int &monsterMax, int &bossMin, int &bossMax);
void initializeNewMap();
void chooseDifficulty(const string &username, int &monsterMin, int &monsterMax, int &bossMin, int &bossMax);
void tutorial(Player &p);

int px = 0, py = 0;
int gx, gy;
int cursorY = 0; // Global cursor for display
int cursorX = 0;
bool returnToDifficultyMenu = false;

/**
 * @brief Generates the save slot key for specific difficulty levels.
 * @param username Current player's name
 * @param difficulty Numeric difficulty level
 * @return Formatted string for file naming
 */
string buildDifficultySaveSlot(const string &username, int difficulty) {
    return username + "__difficulty_" + to_string(difficulty);
}

/**
 * @brief Generates the identifier for a cleared difficulty status.
 * @param username Current player's name
 * @param difficulty Numeric difficulty level
 * @return Formatted string for persistent storage check
 */
string buildClearedSlot(const string &username, int difficulty) {
    return username + "__cleared_" + to_string(difficulty);
}

/**
 * @brief Checks the save system to see if a specific difficulty has been beaten.
 * @param username User to check
 * @param difficulty Level to check
 * @return true if the level was previously cleared
 */
bool isDifficultyCleared(const string &username, int difficulty) {
    string slot = buildClearedSlot(username, difficulty);
    if (!user_save_system::hasSave(slot)) return false;
    user_save_system::SaveData data;
    return user_save_system::loadProgress(slot, data) && data.cleared;
}

/**
 * @brief Manages the session lifecycle (Difficulty -> Load/New -> Tutorial -> Save).
 * @param username Active user
 * @param[out] activeSaveSlot The slot ID currently in use
 * @param[out] p Player object to initialize/restore
 * @param allowTutorial Whether to show the tutorial prompt
 * @param chooseDiffAgain Whether to trigger the difficulty selection UI
 * @return true if the session was restored from an existing save
 */
bool startDifficultySession(const string &username,
                            string &activeSaveSlot,
                            Player &p,
                            int &monsterMin,
                            int &monsterMax,
                            int &bossMin,
                            int &bossMax,
                            bool allowTutorial,
                            bool chooseDiffAgain = true) {
    if (chooseDiffAgain) {
        chooseDifficulty(username, monsterMin, monsterMax, bossMin, bossMax);
    }
    activeSaveSlot = buildDifficultySaveSlot(username, currentDifficulty);

    bool loadedFromSave = false;
    user_save_system::SaveData loadedData;
    
    // Try to load existing save (if exists and not completed)
    if (user_save_system::hasSave(activeSaveSlot) &&
        user_save_system::loadProgress(activeSaveSlot, loadedData) &&
        !loadedData.cleared) {
        loadedFromSave = applySaveData(loadedData, p, monsterMin, monsterMax, bossMin, bossMax);
    }

    clear();
    if (loadedFromSave) {
        centerPrint(getCenteredStartY(1), "Save found for this difficulty. Progress restored.");
        refresh();
        napms(800);
    } else {
        p = Player();
        px = 0;
        py = 0;

        centerPrint(getCenteredStartY(1), "No save found for this difficulty. Starting a new game.");
        refresh();
        ncWait();

        initializeNewMap();
    }

    // Show tutorial prompt if allowTutorial is true (regardless of save state)
    if (allowTutorial) {
        clear();
        centerPrint(getCenteredStartY(1), "Press T for Tutorial, any other key to skip");
        refresh();

        int ch = readKeyWithWindowGuard();
        if (ch == 'T' || ch == 't') {
            Player temp = p;
            tutorial(temp);
        }
    }

    returnToDifficultyMenu = false;
    user_save_system::saveProgress(activeSaveSlot, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
    return loadedFromSave;
}

/**
 * @brief Maps various ncurses key codes to standard 'wasd' chars.
 * @param key Raw input from getch()
 * @return Normalized direction char or null char if invalid
 */
char normalizeMoveKey(int key) {
    if (key == 'w' || key == 'W' || key == KEY_UP) return 'w';
    if (key == 's' || key == 'S' || key == KEY_DOWN) return 's';
    if (key == 'a' || key == 'A' || key == KEY_LEFT) return 'a';
    if (key == 'd' || key == 'D' || key == KEY_RIGHT) return 'd';
    return '\0';
}

/**
 * @brief Exit choices after game over.
 */
enum class PostDeathAction {
    Home, /**< Return to main menu */
    Quit  /**< Terminate application */
};

/**
 * @brief Helper to filter ncurses mouse events for primary clicks.
 */
bool isPrimaryMouseClick(const MEVENT &event) {
    mmask_t clickMask = BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED |
                        BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED;
    return (event.bstate & clickMask) != 0;
}

/**
 * @brief Displays the Game Over screen and handles navigation input.
 * @return The user's selected PostDeathAction
 */
PostDeathAction promptPostDeathAction() {
    while (true) {
        clear();
        vector<string> lines = {
            "You died. Game Over.",
            "",
            "Click HOME to return to difficulty selection.",
            "Click QUIT to quit the program."
        };
        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            centerPrint(startY + i, lines[i]);
        }
        showButton();
        refresh();

        int ch = readKeyWithWindowGuard();
        if (ch == KET_HOME_BUTTON) {
            return PostDeathAction::Home;
        }
        if (ch != KEY_MOUSE) {
            continue;
        }

        MEVENT event;
        if (getmouse(&event) != OK || !isPrimaryMouseClick(event)) {
            continue;
        }

        TopButtonAction action = getTopButtonActionFromMouse(event);
        if (action == TopButtonAction::Home) {
            return PostDeathAction::Home;
        }
        if (action == TopButtonAction::Quit) {
            return PostDeathAction::Quit;
        }
    }
}

/**
 * @brief Packages current game state variables into a serializable struct.
 * @return A SaveData object containing player, map, and monster information
 */
user_save_system::SaveData buildSaveData(const Player &p, int monsterMin, int monsterMax, int bossMin, int bossMax) {
    user_save_system::SaveData data;
    data.valid = true;
    data.size = SIZE;
    data.px = px;
    data.py = py;
    data.gx = gx;
    data.gy = gy;
    data.monsterMin = monsterMin;
    data.monsterMax = monsterMax;
    data.bossMin = bossMin;
    data.bossMax = bossMax;

    data.hp = p.hp;
    data.atk = p.atk;
    data.def = p.def;
    data.gold = p.gold;
    data.exp = p.exp;
    data.level = p.level;
    data.hasKey = p.hasKey;

    data.gridRows.assign(SIZE, string(SIZE, '.'));
    data.discoveredRows.assign(SIZE, string(SIZE, '0'));
    data.visitedRows.assign(SIZE, string(SIZE, '0'));

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data.gridRows[i][j] = grid[i][j];
            data.discoveredRows[i][j] = discovered[i][j] ? '1' : '0';
            data.visitedRows[i][j] = visited[i][j] ? '1' : '0';
        }
    }

    return data;
}

/**
 * @brief Validates and unpacks save data into the active game environment.
 * @param data Source save data
 * @param[out] p Target player struct
 * @return true if data was valid and applied, false if data was corrupted
 */
bool applySaveData(const user_save_system::SaveData &data, Player &p, int &monsterMin, int &monsterMax, int &bossMin, int &bossMax) {
    if (!data.valid || data.size <= 0 || data.size > 50) {
        return false;
    }

    SIZE = data.size;
    px = data.px;
    py = data.py;
    gx = data.gx;
    gy = data.gy;

    monsterMin = data.monsterMin;
    monsterMax = data.monsterMax;
    bossMin = data.bossMin;
    bossMax = data.bossMax;

    p.hp = data.hp;
    p.atk = data.atk;
    p.def = data.def;
    p.gold = data.gold;
    p.exp = data.exp;
    p.level = data.level;
    p.hasKey = data.hasKey;

    if (static_cast<int>(data.gridRows.size()) != SIZE ||
        static_cast<int>(data.discoveredRows.size()) != SIZE ||
        static_cast<int>(data.visitedRows.size()) != SIZE) {
        return false;
    }

    for (int i = 0; i < SIZE; i++) {
        if (static_cast<int>(data.gridRows[i].size()) != SIZE ||
            static_cast<int>(data.discoveredRows[i].size()) != SIZE ||
            static_cast<int>(data.visitedRows[i].size()) != SIZE) {
            return false;
        }
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = data.gridRows[i][j];
            discovered[i][j] = (data.discoveredRows[i][j] == '1');
            visited[i][j] = (data.visitedRows[i][j] == '1');
        }
    }

    if (px < 0 || px >= SIZE || py < 0 || py >= SIZE || gx < 0 || gx >= SIZE || gy < 0 || gy >= SIZE) {
        return false;
    }

    discovered[px][py] = true;
    visited[px][py] = true;
    
    // Determine and set difficulty level based on SIZE and monster stats
    if (SIZE == 9) currentDifficulty = 1;      // Easy
    else if (SIZE == 12) currentDifficulty = 2; // Normal
    else if (SIZE == 15) currentDifficulty = 3; // Hard
    else if (SIZE == 20) currentDifficulty = 4; // Hell
    
    return true;
}
/**
 * @brief Checks if a traversable path exists between two points using Breadth-First Search (BFS).
 * @param sx,sy Starting coordinates.
 * @param ex,ey Target coordinates.
 * @return true if a path consisting only of floor tiles ('.') exists.
 */
bool hasPath(int sx, int sy, int ex, int ey) {
    if (sx == ex && sy == ey) return true;
    if (grid[sx][sy] == '#' || grid[ex][ey] == '#') return false;
    
    bool temp_visited[50][50];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            temp_visited[i][j] = false;
        }
    }
    
    queue<pair<int,int>> q;
    q.push({sx, sy});
    temp_visited[sx][sy] = true;
    
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();
        
        if (x == ex && y == ey) return true;
        
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && 
                !temp_visited[nx][ny] && grid[nx][ny] != '#') {
                temp_visited[nx][ny] = true;
                q.push({nx, ny});
            }
        }
    }
    return false;
}

/**
 * @brief Guarantees a path between two points by converting walls ('#') to floors ('.').
 * @details Performs a BFS to find the shortest potential path ignoring walls, 
 * then backtracks from the target to the start to carve the path.
 * @param sx,sy Starting coordinates.
 * @param ex,ey Target coordinates.
 */
void createPath(int sx, int sy, int ex, int ey) {
    // Helper function to create a path between two points
    if (hasPath(sx, sy, ex, ey)) return;
    
    bool temp_visited[50][50];
    int parent_x[50][50], parent_y[50][50];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            temp_visited[i][j] = false;
            parent_x[i][j] = -1;
            parent_y[i][j] = -1;
        }
    }
    
    queue<pair<int,int>> q;
    q.push({sx, sy});
    temp_visited[sx][sy] = true;
    
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    bool found = false;
    while (!q.empty() && !found) {
        auto [x, y] = q.front();
        q.pop();
        
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && !temp_visited[nx][ny]) {
                temp_visited[nx][ny] = true;
                parent_x[nx][ny] = x;
                parent_y[nx][ny] = y;
                q.push({nx, ny});
                
                if (nx == ex && ny == ey) {
                    found = true;
                    break;
                }
            }
        }
    }
    
    if (found) {
        int x = ex, y = ey;
        while (x != sx || y != sy) {
            if (grid[x][y] == '#') {
                grid[x][y] = '.';
            }
            int px = parent_x[x][y];
            int py = parent_y[x][y];
            x = px;
            y = py;
        }
    }
}

/**
 * @brief Initializes the game world with randomized terrain and essential landmarks.
 * @details Uses probability-based wall generation and ensures all key locations 
 * (Key, Goal, Shop, etc.) are reachable from the player's start.
 */
void initializeNewMap() {
    // Generate random map with walls (25% walls)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = (rand() % 100 < 25 ? '#' : '.');
            visited[i][j] = false;
            discovered[i][j] = false;
        }
    }

    // Set player start and exit (goal)
    px = 0;
    py = 0;
    gx = SIZE - 1;
    gy = SIZE - 1;
    grid[px][py] = '.';
    grid[gx][gy] = 'G';

    // ---- Boss placement: either above or left of exit ----
    bool bossAbove = (rand() % 2 == 0);  // true = above, false = left
    int bossX, bossY;
    if (bossAbove) {
        bossX = gx - 1;
        bossY = gy;
        // The left neighbor becomes a wall
        if (gy - 1 >= 0) {
            grid[gx][gy - 1] = '#';
        }
    } else {
        bossX = gx;
        bossY = gy - 1;
        // The above neighbor becomes a wall
        if (gx - 1 >= 0) {
            grid[gx - 1][gy] = '#';
        }
    }
    // Ensure boss position is valid (should be within bounds)
    if (bossX >= 0 && bossY >= 0 && bossX < SIZE && bossY < SIZE) {
        grid[bossX][bossY] = 'B';
    } else {
        // Fallback: if out of bounds (shouldn't happen for SIZE>=2), place at center
        grid[SIZE/2][SIZE/2] = 'B';
    }

    // Place collectible items: Key (K), Trap (T), Chest (C)
    int kx, ky, tx, ty, cx, cy;
    auto isForbidden = [&](int x, int y) -> bool {
        return (x == px && y == py) ||
               (x == gx && y == gy) ||
               (x == bossX && y == bossY) ||
               grid[x][y] == '#';
    };

    do {
        kx = rand() % SIZE;
        ky = rand() % SIZE;
    } while (isForbidden(kx, ky));
    grid[kx][ky] = 'K';

    do {
        tx = rand() % SIZE;
        ty = rand() % SIZE;
    } while (isForbidden(tx, ty) || (tx == kx && ty == ky));
    grid[tx][ty] = 'T';

    do {
        cx = rand() % SIZE;
        cy = rand() % SIZE;
    } while (isForbidden(cx, cy) || (cx == kx && cy == ky) || (cx == tx && cy == ty));
    grid[cx][cy] = 'C';

    // Ensure paths exist between critical locations
    createPath(px, py, kx, ky);      // P -> K
    createPath(kx, ky, gx, gy);      // K -> G
    createPath(px, py, tx, ty);      // P -> T
    createPath(px, py, cx, cy);      // P -> C
    createPath(px, py, bossX, bossY); // P -> B (boss)

    // Mark start as discovered and visited
    discovered[px][py] = true;
    visited[px][py] = true;
}
/**
 * @brief Triggers the final endgame mini-game: The Princess Chase.
 * @details The player must reach the Goal (Princess) within a step limit. 
 * The step limit is dynamically calculated based on the player's remaining HP.
 * @param p Reference to the Player struct (HP is reduced to 0 on failure).
 * @param isTrial If true, uses a fixed step count for tutorial purposes.
 */
// ===== Catch princess minigame (normal mode) =====
void princessRoomMinigame(Player &p, bool isTrial) {
    const int roomSize = 15;
    char room[roomSize][roomSize];
    srand(time(0));

    int maxSteps = isTrial ? 15 : (12 + (p.hp + 9) / 10); 

    for (int i = 0; i < roomSize; i++) {
        for (int j = 0; j < roomSize; j++) {
            if (i == 0 || i == roomSize - 1 || j == 0 || j == roomSize - 1) room[i][j] = '#';
            else {
                bool isWall = (rand() % 100 < 30);
                if (i == roomSize/2 || j == roomSize/2 || i == j || i == (roomSize-1-j)) isWall = false;
                room[i][j] = (isWall ? '#' : '.');
            }
        }
    }

    int rpx = 1, rpy = 1; 
    int rgx = roomSize/2, rgy = roomSize/2;
    int stepsUsed = 0;

    while (true) {
        clear();
        int mapWidth = roomSize * 2;
        int blockHeight = roomSize + 4;
        int startY = getCenteredStartY(blockHeight);
        [[maybe_unused]] int maxY;
        int maxX;
        getmaxyx(stdscr, maxY, maxX);
        int startX = max(0, (maxX - mapWidth) / 2);
        string title = "PRINCESS CHASE - Steps Left: " + to_string(maxSteps - stepsUsed);
        centerPrint(startY, title);
        
        for (int i = 0; i < roomSize; i++) {
            string row;
            for (int j = 0; j < roomSize; j++) {
                if (i == rpx && j == rpy) row += 'P';
                else if (i == rgx && j == rgy) row += 'G';
                else row += room[i][j];
                row += ' ';
            }
            mvprintw(startY + 2 + i, startX, "%s", row.c_str());
        }

        // Victory check before input (if already overlapped from previous turn)
        if (rpx == rgx && rpy == rgy) {
            centerPrint(startY + roomSize + 3, "VICTORY! You caught the princess!");
            refresh();
            ncWait();
            return;
        }

        if (stepsUsed >= maxSteps) {
            centerPrint(startY + roomSize + 3, "FAILED! The princess ran away...");
            p.hp = 0;
            refresh();
            ncWait();
            return;
        }

        // Display movement prompt
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(startY + roomSize + 3, startX, "Move (W/A/S/D or Arrow Keys):");
        attroff(COLOR_PAIR(1) | A_BOLD);
        refresh();

        // ----- Input handling (only valid movement keys) -----
        int key = readKeyWithWindowGuard();
        char m = normalizeMoveKey(key);

        // Invalid key (non-movement)
        if (m == '\0') {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(startY + roomSize + 5, startX, "Invalid key! Use W/A/S/D or Arrow keys only.");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            napms(800);
            continue;
        }

        int nx = rpx, ny = rpy;
        if (m == 'w') nx--;
        else if (m == 's') nx++;
        else if (m == 'a') ny--;
        else if (m == 'd') ny++;

        // Wall collision
        if (nx <= 0 || nx >= roomSize-1 || ny <= 0 || ny >= roomSize-1 || room[nx][ny] == '#') {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(startY + roomSize + 5, startX, "Blocked by wall!");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            napms(800);
            continue;
        }

        // Valid move
        rpx = nx;
        rpy = ny;
        stepsUsed++;

        // Check victory after player move
        if (rpx == rgx && rpy == rgy) {
            clear();
            centerPrint(getCenteredStartY(2), "VICTORY! You caught the princess!");
            refresh();
            ncWait();
            return;
        }

        // ----- Princess movement (every 2 steps) -----
        if (stepsUsed > 0 && stepsUsed % 2 == 0) {
            int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
            int dir = rand() % 4;
            int ngx = rgx + dx[dir], ngy = rgy + dy[dir];
            if (ngx > 0 && ngx < roomSize-1 && ngy > 0 && ngy < roomSize-1 && room[ngx][ngy] == '.') {
                rgx = ngx;
                rgy = ngy;
            }

            // Check victory after princess move (foolish princess)
            if (rpx == rgx && rpy == rgy) {
                clear();
                centerPrint(getCenteredStartY(2), "Princess walked right into you!");
                centerPrint(getCenteredStartY(3), "VICTORY! You rescued the princess!");
                refresh();
                ncWait();
                return;
            }
        }
    }
}
/**
 * @brief Handles difficulty selection UI and sets game parameters.
 * @details Displays a menu with difficulty levels (Easy to Hell). Already cleared 
 * levels are highlighted. Based on choice, it sets map SIZE and monster stats.
 * @param username Current user (used to check cleared status).
 * @param[out] monsterMin, monsterMax Min/Max attack range for regular monsters.
 * @param[out] bossMin, bossMax Min/Max attack range for the final boss.
 */
void chooseDifficulty(const string &username, int &monsterMin, int &monsterMax, int &bossMin, int &bossMax) {
    int diff = 0;

    // Pre-compute cleared flags (avoid repeated disk reads inside loop)
    bool isCleared[5] = {};
    for (int d = 1; d <= 4; d++) {
        isCleared[d] = isDifficultyCleared(username, d);
    }

    while (diff < 1 || diff > 4) {
        clear();

        auto badge = [&](int d) -> string {
            return isCleared[d] ? " [CLEARED]" : "";
        };

        vector<string> lines = {
            "Choose difficulty (1-4):",
            "1. Easy  " + badge(1) + "  (9x9 map,   ATK 5-10,  Boss 10-15)",
            "2. Normal" + badge(2) + "  (12x12 map, ATK 8-12,  Boss 12-18)",
            "3. Hard  " + badge(3) + "  (15x15 map, ATK 10-15, Boss 15-22)",
            "4. Hell  " + badge(4) + "  (20x20 map, ATK 12-18, Boss 18-25)",
            "Enter choice: 1 / 2 / 3 / 4"
        };

        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            if (i >= 1 && i <= 4) {
                int d = i;
                if (isCleared[d]) {
                    attron(COLOR_PAIR(4) | A_BOLD);
                    centerPrint(startY + i, lines[i]);
                    attroff(COLOR_PAIR(4) | A_BOLD);
                } else {
                    centerPrint(startY + i, lines[i]);
                }
            } else {
                centerPrint(startY + i, lines[i]);
            }
        }
        refresh();
        
        int ch = readKeyWithWindowGuard();
        if (ch >= '1' && ch <= '4') {
            diff = ch - '0';
        }

    }
    
    currentDifficulty = diff;
    
    if (diff == 1) { SIZE = 9; monsterMin=5; monsterMax=10; bossMin=10; bossMax=15; }
    else if (diff == 2) { SIZE = 12; monsterMin=8; monsterMax=12; bossMin=12; bossMax=18; }
    else if (diff == 3) { SIZE = 15; monsterMin=10; monsterMax=15; bossMin=15; bossMax=22; }
    else { SIZE = 20; monsterMin=12; monsterMax=18; bossMin=18; bossMax=25; }

    gx = SIZE - 1; gy = SIZE - 1;
    clear();
    string result = "You chose Lv" + to_string(diff) + "! Map size " + to_string(SIZE) + "x" + to_string(SIZE);
    centerPrint(getCenteredStartY(1), result);
    refresh();
    ncWait();
}

/**
 * @brief Displays the narrative introduction screen.
 * @details Shows a splash screen with basic control instructions and event 
 * hints to set the mood for the adventure.
 */
void introScreen() {
    clear();

    int y = getCenteredStartY(10);

    centerPrint(y++, "==================================");
    centerPrint(y++, "       YOUR ADVENTURE BEGINS      ");
    centerPrint(y++, "==================================");
    y++;

    centerPrint(y++, "Follow what you learned:");
    centerPrint(y++, "- Move with WASD");
    centerPrint(y++, "- Fight enemies");
    centerPrint(y++, "- Find the key");
    y++;

    centerPrint(y++, "Random events may occur:");
    centerPrint(y++, "- Shop");
    centerPrint(y++, "- Mysterious stranger");
    centerPrint(y++, "- Unexpected encounters");
    y++;

    attron(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
    centerPrint(y++, "Press ENTER to start your journey...");
    attroff(COLOR_PAIR(1) | A_BOLD | A_REVERSE);

    refresh();

    while (true) {
        int ch = readKeyWithWindowGuard();
        if (ch == '\n' || ch == KEY_ENTER) break;
    }
}

/**
 * @brief Processes player experience and handles level-up stat increases.
 * @details Checks if EXP >= 100, then increments level and boosts HP, ATK, and DEF.
 * Displays a level-up notification to the player.
 * @param p Reference to the Player object to be updated.
 */
void levelUp(Player &p) {
    while (p.exp >= 100) {
        p.exp -= 100;
        p.level++;
        p.hp = min(p.hp + 20, 100);
        p.atk += 5;
        p.def += 3;
        clear();
        string levelUpMsg = "Level Up! Lv " + to_string(p.level) + " HP+20 ATK+5 DEF+3";
        centerPrint(getCenteredStartY(1), levelUpMsg);
        refresh();
        ncWait();
    }
}

/**
 * @brief A simplified version of the chase mini-game for the tutorial.
 * @details Introduces the player to the movement-based chase mechanic on a 
 * fixed 10x10 grid where the enemy moves every two player steps.
 * @param p Reference to the Player (used for consistency, stats not modified here).
 */
void tutorialMinigame([[maybe_unused]] Player &p) {
    const int N = 10;
    char grid[N][N];

    // ===== initialize the map =====
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == 0 || j == 0 || i == N-1 || j == N-1)
                grid[i][j] = '#';
            else
                grid[i][j] = '.';
        }
    }

    int px = 1, py = 1;
    int ex = N/2, ey = N/2;
    int steps = 0;

    while (true) {
        clear();

        int startY = getCenteredStartY(N + 5);

        centerPrint(startY++, "Catch the monster!");
        centerPrint(startY++, "It moves every 2 steps...");
        centerPrint(startY++, "---------------------");

        // ===== draw the map =====
        for (int i = 0; i < N; i++) {
            string row;
            for (int j = 0; j < N; j++) {
                if (i == px && j == py) row += 'P';
                else if (i == ex && j == ey) row += 'E';
                else row += grid[i][j];
                row += ' ';
            }
            centerPrint(startY++, row);
        }

        centerPrint(startY++, "Move: W/A/S/D");
        refresh();

        // ===== input =====
        int key = readKeyWithWindowGuard();
        char m = normalizeMoveKey(key);

        // Invalid key (non-movement)
        if (m == '\0') {
            attron(COLOR_PAIR(3) | A_BOLD);
            centerPrint(startY, "Invalid key! Use W/A/S/D or Arrow keys only.");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            napms(800);
            continue;
        }

        int nx = px, ny = py;
        if (m == 'w') nx--;
        else if (m == 's') nx++;
        else if (m == 'a') ny--;
        else if (m == 'd') ny++;

        // Wall collision
        if (grid[nx][ny] == '#') {
            attron(COLOR_PAIR(3) | A_BOLD);
            centerPrint(startY, "Blocked by wall!");
            attroff(COLOR_PAIR(3) | A_BOLD);
            refresh();
            napms(800);
            continue;
        }

        // Valid move
        px = nx;
        py = ny;
        steps++;

        // ===== catch the enemy after player move =====
        if (px == ex && py == ey) {
            clear();
            centerPrint(getCenteredStartY(2), "You caught the monster!");
            centerPrint(getCenteredStartY(2)+1, "Victory!");
            refresh();
            ncWait();
            return;
        }

        // ===== enemy movement =====
        if (steps % 2 == 0) {
            int dx[] = {-1, 1, 0, 0};
            int dy[] = {0, 0, -1, 1};

            int tries = 0;
            while (tries < 10) {
                int dir = rand() % 4;
                int nx_e = ex + dx[dir];
                int ny_e = ey + dy[dir];

                if (grid[nx_e][ny_e] != '#') {
                    ex = nx_e;
                    ey = ny_e;
                    break;
                }
                tries++;
            }

            // ===== catch the enemy after enemy move =====
            if (px == ex && py == ey) {
                clear();
                centerPrint(getCenteredStartY(2), "Foolish monster walked right into you!");
                centerPrint(getCenteredStartY(2)+1, "You caught the monster! Victory!");
                refresh();
                ncWait();
                return;
            }
        }
    }
}

/**
 * @brief Runs an interactive tutorial session for new players.
 * @details This function creates a mini 5x5 sandbox where players learn:
 * 1. Movement using WASD.
 * 2. Combat mechanics (Normal/Strong/Defend) through a scripted monster encounter.
 * 3. Objective progression (picking up the 'K'ey to unlock the 'G'oal).
 * 4. The "Catch the Princess" mini-game concept.
 * @param p Reference to the Player struct (tutorial stats carry over to the first game).
 */
void tutorial(Player &p) {
    clear();
    centerPrint(getCenteredStartY(1), "===== TUTORIAL =====");
    refresh();
    ncWait();

    // Removed static 'P' from starting cell (player drawn dynamically)
    char demoMap[5][5] = {
        {'.','.','#','.','K'},
        {'.','#','.','.','.'},
        {'.','.','B','#','.'},
        {'#','.','#','.','.'},
        {'.','.','#','.','G'}
    };

    bool hasKey = false;
    int x = 0, y_pos = 0;
    int step = 0;
    int guidePhase = 1;       // 1=force option1, 2=force option2, 3=force option3, 4=free
    bool explained[4] = {false, false, false, false};

    while (true) {
        clear();

        int y = 0;
        [[maybe_unused]] int maxY;
        int maxX;
        getmaxyx(stdscr, maxY, maxX);

        int startY = getCenteredStartY(12);
        int startX = max(0, (maxX - 10) / 2);

        // ======================
        // STEP HINT SYSTEM
        // ======================
        if (step == 0)
            centerPrint(startY + y++, "Step 1: Move with W/A/S/D");
        else if (step == 1)
            centerPrint(startY + y++, "Step 2: Go fight enemy (B)");
        else if (step == 2)
            centerPrint(startY + y++, "Step 3: Go get the key (K)");
        else if (step == 3)
            centerPrint(startY + y++, "Step 4: Reach the exit (G)");
        centerPrint(startY + y++, "--------------------------");

        // ======================
        // MAP
        // ======================
        for (int i = 0; i < 5; i++) {
            string row;
            for (int j = 0; j < 5; j++) {
                if (i == x && j == y_pos) row += 'P';
                else row += demoMap[i][j];
                row += ' ';
            }
            mvprintw(startY + y, startX, "%s", row.c_str());
            y++;
        }

        // ======================
        // STATS
        // ======================
        string stats = "HP=" + to_string(p.hp) +
                       " ATK=" + to_string(p.atk) +
                       " DEF=" + to_string(p.def) +
                       " GOLD=" + to_string(p.gold) +
                       " EXP=" + to_string(p.exp) +
                       " LV=" + to_string(p.level) +
                       " KEY=" + (hasKey ? "Y" : "N");

        centerPrint(startY + y++, stats);
        centerPrint(startY + y++, "Move: W/A/S/D | Battle: 1/2/3");
        refresh();

        // ======================
        // INPUT
        // ======================
        int key = readKeyWithWindowGuard();
        char m = normalizeMoveKey(key);
        if (m == '\0') continue;

        int nx = x, ny = y_pos;
        if (m == 'w') nx--;
        else if (m == 's') nx++;
        else if (m == 'a') ny--;
        else if (m == 'd') ny++;

        if (nx < 0 || nx >= 5 || ny < 0 || ny >= 5 || demoMap[nx][ny] == '#')
            continue;

        x = nx;
        y_pos = ny;

        if (step == 0) step = 1;

        // ======================
        // KEY
        // ======================
        if (demoMap[x][y_pos] == 'K') {
            clear();
            centerPrint(getCenteredStartY(1), "You found the Key!");
            ncWait();

            hasKey = true;
            demoMap[x][y_pos] = '.';
            if (step <= 2) step = 3;
            napms(300);
        }

        // ======================
        // BATTLE
        // ======================
        else if (demoMap[x][y_pos] == 'B') {
            demoMap[x][y_pos] = '.';

            clear();
            centerPrint(getCenteredStartY(1), "monster encountered!");
            refresh();
            napms(500);

            int monsterHP = 20;
            int monsterMin = 5, monsterMax = 7;
            bool showInvalidMsg = false;

            // Reset guide state for this battle (fresh tutorial each time)
            guidePhase = 1;
            explained[1] = explained[2] = explained[3] = false;

            while (monsterHP > 0 && p.hp > 0) {
                clear();
                int yDisplay = getCenteredStartY(2);
                centerPrint(yDisplay++, "BATTLE - Your HP: " + to_string(p.hp) + " | Monster HP: " + to_string(monsterHP));

                // Show guidance hint if in training mode
                if (guidePhase <= 3) {
                    attron(COLOR_PAIR(2) | A_BOLD);
                    string guideMsg = "[TRAINING] Please press " + to_string(guidePhase);
                    if (guidePhase == 1) guideMsg += " for Normal Attack";
                    else if (guidePhase == 2) guideMsg += " for Strong Attack";
                    else guideMsg += " for Defend";
                    centerPrint(yDisplay++, guideMsg);
                    attroff(COLOR_PAIR(2) | A_BOLD);
                } else {
                    centerPrint(yDisplay++, "Choose freely: 1) Normal  2) Strong  3) Defend");
                }

                if (showInvalidMsg) {
                    attron(COLOR_PAIR(3) | A_BOLD);
                    if (guidePhase <= 3)
                        centerPrint(yDisplay++, "Please press " + to_string(guidePhase) + "!");
                    else
                        centerPrint(yDisplay++, "Invalid! Press 1, 2, or 3.");
                    attroff(COLOR_PAIR(3) | A_BOLD);
                }
                refresh();

                int choice = readKeyWithWindowGuard();
                int playerAttack = 0;
                int defendSuccess = 0;

                // Check if choice matches required guide phase (if in training)
                if (guidePhase <= 3) {
                    if (choice == ('0' + guidePhase)) {
                        showInvalidMsg = true;
                        continue;
                    }
                } else {
                    if (choice != '1' && choice != '2' && choice != '3') {
                        showInvalidMsg = true;
                        continue;
                    }
                }

                showInvalidMsg = false;

                // Execute chosen action
                if (choice == '1') {
                    playerAttack = rand() % p.atk + 1;
                    if (guidePhase == 1 && !explained[1]) {
                        clear();
                        centerPrint(getCenteredStartY(2), "Normal Attack: deals 1 to ATK damage.");
                        centerPrint(getCenteredStartY(2)+1, "No additional cost. Press any key to continue.");
                        refresh();
                        ncWait();
                        explained[1] = true;
                    }
                }
                else if (choice == '2') {
                    if (p.hp <= 3) {
                        clear();
                        centerPrint(getCenteredStartY(1), "Not enough HP! (need at least 3 HP)");
                        refresh();
                        ncWait();
                        continue;
                    }
                    clear();
                    centerPrint(getCenteredStartY(1), "This will cost 3 HP. Continue? (Y/N)");
                    refresh();
                    int confirm = readKeyWithWindowGuard();
                    if (confirm != 'Y' && confirm != 'y') {
                        continue;
                    }
                    p.hp -= 3;
                    if (rand() % 100 < 75) {
                        double mult = 1.3 + (rand() % 40) / 100.0;
                        playerAttack = (int)(p.atk * mult);
                        if (playerAttack < 1) playerAttack = 1;
                    } else {
                        playerAttack = 0;
                    }
                    if (guidePhase == 2 && !explained[2]) {
                        clear();
                        centerPrint(getCenteredStartY(2), "Strong Attack: consumes 3 HP.");
                        centerPrint(getCenteredStartY(2)+1, "75% chance to deal 1.3~1.69x ATK damage, otherwise misses.");
                        centerPrint(getCenteredStartY(2)+2, "Press any key to continue.");
                        refresh();
                        ncWait();
                        explained[2] = true;
                    }
                }
                else if (choice == '3') {
                    defendSuccess = (rand() % 100 < 40) ? 1 : 0;
                    if (guidePhase == 3 && !explained[3]) {
                        clear();
                        centerPrint(getCenteredStartY(2), "Defend: 40% chance to fully block and counterattack,");
                        centerPrint(getCenteredStartY(2)+1, "otherwise reduces damage by 60%.");
                        centerPrint(getCenteredStartY(2)+2, "Press any key to continue.");
                        refresh();
                        ncWait();
                        explained[3] = true;
                    }
                }

                // Apply player damage (non-defend)
                if (choice != '3') {
                    monsterHP -= playerAttack;
                    clear();
                    yDisplay = getCenteredStartY(4);
                    if (playerAttack > 0) {
                        centerPrint(yDisplay++, "You dealt " + to_string(playerAttack) + " damage!");
                    } else if (choice == '1' || choice == '2') {
                        centerPrint(yDisplay++, "Attack missed!");
                    }
                    refresh();
                    ncWait();
                }

                // Check monster death after player attack
                if (monsterHP <= 0) {
                    clear();
                    centerPrint(getCenteredStartY(2), "Monster defeated!");
                    centerPrint(getCenteredStartY(2)+1, "+10 Gold, +20 EXP");
                    p.gold += 10;
                    p.exp += 20;
                    refresh();
                    ncWait();
                    if (step < 2) step = 2;   // Advance to "find key" step
                    break;
                }

                // Monster attack calculation
                int dmg = rand() % (monsterMax - monsterMin + 1) + monsterMin;

                // Defense handling (clear screen and show detailed result)
                if (choice == '3') {
                    clear();
                    yDisplay = getCenteredStartY(2);
                    centerPrint(yDisplay++, "BATTLE - Your HP: " + to_string(p.hp) + " | Monster HP: " + to_string(monsterHP));

                    if (defendSuccess) {
                        int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                        monsterHP -= counterDmg;
                        p.hp = min(p.hp + 5, 100);
                        dmg = 0;
                        attron(COLOR_PAIR(4) | A_BOLD);
                        centerPrint(yDisplay++, "Defend success! Counter attack: " + to_string(counterDmg) + " damage!");
                        attroff(COLOR_PAIR(4) | A_BOLD);
                        if (monsterHP > 0) {
                            centerPrint(yDisplay++, "Monster dealt 0 damage!");
                        }
                        refresh();
                        ncWait();

                        if (monsterHP <= 0) {
                            clear();
                            centerPrint(getCenteredStartY(2), "Monster defeated!");
                            centerPrint(getCenteredStartY(2)+1, "+10 Gold, +20 EXP");
                            p.gold += 10;
                            p.exp += 20;
                            refresh();
                            ncWait();
                            if (step < 2) step = 2;
                            break;
                        }
                    } else {
                        dmg = (int)(dmg * 0.4);
                        attron(COLOR_PAIR(3) | A_BOLD);
                        centerPrint(yDisplay++, "Defense failed! Damage reduced to " + to_string(dmg));
                        attroff(COLOR_PAIR(3) | A_BOLD);
                        refresh();
                        ncWait();
                    }
                }

                if (dmg < 1) dmg = 1;
                p.hp -= dmg;

                // Show monster damage for non-defense choices
                if (choice != '3') {
                    clear();
                    yDisplay = getCenteredStartY(4);
                    centerPrint(yDisplay++, "Monster dealt " + to_string(dmg) + " damage!");
                    refresh();
                    ncWait();
                }
                // For defense branch, damage already shown

                // Advance guide phase after a valid move (only once per phase)
                if (guidePhase <= 3) {
                    guidePhase++;
                    if (guidePhase == 4) {
                        clear();
                        centerPrint(getCenteredStartY(2), "Training complete! Now you can fight freely.");
                        centerPrint(getCenteredStartY(2)+1, "Use 1, 2, or 3 anytime.");
                        refresh();
                        ncWait();
                    }
                }
            }
            demoMap[x][y_pos] = '.';
        }

        // ======================
        // EXIT
        // ======================
        else if (demoMap[x][y_pos] == 'G') {
            clear();

            if (step < 3 || !hasKey) {
                centerPrint(getCenteredStartY(1),
                    "Finish the tutorial steps first!");
                ncWait();
                continue;
            }

            centerPrint(getCenteredStartY(1),
                "You found the monster...");
            refresh();
            napms(800);

            tutorialMinigame(p);
            clear();   // Clear minigame leftovers
            centerPrint(getCenteredStartY(1), "Tutorial Complete!");
            centerPrint(getCenteredStartY(1)+1, "He discovered That you entered his fortress...");
            centerPrint(getCenteredStartY(1)+2, "So he carried the princess on his back and began to run away...");
            refresh();
            ncWait();

            break;
        }
    }
}
// Stage 1: Display ability trigger and effect description
void displaySpecialAbilityEffect(const Monster &m, const string &abilityMsg, 
                                  const vector<string> &effectLines, const Player &p) {
    clear();
    
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    // Display player stats panel (left side)
    PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
    displayPlayerStats(statsPanel);
    
    // Display battle menu buttons (right side)
    showButton();
    
    // Draw a decorative border
    attron(COLOR_PAIR(2) | A_BOLD);
    for (int x = 8; x < maxX - 8; x++) {
        mvaddch(8, x, '=');
        mvaddch(maxY - 8, x, '=');
    }
    for (int y = 8; y < maxY - 8; y++) {
        mvaddch(y, 8, '|');
        mvaddch(y, maxX - 9, '|');
    }
    mvaddch(8, 8, '+');
    mvaddch(8, maxX - 9, '+');
    mvaddch(maxY - 8, 8, '+');
    mvaddch(maxY - 8, maxX - 9, '+');
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    // Display monster name inside border
    int monsterNameY = 10;
    int monsterNameX = (maxX - static_cast<int>(m.name.length())) / 2;
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(monsterNameY, monsterNameX, "%s", m.name.c_str());
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    // Display monster appearance inside border
    int appearanceStartX = (maxX - 30) / 2;
    istringstream iss(m.appearance1);
    string line;
    int lineY = monsterNameY + 2;
    while (getline(iss, line) && lineY < maxY - 10) {
        if (lineY >= 10) {
            mvprintw(lineY++, appearanceStartX, "%s", line.c_str());
        }
    }
    
    // Display ability message inside border
    int msgStartY = maxY - 13;
    if (msgStartY < 15) msgStartY = 15;
    attron(COLOR_PAIR(3) | A_BOLD);
    int msgX = (maxX - static_cast<int>(abilityMsg.length())) / 2;
    mvprintw(msgStartY, msgX, "%s", abilityMsg.c_str());
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // Display effect descriptions inside border
    int effectY = msgStartY + 2;
    attron(COLOR_PAIR(1));
    for (const auto &effectLine : effectLines) {
        if (effectY < maxY - 2) {
            int effectX = (maxX - static_cast<int>(effectLine.length())) / 2;
            mvprintw(effectY++, effectX, "%s", effectLine.c_str());
        }
    }
    attroff(COLOR_PAIR(1));
    
    refresh();
    ncWait();
}

/**
 * @brief Stage 2 of special ability animation: Displays the damage and resource changes.
 * @details Within the same bordered frame as Stage 1, this shows the numeric outcomes 
 * of the interaction, including damage dealt by both parties and any stolen resources.
 * @param m The Monster involved.
 * @param playerDamage Amount of damage dealt to the monster.
 * @param monsterDamage Amount of damage dealt to the player.
 * @param playerAttackMissed Boolean flag to show "MISSED" status.
 * @param p Current player (to update stats panel).
 * @param goldStolen Specific to Blob-type enemies, indicates gold reduction.
 */
// Stage 2: Display attack result within the same bordered frame
void displayAbilityDamageResult(const Monster &m, int playerDamage, int monsterDamage, 
                                 bool playerAttackMissed, const Player &p, int goldStolen = 0) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    clear();
    
    // Display player stats panel (left side)
    PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
    displayPlayerStats(statsPanel);
    
    // Display battle menu buttons (right side)
    showButton();
    
    // Draw a decorative border
    attron(COLOR_PAIR(2) | A_BOLD);
    for (int x = 8; x < maxX - 8; x++) {
        mvaddch(8, x, '=');
        mvaddch(maxY - 8, x, '=');
    }
    for (int y = 8; y < maxY - 8; y++) {
        mvaddch(y, 8, '|');
        mvaddch(y, maxX - 9, '|');
    }
    mvaddch(8, 8, '+');
    mvaddch(8, maxX - 9, '+');
    mvaddch(maxY - 8, 8, '+');
    mvaddch(maxY - 8, maxX - 9, '+');
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    // Display title inside border
    int titleY = 10;
    string titleText = "=== ATTACK RESULT ===";
    int titleX = (maxX - static_cast<int>(titleText.length())) / 2;
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(titleY, titleX, "%s", titleText.c_str());
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // Display player's attack result inside border
    int resultY = titleY + 3;
    attron(COLOR_PAIR(1));
    string playerResult;
    if (playerAttackMissed) {
        playerResult = "Your attack MISSED!";
    } else {
        playerResult = "You dealt " + to_string(playerDamage) + " damage!";
    }
    int playerX = (maxX - static_cast<int>(playerResult.length())) / 2;
    mvprintw(resultY++, playerX, "%s", playerResult.c_str());
    
    // Display gold stolen if applicable
    if (goldStolen > 0) {
        resultY++;
        string goldResult = "Blob stole " + to_string(goldStolen) + " gold!";
        int goldX = (maxX - static_cast<int>(goldResult.length())) / 2;
        mvprintw(resultY++, goldX, "%s", goldResult.c_str());
    }
    
    // Display monster's attack result inside border
    resultY += 2;
    string monsterResult = m.name + " dealt " + to_string(monsterDamage) + " damage!";
    int monsterX = (maxX - static_cast<int>(monsterResult.length())) / 2;
    mvprintw(resultY++, monsterX, "%s", monsterResult.c_str());
    attroff(COLOR_PAIR(1));
    
    refresh();
    ncWait();
}

/**
 * @brief Renders a specific frame of a monster's idle/action animation.
 * @details This is the core animation engine for battle encounters. It:
 * 1. Selects ASCII art frames based on the monster's name and current frame count.
 * 2. Applies unique color palettes (e.g., flickering white for Ghost, pulsing red for Spores).
 * 3. Handles spatial offsets (like the Ghost's floating movement) and state-based 
 * visuals (like the Owl's blinking or Blob's squishing).
 * @param m The Monster object to animate.
 * @param frame The current animation frame index (used for timing/oscillations).
 * @param baseX, baseY The screen coordinates for the top-left of the animation area.
 * @param maxY The screen boundary to prevent drawing outside ncurses window.
 */
// ===== Per-Monster Animation Frames =====
void drawMonsterAnimFrame(const Monster &m, int frame, int baseX, int baseY, int maxY) {
    // Frame strings (no leading newline)
    static const string ghost_face =
        " .------.\n"
        "/  #   # \\\n"
        "|        |\n"
        "~` ~` ~ `~\n";
    static const string ghost_mist =
        "           \n"
        "~` ~` ~ `~\n"
        "~` ~` ~ `~\n"
        "~` ~` ~ `~\n";
    static const string chestnut_normal =
        "   .--OO--.\n"
        " /--_    _--\\\n"
        "(___@____@___)\n"
        "    |    |\n"
        "    (____)\n";
    static const string chestnut_spore =
        "   .--**--.\n"
        " /--_    _--\\\n"
        "(___@____@___)\n"
        "    |    |\n"
        "    (____)\n";
    static const string owl_normal =
        "  /\\ /\\\n"
        "((@ v @))\n"
        "() ::: ()\n"
        "  VV VV\n";
    static const string owl_blink =
        "  /\\ /\\\n"
        "((- v -))\n"
        "() ::: ()\n"
        "  VV VV\n";
    static const string owl_wings =
        "  /\\ /\\\n"
        "((@ v @))\n"
        "() ::: ()\n"
        "  /\\ /\\\n";
    static const string blob_normal =
        "    .----.\n"
        "   ( @  @ )\n"
        "   (      )\n"
        "   `------`\n";
    static const string blob_squish =
        "  .--------.\n"
        " (@        @)\n"
        "  `--------`\n"
        "            \n";
    static const string blob_stretch =
        "    .----.\n"
        "   ( @  @ )\n"
        "   |      |\n"
        "   (      )\n"
        "   `------`\n";

    string frameStr;
    int xOff = 0;

    if (m.name == "Ghost") {
        xOff = (frame % 8 < 4) ? -1 : 1;
        frameStr = ((frame / 15) % 2 == 0) ? ghost_face : ghost_mist;
    } else if (m.name == "chestnut") {
        frameStr = ((frame / 12) % 3 == 2) ? chestnut_spore : chestnut_normal;
    } else if (m.name == "Owl") {
        bool blink = (frame % 14 >= 12);
        bool wingsUp = ((frame / 8) % 2 == 1);
        frameStr = blink ? owl_blink : (wingsUp ? owl_wings : owl_normal);
    } else if (m.name == "Blob") {
        int phase = (frame / 6) % 3;
        frameStr = (phase == 0) ? blob_normal : (phase == 1) ? blob_squish : blob_stretch;
    } else {
        frameStr = m.appearance1;
        if (!frameStr.empty() && frameStr[0] == '\n')
            frameStr = frameStr.substr(1);
    }

    istringstream iss(frameStr);
    string ln;
    int lineY = baseY + 1;

    // Per-monster color + attribute selection
    int colorPair = 1;
    int attrs = A_BOLD;
    if (m.name == "Ghost") {
        // Alternate between white dim and white bold for a flickering ghost effect
        colorPair = 1;
        attrs = ((frame / 8) % 2 == 0) ? (A_BOLD | A_DIM) : A_BOLD;
    } else if (m.name == "chestnut") {
        // Yellow normally; red during spore frame
        bool isSporeFr = ((frame / 12) % 3 == 2);
        colorPair = isSporeFr ? 3 : 2;
        attrs = A_BOLD;
    } else if (m.name == "Owl") {
        // Yellow; dim when blinking
        bool blink = (frame % 14 >= 12);
        colorPair = 2;
        attrs = blink ? A_DIM : A_BOLD;
    } else if (m.name == "Blob") {
        // Green without reverse-video to avoid background block on squish frame
        colorPair = 4;
        attrs = A_BOLD;
    } else {
        colorPair = 2;
        attrs = A_BOLD;
    }

    attron(COLOR_PAIR(colorPair) | attrs);
    while (getline(iss, ln) && lineY < maxY - 5) {
        if (lineY >= 0)
            mvprintw(lineY, baseX + xOff, "%s", ln.c_str());
        lineY++;
    }
    attroff(COLOR_PAIR(colorPair) | attrs);
}

/**
 * @brief Renders the animation frames for the Boss encounter.
 * @details Specifically designed for the final boss, this function cycles through 
 * three distinct animation phases (Idle, Charge, and Slash) to create a dynamic battle atmosphere. 
 * It features:
 * 1. State-based ASCII switching using a frame-based phase logic.
 * 2. Visual feedback through rhythmic flickering.
 * 3. Dedicated color styling
 * @param frame The current animation cycle index.
 * @param baseX The horizontal starting X-coordinate position.
 * @param baseY The vertical starting Y-coordinate position.
 * @param maxY The bottom boundary limit for the ncurses window.
 */
// ===== Boss Animation Frames =====
void drawBossAnimFrame(int frame, int baseX, int baseY, int maxY) {
    static const string boss_idle =
        "       /\\\\\\\n"
        "      / 0 0 \\\n"
        "  .--|   ^   |--.\n"
        " /   |  ---  |   \\\n"
        "|    | [===] |    |\n"
        "|    |  | |  |    |\n"
        " \\   |  | |  |   /\n"
        "  '._|__|_|__|_.'\n"
        "     / /   \\ \\\n"
        "    /_/     \\_\\\n";

    static const string boss_charge =
        "     \\  /\\  /\n"
        "      \\/  \\/ \n"
        "  .--| 0  0 |--.\n"
        " /   |  __  |   \\\n"
        "|    | [##] |    |\n"
        "|    |  ||  |    |\n"
        " \\   |  ||  |   /\n"
        "  '._|__||__|_.'\n"
        "    _/ /  \\ \\\n"
        "   /__/    \\__\\\n";

    static const string boss_slash =
        "      __/\\\\__\n"
        "     / 0 0  /\n"
        " .--|   ^  /--.\n"
        "/   |  -- /    \\\n"
        "|   | [==/=====>\n"
        "|   |  | /|     |\n"
        " \\  |  |/ |    /\n"
        "  '._|__|__|_.'\n"
        "     / /  \\ \\\n"
        "    /_/    \\_\\\n";

    string frameStr;
    int phase = (frame / 8) % 3;
    if (phase == 0) frameStr = boss_idle;
    else if (phase == 1) frameStr = boss_charge;
    else frameStr = boss_slash;

    int colorPair = 3;
    int attrs = ((frame / 4) % 2 == 0) ? A_BOLD : (A_BOLD | A_DIM);

    attron(COLOR_PAIR(colorPair) | attrs);
    istringstream iss(frameStr);
    string ln;
    int lineY = baseY;
    while (getline(iss, ln) && lineY < maxY - 4) {
        if (lineY >= 0) {
            mvprintw(lineY, baseX, "%s", ln.c_str());
        }
        lineY++;
    }
    attroff(COLOR_PAIR(colorPair) | attrs);
}

/**
 * @brief Manages a standard combat encounter.
 * @details Handles the turn-based loop including player input (Normal, Strong, Defend),
 * real-time monster animations, and special ability triggers for specific monster types.
 * @param p Reference to the player state.
 * @param monsterMin Minimum possible monster attack damage.
 * @param monsterMax Maximum possible monster attack damage.
 */
// ===== Battle System =====
void fight(Player &p, int monsterMin, int monsterMax) {
    clear();
    displayDifficultyLevel(currentDifficulty);
    
    // Display player stats panel
    PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
    displayPlayerStats(statsPanel);
    
    int monsterIndex = rand() % monsters.size();
    // ===== DYNAMIC MEMORY ALLOCATION =====
    // Create a dynamic copy of the selected monster for battle state tracking
    Monster* battleMonster = new Monster(monsters[monsterIndex]);
    Monster& m = *battleMonster;  // Reference for easier use
    
    // Animation frame for monster effects
    int animationFrame = 0;
    
    int y = getCenteredStartY(3);
    centerPrint(y++, "You encountered: " + m.name + "!");
    centerPrint(y++, "Monster attack range: " + to_string(monsterMin) + " - " + to_string(monsterMax));
    refresh();
    napms(1000);
    
    int monsterHP = 30 + rand() % 20;

    while (monsterHP > 0 && p.hp > 0) {
        clear();
        displayDifficultyLevel(currentDifficulty);
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        int monsterX = (maxX - 20) / 2;
        int monsterStartY = max(5, (maxY - 12) / 2);
        string line;
        PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};

        int choice = 0;
        bool valid = false;
        bool showInvalidMsg = false;

        while (!valid) {
            clear();
            displayDifficultyLevel(currentDifficulty);
            displayPlayerStats(statsPanel);
            drawMonsterAnimFrame(m, animationFrame, monsterX, monsterStartY, maxY);

            y = monsterStartY - 3;
            centerPrint(y++, "BATTLE - " + m.name + " HP: " + to_string(monsterHP));
            if (showInvalidMsg) {
                attron(COLOR_PAIR(3) | A_BOLD);
                centerPrint(y++, "Invalid! Press 1) Normal  2) Strong  3) Defend");
                attroff(COLOR_PAIR(3) | A_BOLD);
            } else {
                centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
            }
            // Bottom hint (consistent style with rest of game)
            {
                int maxY2, maxX2;
                getmaxyx(stdscr, maxY2, maxX2);
                const string bottomHint = "Press 1 / 2 / 3 to choose...";
                int hintY = max(0, maxY2 - 2);
                move(hintY, 0); clrtoeol();
                attron(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
                mvprintw(hintY, max(0, (maxX2 - static_cast<int>(bottomHint.size())) / 2), "%s", bottomHint.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
            }
            showButton();
            refresh();

            choice = readKeyAnimFrame(150);
            animationFrame++;

            if (choice == ERR) continue;

            if (choice == KET_HOME_BUTTON) {
                returnToDifficultyMenu = true;
                return;
            }

            if (choice == '1' || choice == '2' || choice == '3') {
                valid = true;
                showInvalidMsg = false;
            } else {
                showInvalidMsg = true;
            }
        }

        // Calculate player attack
        int playerAttack = 0;
        int defendSuccess = 0;
        
        if (choice == '1') playerAttack = rand() % p.atk + 1;
        else if (choice == '2') {
            if (p.hp <= 3) {
                clear();
                // Display player stats and monster status (consistent with original battle UI)
                PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                // Display monster appearance etc. (omitted, keep consistent with original code)
                centerPrint(getCenteredStartY(3), "Not enough HP! (need at least 3 HP)");
                refresh();
                ncWait();
                continue; // Return to battle menu
            }
            // Ask for confirmation
            clear();
            displayPlayerStats(statsPanel);
            // Redraw monster UI (omitted, keep original style)
            centerPrint(getCenteredStartY(2), "This will cost 3 HP. Continue? (Y/N)");
            refresh();
            int confirm = readKeyWithWindowGuard();
            if (confirm == KET_HOME_BUTTON) {
                returnToDifficultyMenu = true;
                return;
            }
            if (confirm != 'Y' && confirm != 'y') {
                continue; // Cancel, return to battle menu
            }
            // Deduct HP
            p.hp -= 3;
            // Attack determination
            if (rand() % 100 < 75) {
                double mult = 1.3 + (rand() % 40) / 100.0; // 1.30 ~ 1.69
                playerAttack = (int)(p.atk * mult);
                if (playerAttack < 1) playerAttack = 1;
            } else {
                playerAttack = 0;
            }
        }        
        else if (choice == '3') defendSuccess = (rand() % 100 < 40) ? 1 : 0;

        bool abilityTriggeredThisTurn = false;
        int triggeredAbilityIndex = 0;
        string triggeredAbilityName;
        vector<string> abilityEffectLines;
        int monsterAbilityDamageBonus = 0;  // Monster's damage bonus from ability
        bool playerAttackMissed = false;
        int goldStolen = 0;  // Track gold stolen by Blob

        // New rule: after player picks an attack (1/2), there is a 40% chance
        // to trigger special ability 1 or 2 for this turn.
        if ((choice == '1' || choice == '2') && rand() % 100 < 40) {
            abilityTriggeredThisTurn = true;
            triggeredAbilityIndex = (rand() % 2) + 1;
            triggeredAbilityName = (triggeredAbilityIndex == 1) ? m.specialattack1 : m.specialattack2;

            string abilityMsg = m.name + " USES: " + triggeredAbilityName;

            // ===== GHOST Abilities =====
            if (m.name == "Ghost") {
                if (triggeredAbilityIndex == 1) {
                    // Ability 1: Incorporeal Form - reduces player attack by 50% THIS TURN ONLY
                    playerAttack = (int)(playerAttack * 0.5);
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.35);
                    abilityEffectLines.push_back("Incorporeal Form - becomes intangible and evades attacks");
                } else {
                    // Ability 2: Spectral Drain - reduces player attack by 30% THIS TURN ONLY
                    playerAttack = (int)(playerAttack * 0.7);
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.25);
                    abilityEffectLines.push_back("Spectral Drain - drains your vital energy");
                }
            }
            // ===== CHESTNUT Abilities =====
            else if (m.name == "chestnut") {
                if (triggeredAbilityIndex == 1) {
                    // Ability 1: Spore Burst - reduces player attack by 50% THIS TURN ONLY
                    playerAttack = (int)(playerAttack * 0.5);
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.30);
                    abilityEffectLines.push_back("Spore Burst - releases toxic spores into the air");
                } else {
                    // Ability 2: Toxic Cloud - 70% chance player attack misses
                    if (rand() % 100 < 70) {
                        playerAttack = 0;
                        playerAttackMissed = true;
                        abilityEffectLines.push_back("Toxic Cloud - a poisonous fog covers the field");
                        abilityEffectLines.push_back("Your attack will MISS!");
                    } else {
                        abilityEffectLines.push_back("Toxic Cloud - a poisonous fog covers the field");
                        abilityEffectLines.push_back("You managed to break through the fog!");
                    }
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.20);
                }
            }
            // ===== OWL Abilities =====
            else if (m.name == "Owl") {
                if (triggeredAbilityIndex == 1) {
                    // Ability 1: Fire Blow - fixed +10 monster damage THIS TURN
                    monsterAbilityDamageBonus = 10;
                    abilityEffectLines.push_back("Fire Blow - unleashes a blazing wave of flame");
                } else {
                    // Ability 2: Flash Blindness - 30% chance player attack misses
                    if (rand() % 100 < 30) {
                        playerAttack = 0;
                        playerAttackMissed = true;
                        abilityEffectLines.push_back("Flash Blindness - a blinding flash fills the air");
                        abilityEffectLines.push_back("Your attack will MISS!");
                    } else {
                        abilityEffectLines.push_back("Flash Blindness - a blinding flash fills the air");
                        abilityEffectLines.push_back("You manage to shield your eyes and attack!");
                    }
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.25);
                }
            }
            // ===== BLOB Abilities =====
            else if (m.name == "Blob") {
                if (triggeredAbilityIndex == 1) {
                    // Ability 1: Consume Gold - steal 15 gold from player THIS TURN
                    goldStolen = min(15, p.gold);
                    p.gold = max(0, p.gold - goldStolen);
                    abilityEffectLines.push_back("Consume Gold - absorbs your precious coins");
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.20);
                } else {
                    // Ability 2: Gelatinous Body - 100% chance player attack misses
                    playerAttack = 0;
                    playerAttackMissed = true;
                    abilityEffectLines.push_back("Gelatinous Body - becomes a slippery gelatinous mass");
                    monsterAbilityDamageBonus = (int)(((rand() % (monsterMax - monsterMin + 1)) + monsterMin) * 0.15);
                }
            }
            
            // Display ability effect (STAGE 1: Skill description)
            displaySpecialAbilityEffect(m, abilityMsg, abilityEffectLines, p);
        }

        monsterHP -= playerAttack;

        clear();
        
        // Display player stats panel
        statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);
        
        // Display monster appearance in center with animation
        drawMonsterAnimFrame(m, animationFrame, monsterX, monsterStartY, maxY);

        // Display battle results above monster (only if ability was NOT triggered)
        y = monsterStartY - 3;
        if (!abilityTriggeredThisTurn) {
            if (playerAttack > 0) centerPrint(y++, "You dealt " + to_string(playerAttack) + " damage!");
            else if (choice == '1' || choice == '2') centerPrint(y++, "Attack missed!");
        }

        if (monsterHP <= 0) {
            centerPrint(y++, "monster dealt 0 damage!");
            centerPrint(y++, "monster defeated! +20 Gold, +50 EXP");
            refresh();
            ncWait();

            p.gold += 20;
            p.exp += 50;
            levelUp(p);
            break;
        }

        int dmg = (rand() % (monsterMax - monsterMin + 1)) + monsterMin;
        if (abilityTriggeredThisTurn) {
            dmg += monsterAbilityDamageBonus;
        }
        
        if (choice == '3') {
            if (defendSuccess) {
                int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                monsterHP -= counterDmg;
                p.hp = min(p.hp + 5, 100);
                dmg = 0;
                centerPrint(y++, "Defend successful! Counter attack: " + to_string(counterDmg) + " damage!");
                if (monsterHP <= 0) {
                    centerPrint(y++, "monster dealt 0 damage!");
                    centerPrint(y++, "monster defeated! +20 Gold, +50 EXP");
                    refresh();
                    ncWait();

                    p.gold += 20;
                    p.exp += 50;
                    levelUp(p);
                    break;
                }
            } else {
                dmg = (int)(dmg * 0.4);
            }
        }
        if (dmg < 1) dmg = 1;
        p.hp -= dmg;
        
        // Display damage result (only if ability was NOT triggered - otherwise it was already shown in bordered frame)
        if (!abilityTriggeredThisTurn) {
            centerPrint(y++, m.name + " dealt " + to_string(dmg) + " damage!");
            refresh();
            ncWait();
        } else {
            // Ability was triggered: show full result in bordered frame instead
            displayAbilityDamageResult(m, playerAttack, dmg, playerAttackMissed, p, goldStolen);
        }
    }
    
    // ===== DYNAMIC MEMORY CLEANUP =====
    // Delete the dynamically allocated monster copy
    delete battleMonster;
}

/**
 * @brief Manages the final boss encounter.
 * @details Similar to standard combat but utilizes unique boss animation phases 
 * and increased HP. Higher rewards in gold and experience are granted upon victory.
 * @param p Reference to the player state.
 * @param bossMin Minimum boss attack damage.
 * @param bossMax Maximum boss attack damage.
 */
// ===== Boss Fight =====
void bossFight(Player &p, int bossMin, int bossMax) {
    int y = getCenteredStartY(4);
    clear();
    centerPrint(y++, "Boss battle begins!");
    centerPrint(y++, "Boss attack range: " + to_string(bossMin) + " - " + to_string(bossMax));
    refresh();
    napms(1000);
    
    int bossHP = 100;
    int animationFrame = 0;

    while (bossHP > 0 && p.hp > 0) {
        clear();
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        int bossX = (maxX - 24) / 2;
        int bossStartY = max(5, (maxY - 16) / 2);
        
        // Display player stats panel
        PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        int choice = 0;
        bool valid = false;

        while (!valid) {
            clear();
            displayPlayerStats(statsPanel);
            drawBossAnimFrame(animationFrame, bossX, bossStartY, maxY);

            y = bossStartY - 3;
            centerPrint(y++, "BOSS BATTLE - DREAD KING HP: " + to_string(bossHP));
            centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");

            // Bottom hint
            {
                const string bottomHint = "Press 1 / 2 / 3 to choose...";
                int hintY = max(0, maxY - 2);
                move(hintY, 0); clrtoeol();
                attron(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
                mvprintw(hintY, max(0, (maxX - static_cast<int>(bottomHint.size())) / 2), "%s", bottomHint.c_str());
                attroff(COLOR_PAIR(1) | A_BOLD | A_REVERSE);
            }
            showButton();
            refresh();

            choice = readKeyAnimFrame(120);
            animationFrame++;

            if (choice == ERR) continue;
            if (choice == KET_HOME_BUTTON) {
                returnToDifficultyMenu = true;
                return;
            }
            if (choice == '1' || choice == '2' || choice == '3') {
                valid = true;
            }
        }
        
        int playerAttack = 0;
        int defendSuccess = 0;
        if (choice == '1') playerAttack = rand() % p.atk + 1;
        else if (choice == '2') {
            if (p.hp <= 3) {
                clear();
                PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                centerPrint(getCenteredStartY(3), "Not enough HP! (need at least 3 HP)");
                refresh();
                ncWait();
                continue;
            }
            clear();
            displayPlayerStats(statsPanel);
            centerPrint(getCenteredStartY(2), "This will cost 3 HP. Continue? (Y/N)");
            refresh();
            int confirm = readKeyWithWindowGuard();
            if (confirm == KET_HOME_BUTTON) {
                returnToDifficultyMenu = true;
                return;
            }
            if (confirm != 'Y' && confirm != 'y') {
                continue;
            }
            p.hp -= 3;
            if (rand() % 100 < 75) {
                double mult = 1.3 + (rand() % 40) / 100.0;
                playerAttack = (int)(p.atk * mult); // Boss战没有debuff，直接用p.atk
                if (playerAttack < 1) playerAttack = 1;
            } else {
                playerAttack = 0;
            }
        }        else if (choice == '3') defendSuccess = (rand() % 100 < 40) ? 1 : 0;

        bossHP -= playerAttack;
        
        clear();
        
        // Display player stats panel
        statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);
        drawBossAnimFrame(animationFrame, bossX, bossStartY, maxY);
        
        y = bossStartY - 3;
        if (playerAttack > 0) centerPrint(y++, "You dealt " + to_string(playerAttack) + " damage!");
        else if (choice == '1' || choice == '2') centerPrint(y++, "Attack missed!");

        if (bossHP <= 0) {
            centerPrint(y++, "Boss dealt 0 damage!");
            centerPrint(y++, "Boss defeated! +100 Gold, +200 EXP");
            refresh();
            ncWait();

            p.gold += 100;
            p.exp += 200;
            levelUp(p);
            break;
        }

        int dmg = (rand() % (bossMax - bossMin + 1)) + bossMin;
        if (choice == '3') {
            if (defendSuccess) {
                int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                bossHP -= counterDmg;
                p.hp = min(p.hp + 5, 100);
                dmg = 0;
                centerPrint(y++, "Defend successful! Counter attack: " + to_string(counterDmg) + " damage!");
                if (bossHP <= 0) {
                    centerPrint(y++, "Boss dealt 0 damage!");
                    centerPrint(y++, "Boss defeated! +100 Gold, +200 EXP");
                    refresh();
                    ncWait();

                    p.gold += 100;
                    p.exp += 200;
                    levelUp(p);
                    break;
                }
            } else {
                dmg = (int)(dmg * 0.4);
            }
        }
        if (dmg < 1) dmg = 1;
        p.hp -= dmg;
        centerPrint(y++, "Boss dealt " + to_string(dmg) + " damage!");
        refresh();
        ncWait();
    }
}

/**
 * @brief Handles the interactive shop interface.
 * @details Displays a themed boutique where the player can exchange gold for 
 * healing items or permanent stat increases. Includes graphical mushroom renders.
 * @param p Reference to the player state for gold and stat updates.
 */
// ===== Shop System =====
void shop(Player &p) {
    bool shopping = true;
    while (shopping) {
        clear();
        displayDifficultyLevel(currentDifficulty);

        PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);

        attron(COLOR_PAIR(2) | A_BOLD); 
        centerPrint(1, "=== MUSHROOM BOUTIQUE ===");
        centerPrint(2, "What do you want to buy?");
        attroff(COLOR_PAIR(2) | A_BOLD);

        int midX = COLS / 2;
        int leftCol = midX - 25;
        int rightCol = midX + 5;
        int row1 = 4;
        int row2 = 14;

        drawMushroom(row1, leftCol, 1, '-');
        mvprintw(row1 + 7, leftCol + 1, "1. Normal Mushroom (30G)");
        mvprintw(row1 + 8, leftCol + 4, "[Heal 15 HP]");

        drawMushroom(row1, rightCol, 4, 'O');
        mvprintw(row1 + 7, rightCol + 1, "2. Herbal Mushroom (70G)");
        mvprintw(row1 + 8, rightCol + 4, "[Heal 40 HP]");

        drawMushroom(row2, leftCol, 3, 'X');
        mvprintw(row2 + 7, leftCol + 1, "3. Attack Mushroom (50G)");
        mvprintw(row2 + 8, leftCol + 4, "[Perm +3 ATK]");

        drawMushroom(row2, rightCol, 5, 'U');
        mvprintw(row2 + 7, rightCol + 1, "4. Defense Mushroom (40G)");
        mvprintw(row2 + 8, rightCol + 4, "[Perm +2 DEF]");

        attron(A_REVERSE);
        mvprintw(LINES - 4, (COLS - 10) / 2, " 5. Leave ");
        attroff(A_REVERSE);
        
        refresh();

        int choice = readKeyWithWindowGuard();
        if (choice == KET_HOME_BUTTON) {
            returnToDifficultyMenu = true;
            return;
        }
        string msg = "";

        // ----- Purchase Handling with HP cap check -----
        if (choice == '1') {
            if (p.hp >= 100) {
                msg = "Cannot buy Normal Mushroom: HP is already full!";
            } else if (p.gold >= 30) { 
                p.gold -= 30; 
                p.hp = (p.hp + 15 > 100) ? 100 : p.hp + 15; 
                msg = "Bought Normal Mushroom!"; 
            } else { 
                msg = "Not enough gold!";
            }
        } 
        else if (choice == '2') {
            if (p.hp >= 100) {
                msg = "Cannot buy Herbal Mushroom: HP is already full!";
            } else if (p.gold >= 70) { 
                p.gold -= 70; 
                p.hp = (p.hp + 40 > 100) ? 100 : p.hp + 40; 
                msg = "Bought Herbal Mushroom!"; 
            } else { 
                msg = "Not enough gold!";
            }
        } 
        else if (choice == '3') {
            if (p.gold >= 50) { 
                p.gold -= 50; 
                p.atk += 3; 
                msg = "ATK Increased by 3!"; 
            } else { 
                msg = "Not enough gold!";
            }
        } 
        else if (choice == '4') {
            if (p.gold >= 40) { 
                p.gold -= 40; 
                p.def += 2; 
                msg = "DEF Increased by 2!"; 
            } else { 
                msg = "Not enough gold!";
            }
        } 
        else if (choice == '5') {
            shopping = false;
        }

        // ----- Display feedback message -----
        if (!msg.empty()) {
            clear();
            displayPlayerStats({p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey});
            attron(COLOR_PAIR(2) | A_BOLD);
            centerPrint(getCenteredStartY(1), msg.c_str());
            attroff(COLOR_PAIR(2) | A_BOLD);
            refresh();
            ncWait();
        }
    }
    clear();
}


// ===== Event System =====
void event(Player &p, int monsterMin, int monsterMax, [[maybe_unused]] int bossMin, [[maybe_unused]] int bossMax) {
    int r = rand() % 100;
    
    if (r < 20) {
        fight(p, monsterMin, monsterMax);
    } 
    else if (r < 30) {
        shop(p);
    } 
    else if (r < 45) {
        clear();
        displayDifficultyLevel(currentDifficulty);
        int mushroomRoll = rand() % 100;
        int startY = getCenteredStartY(7);
        int startX = getCenteredX(" :     P       P     : "); 

        if (mushroomRoll < 40) {
            // Normal Mushroom: White, HP +15
            drawMushroom(startY, startX, 1, '-');
            centerPrint(startY + 8, "You found a Normal Mushroom! HP +15.");
            p.hp = min(p.hp + 15, 100);
        } 
        else if (mushroomRoll < 50) {
            // Herbal Mushroom: Green, HP +40
            drawMushroom(startY, startX, 4, 'O');
            centerPrint(startY + 8, "You found a Herbal Mushroom! HP +40.");
            p.hp = min(p.hp + 40, 100);
        } 
        else if (mushroomRoll < 60) {
            // Attack Mushroom: Red, ATK +3
            drawMushroom(startY, startX, 3, 'X');
            centerPrint(startY + 8, "You found an Attack Mushroom! ATK +3.");
            p.atk += 3;
        } 
        else {
            // Defense Mushroom: Blue, DEF +2
            drawMushroom(startY, startX, 5, 'U');
            centerPrint(startY + 8, "You found a Defense Mushroom! DEF +2.");
            p.def += 2;
        }
    }
    else {
        return;
    }

    refresh();
    ncWait();
}

/**
 * @brief Displays the treasure chest opening sequence.
 * @details Renders a multi-frame ASCII animation of a chest opening, 
 * followed by a gold reward and a summary of the player's updated balance.
 * @param p Player reference for gold updates.
 */
// ===== Chest Event Display =====
void displayChestEvent(Player &p) {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    
    clear();
    
    // Draw yellow border
    attron(COLOR_PAIR(2) | A_BOLD);
    for (int x = 8; x < maxX - 8; x++) {
        mvaddch(8, x, '=');
        mvaddch(maxY - 8, x, '=');
    }
    for (int y = 8; y < maxY - 8; y++) {
        mvaddch(y, 8, '|');
        mvaddch(y, maxX - 9, '|');
    }
    mvaddch(8, 8, '+');
    mvaddch(8, maxX - 9, '+');
    mvaddch(maxY - 8, 8, '+');
    mvaddch(maxY - 8, maxX - 9, '+');
    attroff(COLOR_PAIR(2) | A_BOLD);
    
    // Title
    attron(COLOR_PAIR(3) | A_BOLD);
    string titleText = "=== TREASURE CHEST ===";
    int titleX = (maxX - static_cast<int>(titleText.length())) / 2;
    mvprintw(10, titleX, "%s", titleText.c_str());
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    // Chest animation - cycle through 3 frames over time
    for (int i = 0; i < 3; i++) {
        clear();
        
        // Redraw border
        attron(COLOR_PAIR(2) | A_BOLD);
        for (int x = 8; x < maxX - 8; x++) {
            mvaddch(8, x, '=');
            mvaddch(maxY - 8, x, '=');
        }
        for (int y = 8; y < maxY - 8; y++) {
            mvaddch(y, 8, '|');
            mvaddch(y, maxX - 9, '|');
        }
        mvaddch(8, 8, '+');
        mvaddch(8, maxX - 9, '+');
        mvaddch(maxY - 8, 8, '+');
        mvaddch(maxY - 8, maxX - 9, '+');
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        // Title
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(10, titleX, "%s", titleText.c_str());
        attroff(COLOR_PAIR(3) | A_BOLD);
        
        // Draw chest ASCII art
        int chestY = 13;
        int chestX = maxX / 2 - 6;
        
        attron(COLOR_PAIR(4) | A_BOLD);
        if (i < 2) {
            // Closed chest (frame 0-1)
            mvprintw(chestY, chestX, "  ______  ");
            mvprintw(chestY + 1, chestX, " |  []  | ");
            mvprintw(chestY + 2, chestX, " |______| ");
            mvprintw(chestY + 3, chestX, "  ------  ");
        } else {
            // Open chest (frame 2)
            mvprintw(chestY, chestX, "  ______  ");
            mvprintw(chestY + 1, chestX, " |  ^^  | ");
            mvprintw(chestY + 2, chestX, " |______|  ");
            mvprintw(chestY + 3, chestX, "    **    ");
        }
        attroff(COLOR_PAIR(4) | A_BOLD);
        
        refresh();
        napms(500);
    }
    
    // Show gold amount info
    int infoY = 19;
    attron(COLOR_PAIR(1));
    string beforeText = "Before: " + to_string(p.gold) + " Gold";
    int beforeX = (maxX - static_cast<int>(beforeText.length())) / 2;
    mvprintw(infoY, beforeX, "%s", beforeText.c_str());
    
    // Update gold
    p.gold += 20;
    
    string afterText = "After: " + to_string(p.gold) + " Gold (+20)";
    int afterX = (maxX - static_cast<int>(afterText.length())) / 2;
    mvprintw(infoY + 2, afterX, "%s", afterText.c_str());
    
    attron(COLOR_PAIR(2) | A_BOLD);
    string msgText = "Press Enter to continue";
    int msgX = (maxX - static_cast<int>(msgText.length())) / 2;
    mvprintw(infoY + 4, msgX, "%s", msgText.c_str());
    attroff(COLOR_PAIR(2) | A_BOLD);
    attroff(COLOR_PAIR(1));
    
    refresh();
    ncWait();
}

/**
 * @brief Renders the game world grid to the terminal.
 * @details Draws a formatted grid showing the player position (P), walls (#), 
 * undiscovered cells (?), and explored paths (.). Uses color-coded attributes 
 * for improved navigation visibility.
 */
// ===== Map Display =====
void displayMap() {
    // Don't clear here - let the main loop handle clearing
    // clear();

    int termH, termW;
    getmaxyx(stdscr, termH, termW); 

    // Each cell takes 3 characters wide, 2 rows high (border + content)
    int cellWidth = 3;
    int cellHeight = 2;
    int mapW = SIZE * cellWidth + 1;
    int mapH = SIZE * cellHeight + 1;

    int startY = (termH - mapH - 2) / 2;
    int startX = (termW - mapW) / 2;

    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    // Center the MAP title within the map grid (not screen center)
    // The actual map width is SIZE * 4 + 1 (each "+---" is 4 chars, plus final "+")
    string mapTitle = "===== MAP =====";
    int actualMapWidth = SIZE * 4 + 1;
    int titleX = startX + (actualMapWidth - static_cast<int>(mapTitle.length())) / 2;
    if (titleX < 0) titleX = 0;
    mvprintw(startY, titleX, "%s", mapTitle.c_str());
    startY += 2;

    // Draw top border
    move(startY, startX);
    for (int j = 0; j < SIZE; j++) {
        printw("+---");
    }
    printw("+");
    
    // Draw map grid
    for (int i = 0; i < SIZE; i++) {
        // Draw left border and content
        move(startY + 1 + i * cellHeight, startX);
        for (int j = 0; j < SIZE; j++) {
            char cellChar = ' ';
            int colorPair = 1;  // default white
            bool isPlayer = (i == px && j == py);
            
            if (isPlayer) {
                cellChar = 'P';
                colorPair = 4;  // Green for player
            } else if (grid[i][j] == '#') {
                cellChar = '#';
                colorPair = 3;  // Red for wall
            } else if (!discovered[i][j]) {
                cellChar = '?';
                colorPair = 2;  // Yellow for undiscovered
            } else {
                cellChar = '.';
                colorPair = 1;  // White for empty
            }
            
            // Print cell with color
            printw("|");
            if (isPlayer) {
                attron(COLOR_PAIR(colorPair) | A_BOLD | A_REVERSE);
                printw(" %c ", cellChar);
                attroff(COLOR_PAIR(colorPair) | A_BOLD | A_REVERSE);
            } else {
                attron(COLOR_PAIR(colorPair) | A_BOLD);
                printw(" %c ", cellChar);
                attroff(COLOR_PAIR(colorPair) | A_BOLD);
            }
        }
        printw("|");
        
        // Draw horizontal border between rows
        move(startY + 2 + i * cellHeight, startX);
        for (int j = 0; j < SIZE; j++) {
            printw("+---");
        }
        printw("+");
    }

    // Draw bottom border (already done in the last iteration, but let's be explicit)
    move(startY + SIZE * cellHeight, startX);
    for (int j = 0; j < SIZE; j++) {
        printw("+---");
    }
    printw("+");

    refresh();
}

/**
 * @brief Handles player movement and interaction with map cells.
 * @details Updates player coordinates, tracks discovery/visits, and triggers 
 * specific logic for keys, traps, chests, bosses, or the rescue goal based 
 * on the target cell's character.
 * @param m Movement direction input (w/a/s/d).
 * @param p Player reference.
 * @param monsterMin/Max Attack range for standard encounters.
 * @param bossMin/Max Attack range for boss encounters.
 */
// ===== Movement =====
void movePlayer(char m, Player &p, int monsterMin, int monsterMax, int bossMin, int bossMax) {
    int nx = px, ny = py;
    if (m == 'w') nx--;
    else if (m == 's') nx++;
    else if (m == 'a') ny--;
    else if (m == 'd') ny++;

    if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) return;
    if (grid[nx][ny] == '#') { 
        clear();
        centerPrint(getCenteredStartY(1), "Blocked!");
        refresh();
        napms(500);
        return; 
    }

    px = nx; py = ny;
    discovered[px][py] = true;
    bool firstVisit = !visited[px][py];
    visited[px][py] = true;

    if (grid[px][py] == 'K') { 
        clear();
        centerPrint(getCenteredStartY(1), "You found the key!");
        ncWait();
        p.hasKey = true; 
        grid[px][py] = '.';
        refresh();
        napms(500);
        return; 
    }
    if (grid[px][py] == 'T') { 
        clear();
        centerPrint(getCenteredStartY(1), "Trap! HP -15");
        p.hp -= 15; 
        grid[px][py] = '.';
        refresh();
        ncWait();
        return; 
    }
    if (grid[px][py] == 'C') { 
        displayChestEvent(p);
        grid[px][py] = '.';
        return; 
    }
    if (grid[px][py] == 'B') {
        bossFight(p, bossMin, bossMax);
        if (!returnToDifficultyMenu) {
            grid[px][py] = '.';
        }
        return;
    }
    if (grid[px][py] == 'G') {
        if (!p.hasKey) {
            clear();
            centerPrint(getCenteredStartY(1), "You have not found the key yet! Cannot rescue princess.");
            refresh();
            ncWait();
        } else {
            princessRoomMinigame(p, false);
            grid[px][py] = '.';
        }
        return;
    }
    if (grid[px][py] == '.') {
        if (firstVisit) {
            event(p, monsterMin, monsterMax, bossMin, bossMax);
            if (returnToDifficultyMenu) {
                return;
            }
        }
    }
            
}

/**
 * @brief Entry point and core engine loop for the game.
 * @details Initializes ncurses settings, color pairs, and mouse support.
 * Manages the high-level game state, including:
 * 1. User authentication and title/intro sequences.
 * 2. Difficulty session management and save data persistence.
 * 3. The primary game loop, which refreshes the map and player stats.
 * 4. Victory/death conditions and input handling for movement and mouse events.
 * @return 0 on successful termination.
 */
// ===== Main =====
int main() {
    setlocale(LC_ALL, "");
    //Initialize ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, nullptr);
    mouseinterval(150);
    curs_set(0);  // Hide cursor
    start_color();
    init_pair(1, COLOR_WHITE,  COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED,    COLOR_BLACK);
    init_pair(4, COLOR_GREEN,  COLOR_BLACK);
    init_pair(5, COLOR_BLUE,   COLOR_BLACK);
    enforceWindowSizeGate();

    srand(time(0));
    Player p;
    if (!showTitle()) {
        endwin();
        return 0;
    }
    showIntro();

    string username;
    if (!authenticateUser(username)) {
        clear();
        centerPrint(getCenteredStartY(1), "Authentication failed too many times. Exit.");
        refresh();
        ncWait();
        endwin();
        return 0;
    }

    int monsterMin, monsterMax, bossMin, bossMax;
    string activeSaveSlot;
    startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true);

    while (true) {
        clear();
        cursorY = 0;
        
        // Display difficulty level with highlight at top-right
        displayDifficultyLevel(currentDifficulty);
        
        // Display map
        displayMap();
        
        // Display player stats panel in top-left
        PlayerStats statsPanel = {
            p.hp, 100,
            p.atk,
            p.def,
            p.gold,
            p.exp,
            p.level,
            p.hasKey
        };
        displayPlayerStats(statsPanel);
        
        showButton();
        
        refresh();

        if (p.hp <= 0) { 
            PostDeathAction action = promptPostDeathAction();
            if (action == PostDeathAction::Quit) {
                break;
            }

            startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true, true);
            continue;
        }
        if (px == gx && py == gy && p.hasKey) { 
            clear();
            static const string diffNames[] = {"", "Easy", "Normal", "Hard", "Hell"};
            string diffName = (currentDifficulty >= 1 && currentDifficulty <= 4)
                              ? diffNames[currentDifficulty] : to_string(currentDifficulty);
            attron(COLOR_PAIR(4) | A_BOLD);
            centerPrint(getCenteredStartY(3),     "*** VICTORY! ***");
            centerPrint(getCenteredStartY(3) + 1, "You rescued the princess!");
            centerPrint(getCenteredStartY(3) + 2, "Difficulty [" + diffName + "] CLEARED!");
            attroff(COLOR_PAIR(4) | A_BOLD);
            showButton();
            refresh();
            ncWait();

            // Save a permanent cleared marker for this difficulty
            {
                user_save_system::SaveData clearedMark;
                clearedMark.cleared = true;
                clearedMark.valid   = true;
                clearedMark.size    = 1;
                clearedMark.gridRows      = {"."};
                clearedMark.discoveredRows = {"0"};
                clearedMark.visitedRows    = {"0"};
                user_save_system::saveProgress(buildClearedSlot(username, currentDifficulty), clearedMark);
            }

            // Mark game slot as cleared so next session starts fresh
            {
                user_save_system::SaveData completedState =
                    buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax);
                completedState.cleared = true;
                user_save_system::saveProgress(activeSaveSlot, completedState);
            }

            // Return to difficulty selection
            returnToDifficultyMenu = false;
            startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true);
            continue;
        }
        if (px == gx && py == gy && !p.hasKey) {
            centerPrint(cursorY++, "You have not found the key yet! Cannot rescue princess.");
        }

        // Display movement instruction at fixed position (bottom of screen, above buttons)
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(maxY - 3, 0, "%-*s", maxX, "");  // Clear line
        mvprintw(maxY - 3, max(0, (maxX - 32) / 2), "Move (W/A/S/D or Arrow Keys):");
        attroff(COLOR_PAIR(1) | A_BOLD);
        
        showButton();
        refresh();
        
        int key = readKeyWithWindowGuard();

        if (key == KET_HOME_BUTTON) {
            user_save_system::saveProgress(activeSaveSlot, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
            startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true);
            continue;
        }

        if (key == KEY_MOUSE) {
            MEVENT event;
            if (getmouse(&event) == OK && isPrimaryMouseClick(event)) {
                TopButtonAction action = getTopButtonActionFromMouse(event);
                if (action == TopButtonAction::Help) {
                    showHelp();
                    continue;
                }
                if (action == TopButtonAction::Home) {
                    user_save_system::saveProgress(activeSaveSlot, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
                    startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true);
                    continue;
                }
                if (action == TopButtonAction::Quit) {
                    break;
                }
            }
            continue;
        }

        char m = normalizeMoveKey(key);
        if (m != '\0') {
            movePlayer(m, p, monsterMin, monsterMax, bossMin, bossMax);
            user_save_system::saveProgress(activeSaveSlot, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
            if (returnToDifficultyMenu) {
                startDifficultySession(username, activeSaveSlot, p, monsterMin, monsterMax, bossMin, bossMax, true);
                continue;
            }
        }
    }

    endwin();
    return 0;
}
