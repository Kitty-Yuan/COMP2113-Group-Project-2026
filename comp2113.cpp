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

struct Player {
    int hp = 100;
    int atk = 8;
    int def = 5;
    int gold = 10;
    int exp = 0;
    int level = 1;
    bool hasKey = false;
};

int px = 0, py = 0;
int gx, gy;
int cursorY = 0; // Global cursor for display
int cursorX = 0;

char normalizeMoveKey(int key) {
    if (key == 'w' || key == 'W' || key == KEY_UP) return 'w';
    if (key == 's' || key == 'S' || key == KEY_DOWN) return 's';
    if (key == 'a' || key == 'A' || key == KEY_LEFT) return 'a';
    if (key == 'd' || key == 'D' || key == KEY_RIGHT) return 'd';
    return '\0';
}

enum class PostDeathAction {
    Home,
    Quit
};

bool isPrimaryMouseClick(const MEVENT &event) {
    mmask_t clickMask = BUTTON1_CLICKED | BUTTON1_PRESSED | BUTTON1_RELEASED |
                        BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED;
    return (event.bstate & clickMask) != 0;
}

PostDeathAction promptPostDeathAction() {
    while (true) {
        clear();
        vector<string> lines = {
            "You died. Game Over.",
            "",
            "Click HOME to return to title screen.",
            "Click QUIT to quit the program."
        };
        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            centerPrint(startY + i, lines[i]);
        }
        showButton();
        refresh();

        int ch = readKeyWithWindowGuard();
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

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            data.gridRows[i][j] = grid[i][j];
            data.discoveredRows[i][j] = discovered[i][j] ? '1' : '0';
        }
    }

    return data;
}

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

    if (static_cast<int>(data.gridRows.size()) != SIZE || static_cast<int>(data.discoveredRows.size()) != SIZE) {
        return false;
    }

    for (int i = 0; i < SIZE; i++) {
        if (static_cast<int>(data.gridRows[i].size()) != SIZE || static_cast<int>(data.discoveredRows[i].size()) != SIZE) {
            return false;
        }
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = data.gridRows[i][j];
            discovered[i][j] = (data.discoveredRows[i][j] == '1');
            visited[i][j] = false;
        }
    }

    if (px < 0 || px >= SIZE || py < 0 || py >= SIZE || gx < 0 || gx >= SIZE || gy < 0 || gy >= SIZE) {
        return false;
    }

    discovered[px][py] = true;
    return true;
}

// Helper function to check if a path exists using BFS
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

// Helper function to create a path between two points
void createPath(int sx, int sy, int ex, int ey) {
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
            grid[x][y] = '.';
            int px = parent_x[x][y];
            int py = parent_y[x][y];
            x = px;
            y = py;
        }
    }
}

void initializeNewMap() {
    // Generate random map with walls
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = (rand()%100 < 25 ? '#' : '.');
            visited[i][j] = false;
            discovered[i][j] = false;
        }
    }

    // Place key locations
    grid[px][py] = '.';
    grid[gx][gy] = 'G';
    grid[SIZE/2][SIZE/2] = 'B';
    
    // Place collectible items randomly
    int kx, ky, tx, ty, cx, cy;
    do {
        kx = rand() % SIZE;
        ky = rand() % SIZE;
    } while ((kx == px && ky == py) || (kx == gx && ky == gy) || (kx == SIZE/2 && ky == SIZE/2));
    grid[kx][ky] = 'K';
    
    do {
        tx = rand() % SIZE;
        ty = rand() % SIZE;
    } while ((tx == px && ty == py) || (tx == gx && ty == gy) || (tx == SIZE/2 && ty == SIZE/2) || 
             (tx == kx && ty == ky));
    grid[tx][ty] = 'T';
    
    do {
        cx = rand() % SIZE;
        cy = rand() % SIZE;
    } while ((cx == px && cy == py) || (cx == gx && cy == gy) || (cx == SIZE/2 && cy == SIZE/2) || 
             (cx == kx && cy == ky) || (cx == tx && cy == ty));
    grid[cx][cy] = 'C';
    
    // Ensure paths exist between critical locations
    createPath(px, py, kx, ky);      // P -> K
    createPath(kx, ky, gx, gy);      // K -> G
    createPath(px, py, tx, ty);      // P -> T
    createPath(px, py, cx, cy);      // P -> C
    createPath(px, py, SIZE/2, SIZE/2);  // P -> B
    
    discovered[px][py] = true;
}

