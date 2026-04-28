#include "user_save_system.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {

const std::string kDataDir = "user_save_system/data";
const std::string kUsersFile = "user_save_system/data/users.txt";
const std::string kSaveDir = "user_save_system/data/saves";
const int kMaxMapSize = 50;

bool ensureStorage() {
    std::error_code ec;
    fs::create_directories(kSaveDir, ec);
    if (ec) {
        return false;
    }

    if (!fs::exists(kUsersFile)) {
        std::ofstream createFile(kUsersFile);
        if (!createFile.is_open()) {
            return false;
        }
    }

    return true;
}

std::string toHexString(const std::string& input) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : input) {
        oss << std::setw(2) << static_cast<int>(c);
    }
    return oss.str();
}

std::string getSavePath(const std::string& username) {
    return kSaveDir + "/" + toHexString(username) + ".save";
}

bool isValidCredential(const std::string& value) {
    if (value.empty() || value.size() > 32) {
        return false;
    }

    for (char c : value) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }

    return true;
}

std::unordered_map<std::string, std::string> loadUsers() {
    std::unordered_map<std::string, std::string> users;
    std::ifstream fin(kUsersFile);
    if (!fin.is_open()) {
        return users;
    }

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) {
            continue;
        }
        std::size_t sep = line.find('\t');
        if (sep == std::string::npos) {
            continue;
        }
        std::string username = line.substr(0, sep);
        std::string password = line.substr(sep + 1);
        users[username] = password;
    }

    return users;
}

bool appendUser(const std::string& username, const std::string& password) {
    std::ofstream fout(kUsersFile, std::ios::app);
    if (!fout.is_open()) {
        return false;
    }
    fout << username << '\t' << password << "\n";
    return static_cast<bool>(fout);
}

bool readExpectedTag(std::istream& in, const std::string& expectedTag) {
    std::string tag;
    in >> tag;
    return static_cast<bool>(in) && tag == expectedTag;
}

} // namespace

namespace user_save_system {

AuthResult loginOrRegister(const std::string& username, const std::string& password) {
    AuthResult result;

    if (!ensureStorage()) {
        result.message = "Storage initialization failed.";
        return result;
    }

    if (!isValidCredential(username) || !isValidCredential(password)) {
        result.message = "Username/password must be 1-32 chars: letters, digits, _ or -.";
        return result;
    }

    std::unordered_map<std::string, std::string> users = loadUsers();
    auto it = users.find(username);
    if (it != users.end()) {
        result.existingUser = true;
        if (it->second == password) {
            result.authenticated = true;
            result.message = "Login successful. Progress will be restored if found.";
        } else {
            result.message = "Wrong password.";
        }
        return result;
    }

    if (!appendUser(username, password)) {
        result.message = "Unable to create user.";
        return result;
    }

    result.authenticated = true;
    result.existingUser = false;
    result.message = "New user created.";
    return result;
}

bool hasSave(const std::string& username) {
    if (!ensureStorage()) {
        return false;
    }
    return fs::exists(getSavePath(username));
}

bool saveProgress(const std::string& username, const SaveData& data) {
    if (!ensureStorage()) {
        return false;
    }

    if (data.size <= 0 || data.size > kMaxMapSize) {
        return false;
    }
    if (static_cast<int>(data.gridRows.size()) != data.size || static_cast<int>(data.discoveredRows.size()) != data.size) {
        return false;
    }

    for (int i = 0; i < data.size; i++) {
        if (static_cast<int>(data.gridRows[i].size()) != data.size) {
            return false;
        }
        if (static_cast<int>(data.discoveredRows[i].size()) != data.size) {
            return false;
        }
    }

    std::ofstream fout(getSavePath(username), std::ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    fout << "VERSION 1\n";
    fout << "SIZE " << data.size << "\n";
    fout << "POS " << data.px << ' ' << data.py << "\n";
    fout << "GOAL " << data.gx << ' ' << data.gy << "\n";
    fout << "ENEMY " << data.enemyMin << ' ' << data.enemyMax << "\n";
    fout << "BOSS " << data.bossMin << ' ' << data.bossMax << "\n";
    fout << "PLAYER " << data.hp << ' ' << data.atk << ' ' << data.def << ' '
         << data.gold << ' ' << data.exp << ' ' << data.level << ' ' << (data.hasKey ? 1 : 0) << "\n";

    fout << "GRID\n";
    for (const std::string& row : data.gridRows) {
        fout << row << "\n";
    }

    fout << "DISCOVERED\n";
    for (const std::string& row : data.discoveredRows) {
        fout << row << "\n";
    }

    return static_cast<bool>(fout);
}

bool loadProgress(const std::string& username, SaveData& outData) {
    outData = SaveData();

    if (!ensureStorage()) {
        return false;
    }

    std::ifstream fin(getSavePath(username));
    if (!fin.is_open()) {
        return false;
    }

    if (!readExpectedTag(fin, "VERSION")) return false;
    int version = 0;
    fin >> version;
    if (!fin || version != 1) return false;

    if (!readExpectedTag(fin, "SIZE")) return false;
    fin >> outData.size;

    if (!readExpectedTag(fin, "POS")) return false;
    fin >> outData.px >> outData.py;

    if (!readExpectedTag(fin, "GOAL")) return false;
    fin >> outData.gx >> outData.gy;

    if (!readExpectedTag(fin, "ENEMY")) return false;
    fin >> outData.enemyMin >> outData.enemyMax;

    if (!readExpectedTag(fin, "BOSS")) return false;
    fin >> outData.bossMin >> outData.bossMax;

    if (!readExpectedTag(fin, "PLAYER")) return false;
    int hasKeyInt = 0;
    fin >> outData.hp >> outData.atk >> outData.def >> outData.gold >> outData.exp >> outData.level >> hasKeyInt;
    outData.hasKey = (hasKeyInt != 0);

    if (!fin || outData.size <= 0 || outData.size > kMaxMapSize) {
        return false;
    }

    std::string marker;
    fin >> marker;
    if (!fin || marker != "GRID") return false;

    outData.gridRows.assign(outData.size, std::string());
    outData.discoveredRows.assign(outData.size, std::string());

    std::string line;
    std::getline(fin, line); // consume trailing newline

    for (int i = 0; i < outData.size; i++) {
        std::getline(fin, outData.gridRows[i]);
        if (!fin || static_cast<int>(outData.gridRows[i].size()) != outData.size) {
            return false;
        }
    }

    std::getline(fin, marker);
    if (!fin || marker != "DISCOVERED") {
        return false;
    }

    for (int i = 0; i < outData.size; i++) {
        std::getline(fin, outData.discoveredRows[i]);
        if (!fin || static_cast<int>(outData.discoveredRows[i].size()) != outData.size) {
            return false;
        }
    }

    outData.valid = true;
    return true;
}

} // namespace user_save_system
