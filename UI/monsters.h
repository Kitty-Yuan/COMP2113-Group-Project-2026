// monsters.h
#ifndef MONSTERS_H
#define MONSTERS_H
#include <string>
using namespace std;

struct Monster {
    string name;
    string appearance;
    string attack;
    string defense;
    string special_ability;
};

extern Monster ghost;
extern Monster mushroom;
extern Monster owl;

#endif