// =====Catch princess=====
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

        centerPrint(startY + roomSize + 3, "Move (W/A/S/D or Arrow Keys):");
        refresh();
        
        int key = readKeyWithWindowGuard();
        char m = normalizeMoveKey(key);
        int nx = rpx, ny = rpy;
        if (m == 'w') nx--; else if (m == 's') nx++;
        else if (m == 'a') ny--; else if (m == 'd') ny++;

        if (nx > 0 && nx < roomSize-1 && ny > 0 && ny < roomSize-1 && room[nx][ny] != '#') {
            rpx = nx; rpy = ny;
            stepsUsed++;
        }

        if (stepsUsed > 0 && stepsUsed % 2 == 0) {
            int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
            int dir = rand() % 4;
            int ngx = rgx + dx[dir], ngy = rgy + dy[dir];
            if (room[ngx][ngy] == '.') { rgx = ngx; rgy = ngy; }
        }
    }
}

// ===== Difficulty =====
void chooseDifficulty(int &monsterMin, int &monsterMax, int &bossMin, int &bossMax) {
    int diff = 0;
    
    while (diff < 1 || diff > 4) {
        clear();
        vector<string> lines = {
            "Choose difficulty (1-4):",
            "1. Easy (9x9 map, monster ATK 5-10, boss ATK 10-15)",
            "2. Normal (12x12 map, monster ATK 8-12, boss ATK 12-18)",
            "3. Hard (15x15 map, monster ATK 10-15, boss ATK 15-22)",
            "4. Hell (20x20 map, monster ATK 11-16, boss ATK 15-25)",
            "Enter choice: 1 / 2 / 3 / 4"
        };
        int startY = getCenteredStartY(static_cast<int>(lines.size()));
        for (int i = 0; i < static_cast<int>(lines.size()); i++) {
            centerPrint(startY + i, lines[i]);
        }
        refresh();
        
        int ch = readKeyWithWindowGuard();
        if (ch >= '1' && ch <= '4') {
            diff = ch - '0';
        }

    }
    
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

//Intro Screen
void introScreen() {
    clear();

    int y = getCenteredStartY(10);

    centerPrint(y++, "==================================");
    centerPrint(y++, "      YOUR ADVENTURE BEGINS      ");
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

    centerPrint(y++, "Press ENTER to start your journey...");

    refresh();

    while (true) {
        int ch = readKeyWithWindowGuard();
        if (ch == '\n' || ch == KEY_ENTER) break;
    }
}


// ===== Level Up =====
void levelUp(Player &p) {
    int y = 0;
    while (p.exp >= 100) {
        p.exp -= 100;
        p.level++;
        p.hp += 20;
        p.atk += 5;
        p.def += 3;
        clear();
        mvprintw(y++, 0, "Level Up! Lv %d HP+20 ATK+5 DEF+3", p.level);
        refresh();
        napms(1000); // Wait 1 second
    }
}

//Tutorial Catch Princess
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

        centerPrint(startY++, "Catch the enemy!");
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

        int nx = px, ny = py;
        if (m == 'w') nx--;
        else if (m == 's') nx++;
        else if (m == 'a') ny--;
        else if (m == 'd') ny++;

        if (grid[nx][ny] == '#') continue;

        px = nx;
        py = ny;
        steps++;

        // ===== 🎯 catch the enemy =====
        if (px == ex && py == ey) {
            clear();
            centerPrint(getCenteredStartY(2), "You caught the enemy!");
            centerPrint(getCenteredStartY(2)+1, "Victory!");
            refresh();
            ncWait();
            return;
        }

        // ===== 👾 movement=====
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
        }
    }
}


