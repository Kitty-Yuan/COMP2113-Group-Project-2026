#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <limits>
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
    cout << "\n📖 Help:\n";
    cout << "- Movement: Enter w a s d to move.\n";
    cout << "- Battle: Choose 1 Normal Attack, 2 Strong Attack, 3 Defend.\n";
    cout << "- Merchant: Buy or rob (risk).\n";
    cout << "- Chest: May contain gold, potion, or monster.\n";
    cout << "- Level up: EXP reaches 100 → auto level up.\n";
    cout << "- Victory: Find the key and rescue the princess.\n";
    cout << "=====================================\n\n";
}

// ===== Difficulty =====
void chooseDifficulty(int &enemyMin, int &enemyMax, int &bossMin, int &bossMax) {
    int diff;
    cout << "Choose difficulty (1-4):\n";
    cout << "1. Easy (9x9 map, enemy atk 5-10, boss atk 10-15)\n";
    cout << "2. Normal (12x12 map, enemy atk 8-12, boss atk 12-18)\n";
    cout << "3. Hard (15x15 map, enemy atk 10-15, boss atk 15-22)\n";
    cout << "4. Hell (20x20 map, enemy atk 12-18, boss atk 18-25)\n";
    cin >> diff;

    if (diff == 1) { SIZE = 9; enemyMin=5; enemyMax=10; bossMin=10; bossMax=15; }
    else if (diff == 2) { SIZE = 12; enemyMin=8; enemyMax=12; bossMin=12; bossMax=18; }
    else if (diff == 3) { SIZE = 15; enemyMin=10; enemyMax=15; bossMin=15; bossMax=22; }
    else { SIZE = 20; enemyMin=12; enemyMax=18; bossMin=18; bossMax=25; }

    gx = SIZE - 1; gy = SIZE - 1;
    cout << "You chose Lv" << diff << "! Map size " << SIZE << "x" << SIZE << "\n";
}

// ===== Level Up =====
void levelUp(Player &p) {
    while (p.exp >= 100) {
        p.exp -= 100;
        p.level++;
        p.hp += 20;
        p.atk += 5;
        p.def += 3;
        cout << "✨ Level Up! Lv " << p.level << " HP+20 ATK+5 DEF+3\n";
    }
}

// ===== Tutorial =====
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
        // Print map
        cout << "\n🗺 Tutorial Map\n";
        for (int i=0;i<5;i++) {
            for (int j=0;j<5;j++) {
                if (i==x && j==y) cout << "P ";
                else cout << demoMap[i][j] << " ";
            }
            cout << endl;
        }

        // Show stats
        cout << "\nHP=" << p.hp << " ATK=" << p.atk << " DEF=" << p.def
             << " GOLD=" << p.gold << " EXP=" << p.exp << " LV=" << p.level
             << " KEY=" << (hasKey ? "Y" : "N") << "\n";

        // Input move
        cout << "\nMove (w a s d): ";
        char m; cin >> m;
        if (cin.fail() || (m!='w'&&m!='a'&&m!='s'&&m!='d')) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Need help? Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int nx=x, ny=y;
        if (m=='w') nx--; else if (m=='s') nx++;
        else if (m=='a') ny--; else if (m=='d') ny++;
        if (nx<0||nx>=5||ny<0||ny>=5||demoMap[nx][ny]=='#') {
            cout << "🧱 Blocked!\n"; continue;
        }

        x=nx; y=ny;
        cout << "👉 Entered new area...\n";

        if (demoMap[x][y]=='K') {
            cout << "🔑 You found the key!\n";
            hasKey=true; demoMap[x][y]='.';
        }
        else if (demoMap[x][y]=='B') {
            cout << "⚔️ Enemy encountered!\n";
            int enemyHP=20;
            int minPower=5,maxPower=7;
            while (enemyHP>0 && p.hp>0) {
                int choice;
                cout << "\nChoose action: 1 Normal 2 Strong 3 Defend\n";
                cin >> choice;
                if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
                    cin.clear(); cin.ignore(10000,'\n');
                    cout << "Invalid input! Need help? Type help.\n";
                    string cmd; cin >> cmd;
                    if (cmd=="help") showHelp();
                    continue;
                }
                int playerAttack=0;
                if (choice==1) playerAttack=rand()%p.atk+1;
                else if (choice==2 && rand()%100<50) playerAttack=p.atk+rand()%5;
                else if (choice==3){p.hp+=5; cout<<"Defend +5HP!\n";}
                enemyHP-=playerAttack;
                if(playerAttack>0) cout<<"You dealt "<<playerAttack<<" damage!\n";
                if(enemyHP<=0){cout<<"🎉 Enemy defeated!\n"; p.gold+=10; p.exp+=20; break;}
                int edmg=(rand()%(maxPower-minPower+1))+minPower;
                if(choice==3) edmg-=p.def;
                if(edmg<1) edmg=1;
                p.hp-=edmg;
                cout<<"Enemy dealt "<<edmg<<" damage!\n";
            }
            demoMap[x][y]='.';
        }
        else if (demoMap[x][y]=='G') {
            if (!hasKey) {
                cout << "🚫 You have not found the key yet! Cannot rescue princess.\n";
                continue;
            } else {
                cout << "🎉 You rescued the princess! Tutorial complete!\n";
                break;
            }
        }
    }

    cout << "Press Enter to start the real game...\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// ===== Battle System =====
