/*
This file contains the text based monster and elves UI design, and also the look of our player.
*/

/*
 @todo:
 - Add more monsters and elves with unique appearances and abilities.
 - 设计怪兽真正的技能和属性，确保它们在游戏中有实际的作用。
*/
#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include "monsters.h"

using namespace std;

// Ghost
Monster ghost = {
    "Ghost",
    R"(
 .------.
/  #   # \
|        |
~` ~` ~ `~
)",
    "Ethereal Strike: Deals damage that ignores armor.",
    "Incorporeal Form: Reduces physical damage taken by 50%.",
    "Haunting Presence: Chance to frighten enemies, reducing their attack power."
};  

// Mushroom
Monster mushroom = {
    "Mushroom",
    R"(
 .-o-OO-o-.
(__________)
 |  *   * |
 |________|                         
)",
    "Spore Burst: Releases spores that damage and poison enemies.",
    "Fungal Shield: Creates a temporary shield that absorbs damage.",
    "Regeneration: Heals over time when not taking damage."
};

//owl
Monster owl = {
    "Owl",
    R"(
  /\ /\
((@ v @))
() ::: ()
  VV VV
)",
    "Silent Flight: Allows the owl to move without making noise, avoiding detection.",
    "Keen Vision: Increases accuracy and critical hit chance.",
    "Night Hunter: Gains increased damage and evasion during nighttime."
};

//Blob
Monster blob = {
    "Blob",
    R"(
    .----.
   ( @  @ )
   (      )
   `------´
)",
    "Acidic Touch: Deals damage over time and reduces enemy armor.",
    "Amorphous Body: Can squeeze through tight spaces and is immune to being grappled.",
    "Split: When reduced to low health, splits into two smaller blobs with half health."
};

//Player

string player = R"(

        ┌───────┐
        │--o--o-│  
        │       │ 
        │       │  
        │       │ 
        └──├──├─┘ 
           │  │

)";


//cloud
string cloud = R"(
      .--.
   .-(    ).
  (___.__)__)
)"; 



// 云朵结构体
struct Cloud {
    int x;          // X坐标（列位置）
    int y;          // Y坐标（行位置）
    string art;     // 云朵图案
    int width;      // 图案宽度
    int height;     // 图案高度
    
    Cloud(int startX, int startY, string cloudArt) : x(startX), y(startY), art(cloudArt) {
        // 计算云朵的宽度和高度
        height = 0;
        width = 0;
        for (char c : art) {
            if (c == '\n') {
                height++;
            }
        }
        // 找到最长行的长度
        size_t pos = 0;
        while (pos < art.length()) {
            size_t end = art.find('\n', pos);
            if (end == string::npos) end = art.length();
            int lineWidth = end - pos;
            if (lineWidth > width) width = lineWidth;
            pos = end + 1;
        }
    }
    
    // 绘制云朵
    void draw() const {
        int lineY = y;
        string line;
        for (char c : art) {
            if (c == '\n') {
                mvprintw(lineY++, x, "%s", line.c_str());
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            mvprintw(lineY, x, "%s", line.c_str());
        }
    }
    
    // 清除云朵（用空格覆盖）
    void clear() const {
        for (int i = 0; i < height; i++) {
            mvprintw(y + i, x, string(width, ' ').c_str());
        }
    }
    
    // 向左移动
    void moveLeft() {
        clear();
        x--;
        draw();
    }
};

// 玩家类
class Player {
private:
    int screenY;        // 玩家的Y坐标（固定在屏幕中央的某一行）
    int screenX;        // 玩家的X坐标（固定在屏幕中央的某一列）
    string art;
    int width;
    int height;
    
public:
    Player(int centerY, int centerX) : screenY(centerY), screenX(centerX), art(player) {
        // 计算玩家图案尺寸
        height = 0;
        width = 0;
        for (char c : art) {
            if (c == '\n') {
                height++;
            }
        }
        size_t pos = 0;
        while (pos < art.length()) {
            size_t end = art.find('\n', pos);
            if (end == string::npos) end = art.length();
            int lineWidth = end - pos;
            if (lineWidth > width) width = lineWidth;
            pos = end + 1;
        }
    }
    