// ===== Tutorial =====
void tutorial(Player &p) {
    clear();
    centerPrint(getCenteredStartY(1), "===== TUTORIAL =====");
    refresh();
    ncWait();

    char demoMap[5][5] = {
        {'P','.','#','.','K'},
        {'.','#','.','.','.'},
        {'.','.','B','#','.'},
        {'#','.','#','.','.'},
        {'.','.','#','.','G'}
    };

    bool hasKey = false;
    int x = 0, y_pos = 0;
    int step = 0;

    while (true) {
        clear();

        int y = 0;
        [[maybe_unused]] int maxY;
        int maxX;
        getmaxyx(stdscr, maxY, maxX);

        int startY = getCenteredStartY(12);
        int startX = max(0, (maxX - 10) / 2);

        // ======================
        // 🎯 STEP HINT SYSTEM
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

        if (step == 0) {
            step = 1;
        }

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
            
            int monsterHP=20;
            int minPower=5,maxPower=7;
            while (monsterHP>0 && p.hp>0) {
                clear();
                y = 0;
                mvprintw(y++, 0, "BATTLE - Your HP: %d | Monster HP: %d", p.hp, monsterHP);
                mvprintw(y++, 0, "Choose: 1) Normal  2) Strong  3) Defend");
                refresh();
                
                int choice = readKeyWithWindowGuard();
                
                int playerAttack=0;
               int defendSuccess=0;
                if (choice=='1') playerAttack=rand()%p.atk+1;
                else if (choice=='2') {
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
                } 
                    
                else if (choice=='3') defendSuccess=(rand()%100<40)?1:0;
                
                monsterHP-=playerAttack;
                
                clear();
                y = 0;
                if(playerAttack>0) mvprintw(y++, 0, "You dealt %d damage!", playerAttack);
                if(monsterHP<=0){
                    mvprintw(y++, 0, "monster defeated!");
                    p.gold+=10; 
                    p.exp+=20;
                    refresh();
                    napms(500);
                    break;
                }
                
                int edmg=(rand()%(maxPower-minPower+1))+minPower;
                if(choice=='3') {
                    if(defendSuccess) {
                        int counterDmg=(int)(p.atk*0.4+edmg*(0.4+rand()%20/100.0));
                        monsterHP-=counterDmg;
                        p.hp+=5;
                        edmg=0;
                        mvprintw(y++, 0, "Defend success! Counter: %d damage!", counterDmg);
                        if(monsterHP<=0) {
                            mvprintw(y++, 0, "monster defeated!");
                            p.gold+=10; p.exp+=20; refresh(); napms(500); break;
                        }
                    } else {
                        edmg=(int)(edmg*0.4);
                    }
                }
                if(edmg<1) edmg=1;
                p.hp-=edmg;
                mvprintw(y++, 0, "monster dealt %d damage!", edmg);
                refresh();
                napms(500);
            }
            demoMap[x][y_pos]='.';
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
                "You found the exit...");
            refresh();
            napms(800);

            // 🎮 进入小游戏
            tutorialMinigame(p);

            // 🎯 完成
            centerPrint(getCenteredStartY(1),
                "Tutorial Complete!");
            refresh();
            ncWait();

            break;
        }
    }
}    


