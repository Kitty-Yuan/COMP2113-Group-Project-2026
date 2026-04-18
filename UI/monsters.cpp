/*
This file contains the text based monster and elves UI design.
*/

/*
 @todo:
 - Add more monsters and elves with unique appearances and abilities.
 - 设计怪兽真正的技能和属性，确保它们在游戏中有实际的作用。
*/
#include <iostream>
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