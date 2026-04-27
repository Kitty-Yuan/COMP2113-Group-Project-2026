#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <queue>
#include <limits>
#include <string>
using namespace std;

int SIZE; 
char grid[50][50]; 
bool visited[50][50];
bool discovered[50][50];
bool triggered[50][50];   // record if a normal cell has triggered event

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

// ===== Intro =====
void showIntro() {
    cout << "===== Knight Maze RPG =====\n";
    cout << "Press Enter to continue...\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "You are a brave knight, sent into a dangerous maze to rescue the princess.\n";
    cout << "Press Enter to continue...\n"; cin.get();
    cout << "In the maze, you will encounter traps, enemies, merchants, and a Boss.\n";
    cout << "Press Enter to continue...\n"; cin.get();
    cout << "You must find the key and rescue the princess!\n\n";
    cout << "Press Enter to continue...\n"; cin.get();
}

void showHelp() {
    cout << "\n[HELP]\n";
    cout << "- Movement: w a s d\n";
    cout << "- Battle: 1 Normal Attack, 2 Strong Attack, 3 Defend\n";
    cout << "  * Defend: 40% chance to block and counter (50% enemy dmg + 30% your ATK) and heal +5HP; 60% chance to take 30% enemy dmg.\n";
    cout << "- Merchant: Buy or rob (risk).\n";
    cout << "- Chest: Gold or potion.\n";
    cout << "- Level up: 100 EXP -> level up (HP+20 ATK+5 DEF+3)\n";
    cout << "- Victory: Find key and rescue princess.\n";
    cout << "=====================================\n\n";
}

// ===== Instructions =====
void showInstructions() {
    cout << "\n[GAME INSTRUCTIONS]\n";
    cout << "Welcome to Knight Maze RPG. Here are the rules:\n\n";
    cout << "1. Movement: Use w (up), a (left), s (down), d (right) to move through the maze.\n";
    cout << "   - Walls (#) are visible even in fog. You cannot walk through them.\n";
    cout << "   - Unexplored cells show '?', but walls are always shown as '#'.\n\n";
    cout << "2. Combat and Random Events:\n";
    cout << "   - Stepping on the Boss tile (B) triggers a fixed boss battle.\n";
    cout << "   - Stepping on an empty cell (.) for the first time triggers a random event:\n";
    cout << "        * 40% chance -> Fight a normal enemy.\n";
    cout << "        * 20% chance -> Meet a merchant (not fully implemented yet).\n";
    cout << "        * 15% chance -> Meet an old man who heals +10 HP.\n";
    cout << "        * 25% chance -> Nothing happens.\n";
    cout << "   - During any fight (normal enemy or boss), you can choose:\n";
    cout << "        1) Normal Attack\n";
    cout << "        2) Strong Attack (50% chance to deal extra damage)\n";
    cout << "        3) Defend: 40% chance to block completely, counter for (50% enemy dmg + 30% your ATK), and heal +5 HP; 60% chance to take 30% enemy dmg (no counter).\n";
    cout << "   - Defeating enemies gives gold and EXP. At 100 EXP, you level up (HP+20, ATK+5, DEF+3).\n\n";
    cout << "3. Special Tiles:\n";
    cout << "   - K: Key - Required to rescue the princess.\n";
    cout << "   - T: Trap - Instantly lose 15 HP.\n";
    cout << "   - C: Chest - Gain 20 gold.\n";
    cout << "   - G: Princess - Must have key to win. Leads to a chase minigame.\n\n";
    cout << "4. Revisiting a cell will not trigger any event again.\n\n";
    cout << "5. Difficulty: After the tutorial, choose difficulty (1-4) which affects map size and enemy strength.\n\n";
    cout << "6. Princess Chase Minigame:\n";
    cout << "   - When you step on the princess tile with the key, you enter a 15x15 maze.\n";
    cout << "   - You control 'P' (WASD), the princess 'G' moves randomly every 2 of your steps.\n";
    cout << "   - Walls '#' block movement. You have limited steps (depends on your HP).\n";
    cout << "   - Catch the princess to win the game. If you run out of steps, you lose all HP.\n\n";
    cout << "Goal: Find the key, defeat (or avoid) the boss, and rescue the princess in the chase minigame.\n";
    cout << "Good luck!\n";
    cout << "Press Enter to continue...\n";
    cin.get();
}