void fight(Player &p, int enemyMin, int enemyMax) {
    cout << "\n⚔️ You encountered an enemy!\n";
    int enemyHP = 30 + rand() % 20;
    cout << "Enemy attack range: " << enemyMin << " - " << enemyMax << "\n";

    while (enemyHP > 0 && p.hp > 0) {
        int choice;
        cout << "\nChoose action: 1 Normal 2 Strong 3 Defend\n";
        cin >> choice;
        if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Need help? Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int playerAttack = 0;
        if (choice == 1) playerAttack = rand() % p.atk + 1;
        else if (choice == 2 && rand() % 100 < 50) playerAttack = p.atk + rand() % 5;
        else if (choice == 3) { p.hp += 5; cout << "Defend +5HP!\n"; }

        enemyHP -= playerAttack;
        if (playerAttack > 0) cout << "You dealt " << playerAttack << " damage!\n";

        if (enemyHP <= 0) {
            cout << "🎉 Enemy defeated! +20 Gold, +50 EXP\n";
            p.gold += 20; p.exp += 50; levelUp(p);
            break;
        }

        int dmg = (rand() % (enemyMax - enemyMin + 1)) + enemyMin;
        if (choice == 3) dmg -= p.def;
        if (dmg < 1) dmg = 1;
        p.hp -= dmg;
        cout << "Enemy dealt " << dmg << " damage!\n";
    }
}

// ===== Boss Fight =====
void bossFight(Player &p, int bossMin, int bossMax) {
    cout << "👹 Boss battle begins!\n";
    int bossHP = 100;
    cout << "Boss attack range: " << bossMin << " - " << bossMax << "\n";

    while (bossHP > 0 && p.hp > 0) {
        int choice;
        cout << "\nChoose action: 1 Normal 2 Strong 3 Defend\n";
        cin >> choice;
        if (cin.fail() || (choice!=1 && choice!=2 && choice!=3)) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Need help? Type help.\n";
            string cmd; cin >> cmd;
            if (cmd=="help") showHelp();
            continue;
        }

        int playerAttack = 0;
        if (choice == 1) playerAttack = rand() % p.atk + 1;
        else if (choice == 2 && rand() % 100 < 50) playerAttack = p.atk + rand() % 10;
        else if (choice == 3) { p.hp += 10; cout << "Defend +10HP!\n"; }

        bossHP -= playerAttack;
        if (playerAttack > 0) cout << "You dealt " << playerAttack << " damage!\n";

        if (bossHP <= 0) {
            cout << "🎉 Boss defeated! +100 Gold, +200 EXP\n";
            p.gold += 100; p.exp += 200; levelUp(p);
            break;
        }

        int dmg = (rand() % (bossMax - bossMin + 1)) + bossMin;
        if (choice == 3) dmg -= p.def;
        if (dmg < 1) dmg = 1;
        p.hp -= dmg;
        cout << "Boss dealt " << dmg << " damage!\n";
    }
}

