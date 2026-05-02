#ifndef USER_SAVE_SYSTEM_H
#define USER_SAVE_SYSTEM_H

#include <string>
#include <vector>

namespace user_save_system {

struct AuthResult {
    bool authenticated = false;
    bool existingUser = false;
    std::string message;
};

struct SaveData {
    bool valid = false;

    int size = 0;
    int px = 0;
    int py = 0;
    int gx = 0;
    int gy = 0;

    int monsterMin = 0;
    int monsterMax = 0;
    int bossMin = 0;
    int bossMax = 0;

    int hp = 100;
    int atk = 8;
    int def = 5;
    int gold = 10;
    int exp = 0;
    int level = 1;
    bool hasKey = false;

    std::vector<std::string> gridRows;
    std::vector<std::string> discoveredRows;
    std::vector<std::string> visitedRows;

    bool cleared = false;  // true when this difficulty has been beaten
};

AuthResult loginOrRegister(const std::string& username, const std::string& password);
bool hasSave(const std::string& username);
bool saveProgress(const std::string& username, const SaveData& data);
bool loadProgress(const std::string& username, SaveData& outData);

} // namespace user_save_system

#endif