// ===== Catch princess (with guaranteed path connectivity) =====
void princessRoomMinigame(Player &p, bool isTrial) {
    const int roomSize = 15;
    char room[roomSize][roomSize];
    
    int maxSteps = isTrial ? 15 : (12 + (p.hp + 9) / 10);
    
    // Helper: BFS to check connectivity from (1,1) to (roomSize/2, roomSize/2)
    auto isConnected = [&]() -> bool {
        bool vis[15][15] = {false};
        queue<pair<int,int>> q;
        q.push({1, 1});
        vis[1][1] = true;
        int dx[] = {1, -1, 0, 0};
        int dy[] = {0, 0, 1, -1};
        while (!q.empty()) {
            auto [x, y] = q.front(); q.pop();
            if (x == roomSize/2 && y == roomSize/2) return true;
            for (int i = 0; i < 4; i++) {
                int nx = x + dx[i], ny = y + dy[i];
                if (nx >= 0 && nx < roomSize && ny >= 0 && ny < roomSize && !vis[nx][ny] && room[nx][ny] != '#') {
                    vis[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }
        return false;
    };
    
    // Generate maze until connected
    bool connected = false;
    int attempts = 0;
    while (!connected && attempts < 50) {
        attempts++;
        // Generate walls
        for (int i = 0; i < roomSize; i++) {
            for (int j = 0; j < roomSize; j++) {
                if (i == 0 || i == roomSize - 1 || j == 0 || j == roomSize - 1) {
                    room[i][j] = '#';
                } else {
                    bool isWall = (rand() % 100 < 30);
                    // Force open paths: center cross and diagonals
                    if (i == roomSize/2 || j == roomSize/2 || i == j || i == (roomSize-1-j)) {
                        isWall = false;
                    }
                    room[i][j] = isWall ? '#' : '.';
                }
            }
        }
        // Ensure start and princess positions are not walls
        room[1][1] = '.';
        room[roomSize/2][roomSize/2] = '.';
        // Check connectivity
        connected = isConnected();
    }
    
    if (!connected) {
        // Fallback: create a simple open path (clear all walls on the central row and column)
        for (int i = 0; i < roomSize; i++) {
            for (int j = 0; j < roomSize; j++) {
                if (i == 0 || i == roomSize-1 || j == 0 || j == roomSize-1)
                    room[i][j] = '#';
                else if (i == 1 || i == roomSize-2 || j == 1 || j == roomSize-2)
                    room[i][j] = '.';  // create a border corridor
                else if (rand() % 100 < 30)
                    room[i][j] = '#';
                else
                    room[i][j] = '.';
            }
        }
        // Guarantee start and goal are open
        room[1][1] = '.';
        room[roomSize/2][roomSize/2] = '.';
        // Simple path: horizontal line from start to goal's row, then vertical
        for (int j = 1; j <= roomSize/2; j++) room[1][j] = '.';
        for (int i = 1; i <= roomSize/2; i++) room[i][roomSize/2] = '.';
    }
    
    // Positions
    int rpx = 1, rpy = 1;
    int rgx = roomSize/2, rgy = roomSize/2;
    int stepsUsed = 0;
    
    while (true) {
        cout << "\n--- PRINCESS CHASE ---  Steps Left: " << (maxSteps - stepsUsed) << "\n";
        for (int i = 0; i < roomSize; i++) {
            for (int j = 0; j < roomSize; j++) {
                if (i == rpx && j == rpy) cout << "P ";
                else if (i == rgx && j == rgy) cout << "G ";
                else cout << room[i][j] << " ";
            }
            cout << endl;
        }
        
        if (rpx == rgx && rpy == rgy) {
            cout << "\n[VICTORY] You caught the princess!\n";
            return;
        }
        if (stepsUsed >= maxSteps) {
            cout << "\n[FAILED] The princess ran away...\n";
            p.hp = 0;
            return;
        }
        
        char m;
        cout << "Move (WASD): ";
        cin >> m;
        int nx = rpx, ny = rpy;
        m = tolower(m);
        if (m == 'w') nx--;
        else if (m == 's') nx++;
        else if (m == 'a') ny--;
        else if (m == 'd') ny++;
        
        if (nx > 0 && nx < roomSize-1 && ny > 0 && ny < roomSize-1 && room[nx][ny] != '#') {
            rpx = nx;
            rpy = ny;
            stepsUsed++;
        }
        
        // Princess moves every 2 steps
        if (stepsUsed > 0 && stepsUsed % 2 == 0) {
            int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
            int dir = rand() % 4;
            int ngx = rgx + dx[dir], ngy = rgy + dy[dir];
            if (ngx > 0 && ngx < roomSize-1 && ngy > 0 && ngy < roomSize-1 && room[ngx][ngy] == '.') {
                rgx = ngx;
                rgy = ngy;
            }
        }
    }
}

// ===== Difficulty =====
void chooseDifficulty(int &enemyMin, int &enemyMax, int &bossMin, int &bossMax) {
    int diff;
    cout << "Choose difficulty (1-4):\n";
    cout << "1. Easy (9x9 map, enemy atk 5-10, boss atk 10-15)\n";
    cout << "2. Normal (12x12 map, enemy atk 8-12, boss atk 12-18)\n";
    cout << "3. Hard (15x15 map, enemy atk 10-15, boss atk 15-22)\n";
    cout << "4. Hell (20x20 map, enemy atk 11-16, boss atk 17-23)\n";
    cin >> diff;
    if (diff == 0) {
        Player tempP; 
        tempP.hasKey = true; 
        princessRoomMinigame(tempP, true); 
        exit(0); 
    }
    if (diff == 1) { SIZE = 9; enemyMin=5; enemyMax=10; bossMin=10; bossMax=15; }
    else if (diff == 2) { SIZE = 12; enemyMin=8; enemyMax=12; bossMin=12; bossMax=18; }
    else if (diff == 3) { SIZE = 15; enemyMin=10; enemyMax=15; bossMin=15; bossMax=22; }
    else { SIZE = 20; enemyMin=11; enemyMax=16; bossMin=17; bossMax=23; }

    gx = SIZE - 1; gy = SIZE - 1;
    cout << "You chose level " << diff << "! Map size " << SIZE << "x" << SIZE << "\n";
}

// ===== Level Up =====
void levelUp(Player &p) {
    while (p.exp >= 100) {
        p.exp -= 100;
        p.level++;
        p.hp += 20;
        p.atk += 5;
        p.def += 3;
        cout << "[LEVEL UP] Lv " << p.level << " HP+20 ATK+5 DEF+3\n";
    }
}

// ===== Tutorial (uses its own Player instance, does not affect real game) =====
void tutorial(Player &p) {
    cout << "\n===== Tutorial =====\n";

    char demoMap[5][5] = {
        {'P','.','#','.','.'},
        {'.','#','K','.','.'},
        {'.','.','B','#','.'},
        {'#','.','.','.','.'},
        {'.','.','#','.','G'}
    };

    bool hasKey = false;
    int x=0,y=0;

    while (true) {
        cout << "\n[Tutorial Map]\n";
        for (int i=0;i<5;i++) {
            for (int j=0;j<5;j++) {
                if (i==x && j==y) cout << "P ";
                else cout << demoMap[i][j] << " ";
            }
            cout << endl;
        }

        cout << "\nHP=" << p.hp << " ATK=" << p.atk << " DEF=" << p.def
             << " GOLD=" << p.gold << " EXP=" << p.exp << " LV=" << p.level
             << " KEY=" << (hasKey ? "Y" : "N") << "\n";

        cout << "\nMove (w a s d): ";
        char m; cin >> m;
        if (cin.fail() || (m!='w'&&m!='a'&&m!='s'&&m!='d')) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Type help for commands.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int nx=x, ny=y;
        if (m=='w') nx--; else if (m=='s') nx++;
        else if (m=='a') ny--; else if (m=='d') ny++;
        if (nx<0||nx>=5||ny<0||ny>=5||demoMap[nx][ny]=='#') {
            cout << "[BLOCKED] Cannot go there.\n"; continue;
        }

        x=nx; y=ny;
        cout << "\n--> Entered new cell.\n";

        if (demoMap[x][y]=='K') {
            cout << "[KEY] You found the key!\n";
            hasKey=true; demoMap[x][y]='.';
        }
        else if (demoMap[x][y]=='B') {
            cout << "[ENEMY] Combat starts!\n";
            int enemyHP=20;
            int minPower=5,maxPower=7;
            while (enemyHP>0 && p.hp>0) {
                int choice;
                cout << "\nAction: 1 Normal 2 Strong 3 Defend\n";
                cin >> choice;
                if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
                    cin.clear(); cin.ignore(10000,'\n');
                    cout << "Invalid input! Type help.\n";
                    string cmd; cin >> cmd;
                    if (cmd=="help") showHelp();
                    continue;
                }
                int playerAttack=0;
                if (choice==1) playerAttack=rand()%p.atk+1;
                else if (choice==2 && rand()%100<50) playerAttack=p.atk+rand()%5;
                else if (choice==3) {
                    int rawDmg = (rand()%(maxPower-minPower+1))+minPower;
                    if (rand()%100 < 40) {
                        int counter = static_cast<int>(rawDmg * 0.5 + p.atk * 0.3);
                        if (counter < 1) counter = 1;
                        enemyHP -= counter;
                        p.hp += 5;
                        cout << "[DEFEND SUCCESS] Blocked, healed +5HP, counter " << counter << " damage!\n";
                    } else {
                        int dmgTaken = static_cast<int>(rawDmg * 0.3);
                        if (dmgTaken < 1) dmgTaken = 1;
                        p.hp -= dmgTaken;
                        cout << "[DEFEND FAIL] Took " << dmgTaken << " damage.\n";
                    }
                }
                if (choice==1 || choice==2) {
                    enemyHP -= playerAttack;
                    if(playerAttack>0) cout << "You dealt " << playerAttack << " damage!\n";
                }
                if(enemyHP<=0){cout<<"[VICTORY] Enemy defeated! +10 Gold, +20 EXP\n"; p.gold+=10; p.exp+=20; break;}
                if (choice!=3) {
                    int edmg=(rand()%(maxPower-minPower+1))+minPower;
                    p.hp-=edmg;
                    cout<<"Enemy dealt "<<edmg<<" damage!\n";
                }
            }
            demoMap[x][y]='.';
        }
        else if (demoMap[x][y]=='G') {
            if (!hasKey) {
                cout << "[NO KEY] You cannot rescue the princess without the key.\n";
                continue;
            } else {
                cout << "[SUCCESS] You rescued the princess! Tutorial complete!\n";
                break;
            }
        }
    }

    // After tutorial, offer to try the final chase minigame
    cout << "\nPress 0 to try the final chase minigame now, or press Enter to continue to difficulty selection.\n";
    string choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, choice);
    if (choice == "0") {
        Player demoPlayer;
        demoPlayer.hasKey = true; // give key for demo
        princessRoomMinigame(demoPlayer, false);
        cout << "\nChase minigame finished. Press Enter to continue to difficulty selection.\n";
        cin.get();
    }
}

// ===== Battle System =====
void fight(Player &p, int enemyMin, int enemyMax) {
    cout << "\n[ENEMY] You encountered an enemy!\n";
    int enemyHP = 30 + rand() % 20;
    cout << "Enemy attack range: " << enemyMin << " - " << enemyMax << "\n";

    while (enemyHP > 0 && p.hp > 0) {
        int choice;
        cout << "\nAction: 1 Normal 2 Strong 3 Defend\n";
        cin >> choice;
        if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int playerAttack = 0;
        if (choice == 1) playerAttack = rand() % p.atk + 1;
        else if (choice == 2 && rand() % 100 < 50) playerAttack = p.atk + rand() % 5;

        int rawDmg = (rand() % (enemyMax - enemyMin + 1)) + enemyMin;

        if (choice == 1 || choice == 2) {
            enemyHP -= playerAttack;
            if (playerAttack > 0) cout << "You dealt " << playerAttack << " damage!\n";
            if (enemyHP <= 0) {
                cout << "[VICTORY] Enemy defeated! +20 Gold, +50 EXP\n";
                p.gold += 20; p.exp += 50; levelUp(p);
                break;
            }
            int dmg = rawDmg;
            if (dmg < 1) dmg = 1;
            p.hp -= dmg;
            cout << "Enemy dealt " << dmg << " damage!\n";
        }
        else if (choice == 3) {
            if (rand() % 100 < 40) {
                int counter = static_cast<int>(rawDmg * 0.5 + p.atk * 0.3);
                if (counter < 1) counter = 1;
                enemyHP -= counter;
                p.hp += 5;
                cout << "[DEFEND SUCCESS] Blocked, healed +5HP, counter " << counter << " damage!\n";
                if (enemyHP <= 0) {
                    cout << "[VICTORY] Enemy defeated! +20 Gold, +50 EXP\n";
                    p.gold += 20; p.exp += 50; levelUp(p);
                    break;
                }
            } else {
                int dmgTaken = static_cast<int>(rawDmg * 0.3);
                if (dmgTaken < 1) dmgTaken = 1;
                p.hp -= dmgTaken;
                cout << "[DEFEND FAIL] Took " << dmgTaken << " damage.\n";
            }
        }
    }
}

// ===== Boss Fight =====
void bossFight(Player &p, int bossMin, int bossMax) {
    cout << "\n[BOSS] Boss battle begins!\n";
    int bossHP = 100;
    cout << "Boss attack range: " << bossMin << " - " << bossMax << "\n";

    while (bossHP > 0 && p.hp > 0) {
        int choice;
        cout << "\nAction: 1 Normal 2 Strong 3 Defend\n";
        cin >> choice;
        if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int playerAttack = 0;
        if (choice == 1) playerAttack = rand() % p.atk + 1;
        else if (choice == 2 && rand() % 100 < 50) playerAttack = p.atk + rand() % 10;

        int rawDmg = (rand() % (bossMax - bossMin + 1)) + bossMin;

        if (choice == 1 || choice == 2) {
            bossHP -= playerAttack;
            if (playerAttack > 0) cout << "You dealt " << playerAttack << " damage!\n";
            if (bossHP <= 0) {
                cout << "[VICTORY] Boss defeated! +100 Gold, +200 EXP\n";
                p.gold += 100; p.exp += 200; levelUp(p);
                break;
            }
            int dmg = rawDmg;
            if (dmg < 1) dmg = 1;
            p.hp -= dmg;
            cout << "Boss dealt " << dmg << " damage!\n";
        }
        else if (choice == 3) {
            if (rand() % 100 < 40) {
                int counter = static_cast<int>(rawDmg * 0.5 + p.atk * 0.3);
                if (counter < 1) counter = 1;
                bossHP -= counter;
                p.hp += 5;
                cout << "[DEFEND SUCCESS] Blocked, healed +5HP, counter " << counter << " damage!\n";
                if (bossHP <= 0) {
                    cout << "[VICTORY] Boss defeated! +100 Gold, +200 EXP\n";
                    p.gold += 100; p.exp += 200; levelUp(p);
                    break;
                }
            } else {
                int dmgTaken = static_cast<int>(rawDmg * 0.3);
                if (dmgTaken < 1) dmgTaken = 1;
                p.hp -= dmgTaken;
                cout << "[DEFEND FAIL] Took " << dmgTaken << " damage.\n";
            }
        }
    }
}

// ===== Event System =====
void event(Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
    int r = rand() % 100;
    if (r < 40) fight(p, enemyMin, enemyMax);
    else if (r < 60) { cout << "[MERCHANT] You met a merchant! (Shop not implemented)\n"; }
    else if (r < 75) { cout << "[OLD MAN] He heals you +10HP.\n"; p.hp += 10; }
    else cout << "[QUIET] Nothing happens.\n";
}

// ===== Map Display =====
void displayMap() {
    cout << "\n[MAP]\n";
    cout << "=====================\n";
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (i == px && j == py) {
                cout << "P ";
                discovered[i][j] = true;
            }
            else if (!discovered[i][j]) {
                if (grid[i][j] == '#') cout << "# ";
                else cout << "? ";
            }
            else cout << grid[i][j] << " ";
        }
        cout << endl;
    }
    cout << "=====================\n";
}