// ===== Event System =====
void event(Player &p, int enemyMin, int enemyMax, int bossMin, int bossMax) {
    int r = rand() % 100;
    if (r < 40) fight(p, enemyMin, enemyMax);       // Enemy 40%
    else if (r < 60) { cout << "🛒 You met a merchant! (Shop not implemented)\n"; }
    else if (r < 75) { cout << "👴 You met an old man. He heals you +10HP.\n"; p.hp += 10; }
    else cout << "🌿 Quiet area. Nothing happens.\n";
}

// ===== Map Display =====
void displayMap() {
    cout << "\n🗺 MAP\n";
    cout << "=====================\n";
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (i == px && j == py) {
                cout << "P ";
                discovered[i][j] = true;
            }
            else if (!discovered[i][j]) cout << "? ";
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
    if (grid[nx][ny] == '#') { cout << "🧱 Blocked!\n"; return; }

    px = nx; py = ny;
    discovered[px][py] = true;
    cout << "\n👉 Entered new area...\n";

    if (grid[px][py] == 'K') { cout << "🔑 You found the key!\n"; p.hasKey = true; grid[px][py] = '.'; return; }
    if (grid[px][py] == 'T') { cout << "💥 Trap! HP -15\n"; p.hp -= 15; grid[px][py] = '.'; return; }
    if (grid[px][py] == 'C') { cout << "🎁 Chest! +20 Gold\n"; p.gold += 20; grid[px][py] = '.'; return; }
    if (grid[px][py] == 'B') { bossFight(p, bossMin, bossMax); grid[px][py] = '.'; return; }
    if (grid[px][py] == 'G') {
        if (!p.hasKey) {
            cout << "🚫 You have not found the key yet! Cannot rescue princess.\n";
        } else {
            cout << "🎉 You rescued the princess! Victory!\n";
        }
        return;
    }

    if (grid[px][py] == '.') event(p, enemyMin, enemyMax, bossMin, bossMax);
}

// ===== Main =====
int main() {
    srand(time(0));
    Player p;
    showIntro();

    int enemyMin, enemyMax, bossMin, bossMax;
    chooseDifficulty(enemyMin, enemyMax, bossMin, bossMax);
    tutorial(p);

    // Initialize map
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = (rand()%100 < 25 ? '#' : '.'); // Random walls
            visited[i][j] = false;
            discovered[i][j] = false;
        }
    }
    grid[gx][gy] = 'G'; // Princess
    grid[SIZE/2][SIZE/2] = 'B'; // Boss
    grid[rand()%SIZE][rand()%SIZE] = 'K'; // Key
    grid[rand()%SIZE][rand()%SIZE] = 'T'; // Trap
    grid[rand()%SIZE][rand()%SIZE] = 'C'; // Chest

    while (true) {
        displayMap();

        cout << "\nHP=" << p.hp << " ATK=" << p.atk << " DEF=" << p.def
             << " GOLD=" << p.gold << " EXP=" << p.exp << " LV=" << p.level
             << " KEY=" << (p.hasKey ? "Y" : "N") << "\n";

        if (p.hp <= 0) { 
            cout << "💀 You died. Game Over.\n"; 
            break; 
        }
        if (px == gx && py == gy && p.hasKey) { 
            cout << "🎉 You rescued the princess! Victory!\n"; 
            break; 
        }
        if (px == gx && py == gy && !p.hasKey) {
            cout << "🚫 You have not found the key yet! Cannot rescue princess.\n";
        }

        char m;
        cout << "\nMove WASD: ";
        cin >> m;
        if (cin.fail() || (m!='w'&&m!='a'&&m!='s'&&m!='d')) {
            cin.clear(); cin.ignore(10000,'\n');
            cout << "Invalid input! Need help? Type help.\n";
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