// ===== Battle System =====
void fight(Player &p, int monsterMin, int monsterMax) {
    clear();
    
    // Display player stats panel
    PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
    displayPlayerStats(statsPanel);
    
    int monsterIndex = rand() % monsters.size();
    Monster m = monsters[monsterIndex];
    
    // Special ability states
    int turnsReduceATK = 0;           // Remaining turns with 50% ATK reduction (Ghost attack1)
    bool ghostRedForm = false;        // Ghost is in red form (attack2)
    
    int y = getCenteredStartY(3);
    centerPrint(y++, "You encountered: " + m.name + "!");
    centerPrint(y++, "Monster attack range: " + to_string(monsterMin) + " - " + to_string(monsterMax));
    refresh();
    napms(1000);
    
    int monsterHP = 30 + rand() % 20;

    while (monsterHP > 0 && p.hp > 0) {
        clear();
        
        // Display player stats panel
        PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);
        
        // Display monster appearance in center
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        int monsterX = (maxX - 20) / 2;  // Center horizontally
        int monsterStartY = max(5, (maxY - 12) / 2);  // Center vertically
        
        // Display monster name and appearance at center
        mvprintw(monsterStartY, monsterX, "%s", m.name.c_str());
        
        // Special display for Ghost with red form
        string currentAppearance = m.appearance1;
        if (ghostRedForm && m.name == "Ghost") {
            attron(COLOR_PAIR(2) | A_BOLD);  // Red color
        }
        
        istringstream iss(currentAppearance);
        string line;
        int lineY = monsterStartY + 1;
        while (getline(iss, line) && lineY < maxY - 5) {
            mvprintw(lineY++, monsterX, "%s", line.c_str());
        }
        
        if (ghostRedForm && m.name == "Ghost") {
            attroff(COLOR_PAIR(2) | A_BOLD);
        }
        
        // Display battle info above monster
        y = monsterStartY - 3;
        centerPrint(y++, "BATTLE - " + m.name + " HP: " + to_string(monsterHP));
        if (turnsReduceATK > 0) {
            attron(COLOR_PAIR(2) | A_BOLD);
            centerPrint(y++, "** Your ATK is reduced 50%% for " + to_string(turnsReduceATK) + " more turn(s) **");
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else if (ghostRedForm && m.name == "Ghost") {
            attron(COLOR_PAIR(2) | A_BOLD);
            centerPrint(y++, "** Your ATK is reduced 80%% this turn **");
            attroff(COLOR_PAIR(2) | A_BOLD);
        } else {
            y++;
        }
        centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
        refresh();
        
        int choice;
        bool valid = false;

        while (!valid) {
            clear();
            
            // Display player stats panel
            displayPlayerStats(statsPanel);
            
            // Display monster appearance in center
            mvprintw(monsterStartY, monsterX, "%s", m.name.c_str());
            if (ghostRedForm && m.name == "Ghost") {
                attron(COLOR_PAIR(2) | A_BOLD);
            }
            istringstream iss(m.appearance1);
            lineY = monsterStartY + 1;
            while (getline(iss, line) && lineY < maxY - 5) {
                mvprintw(lineY++, monsterX, "%s", line.c_str());
            }
            if (ghostRedForm && m.name == "Ghost") {
                attroff(COLOR_PAIR(2) | A_BOLD);
            }
            
            // Display battle info above monster
            y = monsterStartY - 3;
            centerPrint(y++, "BATTLE - " + m.name + " HP: " + to_string(monsterHP));
            if (turnsReduceATK > 0) {
                attron(COLOR_PAIR(2) | A_BOLD);
                centerPrint(y++, "** Your ATK is reduced 50%% for " + to_string(turnsReduceATK) + " more turn(s) **");
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else if (ghostRedForm && m.name == "Ghost") {
                attron(COLOR_PAIR(2) | A_BOLD);
                centerPrint(y++, "** Your ATK is reduced 80%% this turn **");
                attroff(COLOR_PAIR(2) | A_BOLD);
            } else {
                y++;
            }
            centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
            refresh();

            choice = readKeyWithWindowGuard();

            if (choice == '1' || choice == '2' || choice == '3') {
                valid = true;
            } else {
                clear();
                
                // Display player stats panel
                displayPlayerStats(statsPanel);
                
                // Display monster appearance in center
                if (ghostRedForm && m.name == "Ghost") {
                    attron(COLOR_PAIR(2) | A_BOLD);
                }
                istringstream iss2(m.appearance1);
                lineY = monsterStartY + 1;
                while (getline(iss2, line) && lineY < maxY - 5) {
                    mvprintw(lineY++, monsterX, "%s", line.c_str());
                }
                if (ghostRedForm && m.name == "Ghost") {
                    attroff(COLOR_PAIR(2) | A_BOLD);
                }
                
                // Display error message and battle info above monster
                y = monsterStartY - 3;
                centerPrint(y++, "Invalid input!");
                centerPrint(y++, "Please press 1 / 2 / 3");
                centerPrint(y++, "1) Normal  2) Strong  3) Defend");
                centerPrint(y++, "OR click MANUAL for help");
                refresh();
                napms(600);
            }
        }
        
        // Calculate player attack with debuff effects
        int playerAttack = 0;
        int defendSuccess = 0;
        int atkBuff = p.atk;
        
        // Apply ATK reduction if active
        if (turnsReduceATK > 0) {
            atkBuff = (int)(p.atk * 0.5);  // 50% reduction
        } else if (ghostRedForm && m.name == "Ghost") {
            atkBuff = (int)(p.atk * 0.2);  // 80% reduction -> 20% remains
        }
        
        if (choice == '1') playerAttack = rand() % atkBuff + 1;
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
            if (confirm != 'Y' && confirm != 'y') {
                continue; // Cancel, return to battle menu
            }
            // Deduct HP
            p.hp -= 3;
            // Attack determination
            if (rand() % 100 < 75) {
                double mult = 1.3 + (rand() % 40) / 100.0; // 1.30 ~ 1.69
                playerAttack = (int)(atkBuff * mult);
                if (playerAttack < 1) playerAttack = 1;
            } else {
                playerAttack = 0;
            }
        }        
        else if (choice == '3') defendSuccess = (rand() % 100 < 40) ? 1 : 0;

        monsterHP -= playerAttack;
        
        // Clear ATK reduction for this turn after applying
        ghostRedForm = false;

        clear();
        
        // Display player stats panel
        statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);
        
        // Display monster appearance in center
        mvprintw(monsterStartY, monsterX, "%s", m.name.c_str());
        istringstream iss3(m.appearance1);
        while (getline(iss3, line) && lineY < maxY - 5) {
            mvprintw(lineY++, monsterX, "%s", line.c_str());
        }
        
        // Display battle results above monster
        y = monsterStartY - 3;
        if (playerAttack > 0) centerPrint(y++, "You dealt " + to_string(playerAttack) + " damage!");
        else if (choice == '1' || choice == '2') centerPrint(y++, "Attack missed!");

        if (monsterHP <= 0) {
            centerPrint(y++, "monster defeated! +20 Gold, +50 EXP");
            p.gold += 20; p.exp += 50; levelUp(p);
            refresh();
            ncWait();
            break;
        }

        // Decrease remaining turns of ATK reduction
        if (turnsReduceATK > 0) {
            turnsReduceATK--;
        }

        int dmg = (rand() % (monsterMax - monsterMin + 1)) + monsterMin;
        
        // 100% trigger special ability, with 30% chance to trigger both
        bool useAttack1 = false, useAttack2 = false;
        int bothChance = rand() % 100;
        
        if (bothChance < 30) {
            // 30% chance: trigger both attacks
            useAttack1 = true;
            useAttack2 = true;
        } else {
            // 70% chance: trigger one randomly
            useAttack1 = (rand() % 2 == 0);
            useAttack2 = !useAttack1;
        }
        
        string specialMsg = "";
        
        // Handle Ghost special attack 1: reduce ATK for 2 turns
        if (useAttack1 && m.name == "Ghost") {
            turnsReduceATK = 2;
            specialMsg += "Ghost uses Incorporeal Form! Your ATK will be reduced 50%% for 2 turns! ";
        } else if (useAttack1) {
            dmg = (int)(dmg * 1.3);  // 30% bonus
            specialMsg += m.name + " uses " + m.specialattack1 + "! ";
        }
        
        // Handle Ghost special attack 2: reduce ATK 80% and turn red
        if (useAttack2 && m.name == "Ghost") {
            ghostRedForm = true;
            specialMsg += "Ghost uses Haunting Presence! Your ATK is reduced 80%% this turn and Ghost turns RED! ";
            dmg = (int)(dmg * 1.2);  // 20% bonus
        } else if (useAttack2) {
            dmg = (int)(dmg * 1.2);  // 20% bonus
            specialMsg += m.name + " uses " + m.specialattack2 + "! ";
        }
        
        if (choice == '3') {
            if (defendSuccess) {
                int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                monsterHP -= counterDmg;
                p.hp += 5;
                dmg = 0;
                specialMsg = "";
                turnsReduceATK = 0;  // Clear debuff on successful defend
                ghostRedForm = false;
                centerPrint(y++, "Defend successful! Counter attack: " + to_string(counterDmg) + " damage!");
                if (monsterHP <= 0) {
                    centerPrint(y++, "monster defeated!");
                    p.gold += 20; p.exp += 50; levelUp(p);
                    refresh();
                    ncWait();
                    break;
                }
            } else {
                dmg = (int)(dmg * 0.4);
            }
        }
        if (dmg < 1) dmg = 1;
        p.hp -= dmg;
        
        // Display special ability messages
        if (!specialMsg.empty()) {
            attron(COLOR_PAIR(2) | A_BOLD);
            centerPrint(y++, specialMsg);
            attroff(COLOR_PAIR(2) | A_BOLD);
        }
        centerPrint(y++, m.name + " dealt " + to_string(dmg) + " damage!");
        refresh();
        napms(800);
    }
}