// ===== Movement =====
void movePlayer(char m, Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
    int nx = px, ny = py;
    if (m == 'w') nx--;
    else if (m == 's') nx++;
    else if (m == 'a') ny--;
    else if (m == 'd') ny++;

    if (nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) return;
    if (grid[nx][ny] == '#') { cout << "[BLOCKED] Wall!\n"; return; }

    px = nx; py = ny;
    discovered[px][py] = true;
    cout << "\n--> Entered new cell.\n";

    if (grid[px][py] == 'K') { cout << "[KEY] You found the key!\n"; p.hasKey = true; grid[px][py] = '.'; triggered[px][py] = true; return; }
    if (grid[px][py] == 'T') { cout << "[TRAP] HP -15\n"; p.hp -= 15; grid[px][py] = '.'; triggered[px][py] = true; return; }
    if (grid[px][py] == 'C') { cout << "[CHEST] +20 Gold\n"; p.gold += 20; grid[px][py] = '.'; triggered[px][py] = true; return; }
    if (grid[px][py] == 'B') { bossFight(p, bossMin, bossMax); grid[px][py] = '.'; triggered[px][py] = true; return; }
    if (grid[px][py] == 'G') {
        if (!p.hasKey) {
            cout << "[NO KEY] Cannot rescue princess without key.\n";
        } else {
            princessRoomMinigame(p, false);
            grid[px][py] = '.';
            triggered[px][py] = true;
        }
        return;
    }
    if (grid[px][py] == '.') {
        if (!triggered[px][py]) {
            event(p, enemyMin, enemyMax, bossMin, bossMax);
            triggered[px][py] = true;
        } else {
            cout << "[ALREADY VISITED] Nothing happens.\n";
        }
    }
}

