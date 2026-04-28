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

user_save_system::SaveData buildSaveData(const Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
    user_save_system::SaveData data;
    data.valid = true;
    data.size = SIZE;
    data.px = px;
    data.py = py;
    data.gx = gx;
    data.gy = gy;
    data.enemyMin = enemyMin;
    data.enemyMax = enemyMax;
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

bool applySaveData(const user_save_system::SaveData &data, Player &p, int &enemyMin, int &enemyMax, int &bossMin, int &bossMax) {
    if (!data.valid || data.size <= 0 || data.size > 50) {
        return false;
    }

    SIZE = data.size;
    px = data.px;
    py = data.py;
    gx = data.gx;
    gy = data.gy;

    enemyMin = data.enemyMin;
    enemyMax = data.enemyMax;
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
        int maxY, maxX;
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
void chooseDifficulty(int &enemyMin, int &enemyMax, int &bossMin, int &bossMax) {
    int diff = 0;
    
    while (diff < 1 || diff > 4) {
        clear();
        vector<string> lines = {
            "Choose difficulty (1-4):",
            "1. Easy (9x9 map, enemy atk 5-10, boss atk 10-15)",
            "2. Normal (12x12 map, enemy atk 8-12, boss atk 12-18)",
            "3. Hard (15x15 map, enemy atk 10-15, boss atk 15-22)",
            "4. Hell (20x20 map, enemy atk 11-16, boss atk 15-25)",
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
    
    if (diff == 1) { SIZE = 9; enemyMin=5; enemyMax=10; bossMin=10; bossMax=15; }
    else if (diff == 2) { SIZE = 12; enemyMin=8; enemyMax=12; bossMin=12; bossMax=18; }
    else if (diff == 3) { SIZE = 15; enemyMin=10; enemyMax=15; bossMin=15; bossMax=22; }
    else { SIZE = 20; enemyMin=12; enemyMax=18; bossMin=18; bossMax=25; }

    gx = SIZE - 1; gy = SIZE - 1;
    clear();
    string result = "You chose Lv" + to_string(diff) + "! Map size " + to_string(SIZE) + "x" + to_string(SIZE);
    centerPrint(getCenteredStartY(1), result);
    refresh();
    ncWait();
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

// ===== Tutorial =====
void tutorial(Player &p) {
    clear();
    centerPrint(getCenteredStartY(1), "===== TUTORIAL =====");
    refresh();
    ncWait();

    char demoMap[5][5] = {
        {'P','.','#','.','.'},
        {'.','#','K','.','.'},
        {'.','.','B','#','.'},
        {'#','.','.','.','.'},
        {'.','.','#','.','G'}
    };

    bool hasKey = false;
    int x=0, y_pos=0;

    while (true) {
        clear();
        int y = 0;
        int mapWidth = 10;
        int blockHeight = 10;
        int startY = getCenteredStartY(blockHeight + 3);
        int maxY, maxX;
        getmaxyx(stdscr, maxY, maxX);
        int startX = max(0, (maxX - mapWidth) / 2);
        
        // Print map
        centerPrint(startY + y++, "Tutorial Map:");
        for (int i=0;i<5;i++) {
            string row;
            for (int j=0;j<5;j++) {
                if (i==x && j==y_pos) row += 'P';
                else row += demoMap[i][j];
                row += ' ';
            }
            mvprintw(startY + y, startX, "%s", row.c_str());
            y++;
        }

        // Show stats
        string stats = "HP=" + to_string(p.hp) +
                       " ATK=" + to_string(p.atk) +
                       " DEF=" + to_string(p.def) +
                       " GOLD=" + to_string(p.gold) +
                       " EXP=" + to_string(p.exp) +
                       " LV=" + to_string(p.level) +
                       " KEY=" + (hasKey ? string("Y") : string("N"));
        centerPrint(startY + y++, stats);
        

        show_ATT(p.hp, 100, "HP", 5, 5);
        show_ATT(p.atk, 50, "ATK", 7, 5);
        show_ATT(p.def, 30, "DEF", 9, 5);
        refresh();  
        
        // Input move
        centerPrint(startY + y++, "Move (W/A/S/D or Arrow Keys):");
        refresh();
        
        int key = readKeyWithWindowGuard();
        char m = normalizeMoveKey(key);
        if (m == '\0') {
            continue;
        }

        int nx=x, ny=y_pos;
        if (m=='w') nx--; else if (m=='s') nx++;
        else if (m=='a') ny--; else if (m=='d') ny++;
        
        if (nx<0||nx>=5||ny<0||ny>=5||demoMap[nx][ny]=='#') {
            continue;
        }

        x=nx; y_pos=ny;

        if (demoMap[x][y_pos]=='K') {
            clear();
            centerPrint(getCenteredStartY(1), "You found the key!");
            ncWait();
            hasKey=true; 
            demoMap[x][y_pos]='.';
            refresh();
            napms(500);
        }
        else if (demoMap[x][y_pos]=='B') {
            clear();
            centerPrint(getCenteredStartY(1), "Enemy encountered!");
            refresh();
            napms(500);
            
            int enemyHP=20;
            int minPower=5,maxPower=7;
            while (enemyHP>0 && p.hp>0) {
                clear();
                y = 0;
                mvprintw(y++, 0, "BATTLE - Your HP: %d | Enemy HP: %d", p.hp, enemyHP);
                mvprintw(y++, 0, "Choose: 1) Normal  2) Strong  3) Defend");
                refresh();
                
                int choice = readKeyWithWindowGuard();
                
                int playerAttack=0;
               int defendSuccess=0;
                if (choice=='1') playerAttack=rand()%p.atk+1;
                else if (choice=='2' && rand()%100<60) playerAttack=(int)(p.atk*(130+rand()%40)/100.0);
                else if (choice=='3') defendSuccess=(rand()%100<40)?1:0;
                
                enemyHP-=playerAttack;
                
                clear();
                y = 0;
                if(playerAttack>0) mvprintw(y++, 0, "You dealt %d damage!", playerAttack);
                if(enemyHP<=0){
                    mvprintw(y++, 0, "Enemy defeated!");
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
                        enemyHP-=counterDmg;
                        p.hp+=5;
                        edmg=0;
                        mvprintw(y++, 0, "Defend success! Counter: %d damage!", counterDmg);
                        if(enemyHP<=0) {
                            mvprintw(y++, 0, "Enemy defeated!");
                            p.gold+=10; p.exp+=20; refresh(); napms(500); break;
                        }
                    } else {
                        edmg=(int)(edmg*0.4);
                    }
                }
                if(edmg<1) edmg=1;
                p.hp-=edmg;
                mvprintw(y++, 0, "Enemy dealt %d damage!", edmg);
                refresh();
                napms(500);
            }
            demoMap[x][y_pos]='.';
        }
        else if (demoMap[x][y_pos]=='G') {
            clear();
            if (!hasKey) {
                centerPrint(getCenteredStartY(1), "You have not found the key yet! Cannot rescue princess.");
                refresh();
                napms(500);
                continue;
            } else {
                centerPrint(getCenteredStartY(1), "You rescued the princess! Tutorial complete!");
                refresh();
                ncWait();
                break;
            }
        }
    }
}

// ===== Battle System =====
void fight(Player &p, int enemyMin, int enemyMax) {
    int y = getCenteredStartY(4);
    clear();
    centerPrint(y++, "You encountered an enemy!");
    centerPrint(y++, "Enemy attack range: " + to_string(enemyMin) + " - " + to_string(enemyMax));
    refresh();
    napms(1000);
    
    int enemyHP = 30 + rand() % 20;

    while (enemyHP > 0 && p.hp > 0) {
        clear();
        y = getCenteredStartY(4);
        centerPrint(y++, "BATTLE - Your HP: " + to_string(p.hp) + " | Enemy HP: " + to_string(enemyHP));
        centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
        refresh();
        
        int choice = readKeyWithWindowGuard();
        
        int playerAttack = 0;
        int defendSuccess = 0;
        if (choice == '1') playerAttack = rand() % p.atk + 1;
        else if (choice == '2' && rand() % 100 < 60) playerAttack = (int)(p.atk * (130 + rand() % 40) / 100.0);
        else if (choice == '3') defendSuccess = (rand() % 100 < 40) ? 1 : 0;

        enemyHP -= playerAttack;
        
        clear();
        y = getCenteredStartY(4);
        if (playerAttack > 0) centerPrint(y++, "You dealt " + to_string(playerAttack) + " damage!");
        else if (choice == '1' || choice == '2') centerPrint(y++, "Attack missed!");

        if (enemyHP <= 0) {
            centerPrint(y++, "Enemy defeated! +20 Gold, +50 EXP");
            p.gold += 20; p.exp += 50; levelUp(p);
            refresh();
            ncWait();
            break;
        }

        int dmg = (rand() % (enemyMax - enemyMin + 1)) + enemyMin;
        if (choice == '3') {
            if (defendSuccess) {
                int counterDmg = (int)(p.atk * 0.4 + dmg * (0.4 + rand() % 20 / 100.0));
                enemyHP -= counterDmg;
                p.hp += 5;
                dmg = 0;
                centerPrint(y++, "Defend successful! Counter attack: " + to_string(counterDmg) + " damage!");
                if (enemyHP <= 0) {
                    centerPrint(y++, "Enemy defeated!");
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
        centerPrint(y++, "Enemy dealt " + to_string(dmg) + " damage!");
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
        y = getCenteredStartY(4);
        centerPrint(y++, "BOSS BATTLE - Your HP: " + to_string(p.hp) + " | Boss HP: " + to_string(bossHP));
        centerPrint(y++, "Choose: 1) Normal  2) Strong  3) Defend");
        refresh();
        
        int choice = readKeyWithWindowGuard();
        
        int playerAttack = 0;
        int defendSuccess = 0;
        if (choice == '1') playerAttack = rand() % p.atk + 1;
        else if (choice == '2' && rand() % 100 < 60) playerAttack = (int)(p.atk * (130 + rand() % 40) / 100.0);
        else if (choice == '3') defendSuccess = (rand() % 100 < 40) ? 1 : 0;

        bossHP -= playerAttack;
        
        clear();
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

// ===== Event System =====
void event(Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
    int r = rand() % 100;
    if (r < 40) fight(p, enemyMin, enemyMax);       // Enemy 40%
    else if (r < 60) {
        clear();
        centerPrint(getCenteredStartY(1), "You met a merchant! (Shop not implemented)");
    }
    else if (r < 75) {
        clear();
        centerPrint(getCenteredStartY(1), "You met an old man. He heals you +10HP.");
        p.hp += 10;
    }
    else {
        clear();
        centerPrint(getCenteredStartY(1), "Quiet area. Nothing happens.");
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
void movePlayer(char m, Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
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
    if (grid[px][py] == '.') event(p, enemyMin, enemyMax, bossMin, bossMax);
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

    int enemyMin, enemyMax, bossMin, bossMax;
    bool loadedFromSave = false;
    user_save_system::SaveData loadedData;
    if (user_save_system::hasSave(username) && user_save_system::loadProgress(username, loadedData)) {
        loadedFromSave = applySaveData(loadedData, p, enemyMin, enemyMax, bossMin, bossMax);
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
        tutorial(p);
        chooseDifficulty(enemyMin, enemyMax, bossMin, bossMax);
        initializeNewMap();
    }

    user_save_system::saveProgress(username, buildSaveData(p, enemyMin, enemyMax, bossMin, bossMax));

    while (true) {
        clear();
        cursorY = 0;
        
        // Display map
        displayMap();
        
        // Display stats
        string stats = "HP=" + to_string(p.hp) +
                       " ATK=" + to_string(p.atk) +
                       " DEF=" + to_string(p.def) +
                       " GOLD=" + to_string(p.gold) +
                       " EXP=" + to_string(p.exp) +
                       " LV=" + to_string(p.level) +
                       " KEY=" + (p.hasKey ? string("Y") : string("N"));
        centerPrint(cursorY++, stats);
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
            chooseDifficulty(enemyMin, enemyMax, bossMin, bossMax);
            initializeNewMap();
            user_save_system::saveProgress(username, buildSaveData(p, enemyMin, enemyMax, bossMin, bossMax));
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
                    chooseDifficulty(enemyMin, enemyMax, bossMin, bossMax);
                    initializeNewMap();
                    user_save_system::saveProgress(username, buildSaveData(p, enemyMin, enemyMax, bossMin, bossMax));
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
            movePlayer(m, p, enemyMin, enemyMax, bossMin, bossMax);
            user_save_system::saveProgress(username, buildSaveData(p, enemyMin, enemyMax, bossMin, bossMax));
        }
    }

    endwin();
    return 0;
}