    // 绘制玩家
    void draw() const {
        int lineY = screenY;
        string line;
        for (char c : art) {
            if (c == '\n') {
                mvprintw(lineY++, screenX, "%s", line.c_str());
                line.clear();
            } else {
                line += c;
            }
        }
        if (!line.empty()) {
            mvprintw(lineY, screenX, "%s", line.c_str());
        }
    }
    
    // 清除玩家
    void clear() const {
        for (int i = 0; i < height; i++) {
            mvprintw(screenY + i, screenX, string(width, ' ').c_str());
        }
    }
    
    // 玩家位置不变，但返回坐标供外部使用
    int getX() const { return screenX; }
    int getY() const { return screenY; }
};

// 游戏类
class SideScrollGame {
private:
    vector<Cloud> clouds;
    int screenWidth;
    int screenHeight;
    int cloudSpawnCounter;
    int cloudSpawnDelay;
    int score;
    bool running;
    
public:
    SideScrollGame() : cloudSpawnCounter(0), cloudSpawnDelay(20), score(0), running(true) {
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);  // 非阻塞输入
        
        getmaxyx(stdscr, screenHeight, screenWidth);
        
        // 初始化随机数
        srand(time(0));
        
        // 生成初始云朵
        for (int i = 0; i < 3; i++) {
            spawnCloud(screenWidth + (i * 30));
        }
    }
    
    ~SideScrollGame() {
        endwin();
    }
    
    // 生成新云朵
    void spawnCloud(int x = -1) {
        if (x == -1) {
            x = screenWidth;
        }
        // 随机Y位置（上方区域，避开玩家）
        int y = rand() % (screenHeight / 3);
        if (y < 1) y = 1;
        
        clouds.emplace_back(x, y, cloud);
        cloudSpawnCounter = 0;
    }
    
    // 更新所有云朵位置
    void updateClouds() {
        // 移动所有云朵向左
        for (auto& cloud : clouds) {
            cloud.moveLeft();
        }
        
        // 移除超出屏幕左侧的云朵
        clouds.erase(
            remove_if(clouds.begin(), clouds.end(),
                [this](const Cloud& c) { return c.x + c.width < 0; }),
            clouds.end()
        );
        
        // 生成新云朵
        cloudSpawnCounter++;
        if (cloudSpawnCounter >= cloudSpawnDelay) {
            spawnCloud();
            // 随机调整生成间隔
            cloudSpawnDelay = 15 + rand() % 15;
        }
    }
    
    // 处理输入
    void handleInput() {
        int ch = wgetch(stdscr);
        // 玩家固定不动，但可以按Q退出
        if (ch == 'q' || ch == 'Q') {
            running = false;
        }
    }
    
    // 显示UI信息
    void drawUI() {
        mvprintw(0, 0, "Score: %d", score);
        mvprintw(1, 0, "Clouds: %zu", clouds.size());
        mvprintw(2, 0, "Press Q to quit");
        mvprintw(3, 0, "← 云朵向左移动 | 玩家固定在中央 →");
        
        // 绘制地面
        for (int i = 0; i < screenWidth; i++) {
            mvprintw(screenHeight - 5, i, "=");
        }
    }
    
    // 更新分数（云朵成功通过玩家）
    void updateScore() {
        // 简单计分：每帧加1分
        score++;
    }
    
    // 渲染所有内容
    void render(Player& player) {
        clear();
        
        // 绘制所有云朵
        for (auto& cloud : clouds) {
            cloud.draw();
        }
        
        // 绘制玩家（固定在屏幕中央偏下位置）
        player.draw();
        
        // 绘制UI
        drawUI();
        
        refresh();
    }
    
    bool isRunning() const { return running; }
    
    // 游戏主循环
    void run() {
        Player player(screenHeight - 12, screenWidth / 2 - 10);
        
        while (running) {
            handleInput();
            updateClouds();
            updateScore();
            render(player);
            
            // 控制游戏速度
            napms(50);  // 约20 FPS
        }
    }
};

// Main函数
int main() {
    SideScrollGame game;
    game.run();
    
    // 游戏结束画面
    clear();
    mvprintw(10, 20, "Game Over!");
    mvprintw(12, 20, "Press any key to exit...");
    refresh();
    getch();
    
    return 0;
}