// ===== Boss Fight =====
void bossFight(Player &p, int bossMin, int bossMax) {
    int y = getCenteredStartY(4);
    clear();
    centerPrint(y++, "Boss battle begins!");
    centerPrint(y++, "Boss attack range: " + to_string(bossMin) + " - " + to_string(bossMax));
    refresh();
    napms(1000);
    
    int bossHP = 100;

    while (bossHP > 0 && p.hp > 0) {
        clear();
        
        // Display player stats panel
        PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
        displayPlayerStats(statsPanel);
        
        y = getCenteredStartY(3);
        centerPrint(y++, "BOSS BATTLE - Boss HP: " + to_string(bossHP));
        centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
        refresh();
        
        int choice = readKeyWithWindowGuard();
        
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
        
        y = getCenteredStartY(4);
        if (playerAttack > 0) centerPrint(y++, "You dealt " + to_string(playerAttack) + " damage!");
        else if (choice == '1' || choice == '2') centerPrint(y++, "Attack missed!");

        if (bossHP <= 0) {
            centerPrint(y++, "Boss defeated! +100 Gold, +200 EXP");
            p.gold += 100; p.exp += 200; levelUp(p);
            refresh();
            ncWait();
            break;
        }

        int dmg = (rand() % (bossMax - bossMin + 1)) + bossMin;
        if (choice == '3') {
            if (defendSuccess) {
                int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                bossHP -= counterDmg;
                p.hp += 5;
                dmg = 0;
                centerPrint(y++, "Defend successful! Counter attack: " + to_string(counterDmg) + " damage!");
                if (bossHP <= 0) {
                    centerPrint(y++, "Boss defeated!");
                    p.gold += 100; p.exp += 200; levelUp(p);
                    refresh();
                    ncWait();
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
        napms(800);
    }
}

// ===== Shop System =====
void shop(Player &p) {
    clear();
    
    // Display player stats panel
    PlayerStats statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
    displayPlayerStats(statsPanel);
    
    int startY = getCenteredStartY(6);
    centerPrint(startY++, "=== MERCHANT ===");
    centerPrint(startY++, "Welcome! What would you like to buy?");
    centerPrint(startY++, "1. Small Potion (Heal 15 HP) - 30 gold");
    centerPrint(startY++, "2. Large Potion (Heal 40 HP) - 70 gold");
    centerPrint(startY++, "3. Attack Boost (Permanent +3 ATK) - 50 gold");
    centerPrint(startY++, "4. Defense Boost (Permanent +2 DEF) - 40 gold");
    centerPrint(startY++, "5. Leave");
    refresh();

    while (true) {
        int choice = readKeyWithWindowGuard();
        if (choice == '1') {
            if (p.gold >= 30) {
                p.gold -= 30;
                p.hp += 15;
                clear();
                
                // Display updated stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "You bought a Small Potion. +15 HP.");
                refresh();
                ncWait();
                break;
            } else {
                clear();
                
                // Display stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "Not enough gold!");
                refresh();
                ncWait();
                shop(p);
                return;
            }
        } else if (choice == '2') {
            if (p.gold >= 70) {
                p.gold -= 70;
                p.hp += 40;
                clear();
                
                // Display updated stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "You bought a Large Potion. +40 HP.");
                refresh();
                ncWait();
                break;
            } else {
                clear();
                
                // Display stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "Not enough gold!");
                refresh();
                ncWait();
                shop(p);
                return;
            }
        } else if (choice == '3') {
            if (p.gold >= 50) {
                p.gold -= 50;
                p.atk += 3;
                clear();
                
                // Display updated stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "You bought an Attack Boost. ATK +3!");
                refresh();
                ncWait();
                break;
            } else {
                clear();
                
                // Display stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "Not enough gold!");
                refresh();
                ncWait();
                shop(p);
                return;
            }
        } else if (choice == '4') {
            if (p.gold >= 40) {
                p.gold -= 40;
                p.def += 2;
                clear();
                
                // Display updated stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "You bought a Defense Boost. DEF +2!");
                refresh();
                ncWait();
                break;
            } else {
                clear();
                
                // Display stats
                statsPanel = {p.hp, 100, p.atk, p.def, p.gold, p.exp, p.level, p.hasKey};
                displayPlayerStats(statsPanel);
                
                centerPrint(getCenteredStartY(1), "Not enough gold!");
                refresh();
                ncWait();
                shop(p);
                return;
            }
        } else if (choice == '5') {
            break;
        }
    }
    clear();
}


// ===== Event System =====
void event(Player &p, int monsterMin, int monsterMax, [[maybe_unused]] int bossMin, [[maybe_unused]] int bossMax) {
    int r = rand() % 100;
    if (r < 40) fight(p, monsterMin, monsterMax);       // Monster 40%
    else if (r < 60) {
        shop(p);
    }
    else if (r < 75) {
        clear();
        centerPrint(getCenteredStartY(1), "You met an old man. He heals you +10HP.");
        p.hp += 10;
    }
    else {
        return;
    }
    refresh();
    ncWait();
}

// ===== Map Display =====
#include <ncurses.h>

void displayMap() {
    clear();

    int termH, termW;
    getmaxyx(stdscr, termH, termW); 

    int mapH = SIZE + 2; 
    int mapW = SIZE * 2; 

    int startY = (termH - mapH) / 2;
    int startX = (termW - mapW) / 2;

    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    mvprintw(startY, startX, "[MAP]");
    mvprintw(startY + 1, startX, "=====================");

    int screenY = startY + 2;

    for (int i = 0; i < SIZE; i++) {

        int screenX = startX;
        move(screenY + i, screenX);

        for (int j = 0; j < SIZE; j++) {

            if (i == px && j == py) {
                printw("P ");
                discovered[i][j] = true;
            }
            else if (!discovered[i][j]) {
                if (grid[i][j] == '#') printw("# ");
                else printw("? ");
            }
            else {
                printw("%c ", grid[i][j]);
            }
        }
    }

    mvprintw(screenY + SIZE, startX, "=====================");

    refresh();
}

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
        napms(500);
        return; 
    }
    if (grid[px][py] == 'C') { 
        clear();
        centerPrint(getCenteredStartY(1), "Chest! +20 Gold");
        p.gold += 20; 
        grid[px][py] = '.';
        refresh();
        napms(500);
        return; 
    }
    if (grid[px][py] == 'B') { bossFight(p, bossMin, bossMax); grid[px][py] = '.'; return; }
    if (grid[px][py] == 'G') {
        if (!p.hasKey) {
            clear();
            centerPrint(getCenteredStartY(1), "You have not found the key yet! Cannot rescue princess.");
            refresh();
            napms(500);
        } else {
            princessRoomMinigame(p, false);
            grid[px][py] = '.';
        }
        return;
    }
    if (grid[px][py] == '.') event(p, monsterMin, monsterMax, bossMin, bossMax);
}

// ===== Main =====
int main() {
    //初始化 ncurses
    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, nullptr);
    mouseinterval(150);
    curs_set(0);  // Hide cursor
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);

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
    bool loadedFromSave = false;
    user_save_system::SaveData loadedData;
    if (user_save_system::hasSave(username) && user_save_system::loadProgress(username, loadedData)) {
        loadedFromSave = applySaveData(loadedData, p, monsterMin, monsterMax, bossMin, bossMax);
        clear();
        if (loadedFromSave) {
            centerPrint(getCenteredStartY(1), "Save found. Progress restored.");
        } else {
            centerPrint(getCenteredStartY(1), "Save file is invalid. Starting a new game.");
        }
        refresh();
        napms(800);
    }

    if (!loadedFromSave) {

        chooseDifficulty(monsterMin, monsterMax, bossMin, bossMax);
        clear();
        centerPrint(5, "Press T for Tutorial, any other key to skip");
        refresh();

        int ch = getch();

        if (ch == 'T' || ch == 't') {
            Player temp = p;
            tutorial(temp);
        }

        initializeNewMap();
    }

    user_save_system::saveProgress(username, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));

    while (true) {
        clear();
        cursorY = 0;
        
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

            p = Player();
            px = 0;
            py = 0;
            if (!showTitle()) {
                break;
            }
            showIntro();
            chooseDifficulty(monsterMin, monsterMax, bossMin, bossMax);
            initializeNewMap();
            user_save_system::saveProgress(username, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
            continue;
        }
        if (px == gx && py == gy && p.hasKey) { 
            clear();
            centerPrint(getCenteredStartY(1), "You rescued the princess! Victory!");
            showButton();
            refresh();
            ncWait();
            break; 
        }
        if (px == gx && py == gy && !p.hasKey) {
            centerPrint(cursorY++, "You have not found the key yet! Cannot rescue princess.");
        }

        centerPrint(cursorY++, "Move (W/A/S/D or Arrow Keys):");
        showButton();
        refresh();
        
        int key = readKeyWithWindowGuard();

        if (key == KEY_MOUSE) {
            MEVENT event;
            if (getmouse(&event) == OK && isPrimaryMouseClick(event)) {
                TopButtonAction action = getTopButtonActionFromMouse(event);
                if (action == TopButtonAction::Help) {
                    showHelp();
                    continue;
                }
                if (action == TopButtonAction::Home) {
                    p = Player();
                    px = 0;
                    py = 0;
                    if (!showTitle()) {
                        break;
                    }
                    showIntro();
                    chooseDifficulty(monsterMin, monsterMax, bossMin, bossMax);
                    initializeNewMap();
                    user_save_system::saveProgress(username, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
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
            user_save_system::saveProgress(username, buildSaveData(p, monsterMin, monsterMax, bossMin, bossMax));
        }
    }

    endwin();
    return 0;
}