// ===== Helper: BFS to check connectivity and get path =====
bool isConnected(int sx, int sy, int tx, int ty, vector<pair<int,int>> &path) {
    bool vis[50][50] = {false};
    pair<int,int> parent[50][50];
    queue<pair<int,int>> q;
    q.push({sx, sy});
    vis[sx][sy] = true;
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        if (x == tx && y == ty) {
            path.clear();
            int cx = tx, cy = ty;
            while (!(cx == sx && cy == sy)) {
                path.push_back({cx, cy});
                auto [px, py] = parent[cx][cy];
                cx = px; cy = py;
            }
            path.push_back({sx, sy});
            reverse(path.begin(), path.end());
            return true;
        }
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i], ny = y + dy[i];
            if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && !vis[nx][ny] && grid[nx][ny] != '#') {
                vis[nx][ny] = true;
                parent[nx][ny] = {x, y};
                q.push({nx, ny});
            }
        }
    }
    return false;
}

// ===== Main =====
int main() {
    srand(time(0));
    showIntro();

    // Prompt for instructions before tutorial
    cout << "Type 'instruction' to see detailed game rules, or press Enter to start the tutorial.\n";
    string input;
    getline(cin, input);
    if (input == "instruction") {
        showInstructions();
    }

    // Tutorial with a temporary player (does not affect real game)
    Player tutorialPlayer;
    tutorial(tutorialPlayer);

    // Real game player with fresh stats
    Player p;
    int enemyMin, enemyMax, bossMin, bossMax;
    chooseDifficulty(enemyMin, enemyMax, bossMin, bossMax);

    // ========== Generate maze with guaranteed path and fixed boss position ==========
    bool mapValid = false;
    int maxAttempts = 100;
    for (int attempt = 0; attempt < maxAttempts && !mapValid; attempt++) {
        // 1. Initialize all cells as empty
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                grid[i][j] = '.';

        // 2. Place random walls (25% chance), but not on start or goal
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if ((i == 0 && j == 0) || (i == gx && j == gy)) continue;
                if (rand() % 100 < 25) grid[i][j] = '#';
            }
        }

        // 3. Check connectivity from start to goal
        vector<pair<int,int>> path;
        if (isConnected(0, 0, gx, gy, path)) {
            // 4. Place Boss at adjacent cell above or left of goal
            int bx = -1, by = -1;
            // Try above first
            if (gx - 1 >= 0 && grid[gx-1][gy] == '.') {
                bx = gx - 1; by = gy;
            } 
            // Then try left
            else if (gy - 1 >= 0 && grid[gx][gy-1] == '.') {
                bx = gx; by = gy - 1;
            }
            // If both are blocked (e.g., walls), force one to be empty
            if (bx == -1) {
                if (gx - 1 >= 0) {
                    bx = gx - 1; by = gy;
                    grid[bx][by] = '.'; // remove wall
                } else if (gy - 1 >= 0) {
                    bx = gx; by = gy - 1;
                    grid[bx][by] = '.';
                }
            }
            grid[bx][by] = 'B';

            // 5. Place key on the path (avoid boss cell)
            int keyIdx;
            do {
                keyIdx = rand() % (path.size() - 2) + 1; // exclude start and goal
            } while (path[keyIdx].first == bx && path[keyIdx].second == by);
            int kx = path[keyIdx].first, ky = path[keyIdx].second;
            grid[kx][ky] = 'K';

            // 6. Place traps and chests (3 each) on random empty cells (not on key, boss, goal, start)
            int trapCount = 3, chestCount = 3;
            for (int t = 0; t < trapCount; t++) {
                int tx, ty;
                do {
                    tx = rand() % SIZE;
                    ty = rand() % SIZE;
                } while (grid[tx][ty] != '.');
                grid[tx][ty] = 'T';
            }
            for (int c = 0; c < chestCount; c++) {
                int cx, cy;
                do {
                    cx = rand() % SIZE;
                    cy = rand() % SIZE;
                } while (grid[cx][cy] != '.');
                grid[cx][cy] = 'C';
            }

            // 7. Ensure goal is princess
            grid[gx][gy] = 'G';

            mapValid = true;
        }
    }

    if (!mapValid) {
        cout << "Map generation failed! Using fallback map.\n";
        for (int i = 0; i < SIZE; i++)
            for (int j = 0; j < SIZE; j++)
                grid[i][j] = '.';
        grid[gx][gy] = 'G';
        // Place boss above or left
        if (gx-1 >= 0) grid[gx-1][gy] = 'B';
        else if (gy-1 >= 0) grid[gx][gy-1] = 'B';
        else grid[SIZE/2][SIZE/2] = 'B';
        // Place key somewhere on a simple path
        int midX = SIZE/2, midY = SIZE/2;
        grid[midX][midY] = 'K';
        grid[rand()%SIZE][rand()%SIZE] = 'T';
        grid[rand()%SIZE][rand()%SIZE] = 'C';
    }

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            visited[i][j] = false;
            discovered[i][j] = false;
            triggered[i][j] = false;
        }
    discovered[0][0] = true;
    triggered[0][0] = true;

    // ========== Main game loop ==========
    while (true) {
        displayMap();

        cout << "\nHP=" << p.hp << " ATK=" << p.atk << " DEF=" << p.def
             << " GOLD=" << p.gold << " EXP=" << p.exp << " LV=" << p.level
             << " KEY=" << (p.hasKey ? "Y" : "N") << "\n";

        if (p.hp <= 0) { 
            cout << "[GAME OVER] You died.\n"; 
            break; 
        }
        if (px == gx && py == gy && p.hasKey) { 
            cout << "[VICTORY] You rescued the princess! Congratulations!\n"; 
            break; 
        }
        if (px == gx && py == gy && !p.hasKey) {
            cout << "[NO KEY] You need the key to rescue the princess.\n";
        }

        char m;
        cout << "\nMove WASD: ";
        cin >> m;
        if (cin.fail() || (m!='w'&&m!='a'&&m!='s'&&m!='d')) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
        } else movePlayer(m, p, enemyMin, enemyMax, bossMin, bossMax);
    }

    cout << "\n===== Game Over =====\n";
    cout << "Final Stats: HP=" << p.hp << " ATK=" << p.atk << " DEF=" << p.def
         << " GOLD=" << p.gold << " EXP=" << p.exp << " LV=" << p.level
         << " KEY=" << (p.hasKey ? "Y" : "N") << "\n";
    return 0;
}