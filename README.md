# COMP2113-Group-Project-2026
# 🛡️ Knight Maze RPG

A **text-based roguelike RPG game** developed in **C++**, featuring maze exploration, combat mechanics, and procedural events.
This project demonstrates core concepts in **game logic design, data structures, and system modularization**.

---

## 🎮 Game Overview

You play as a **knight** trapped in a dangerous maze.
Your mission is to:

* 🔍 Explore the maze
* 🔑 Find the key
* ⚔️ Defeat enemies and the boss
* 👑 Rescue the princess

But beware — the maze is filled with **traps, random events, and hidden dangers**.

---

## ✨ Features

### 🗺️ Maze System

* Procedurally generated grid-based map
* Fog of war (undiscovered areas hidden)
* Obstacles (`#`) block movement

### ⚔️ Combat System

* Turn-based battle mechanics
* Multiple actions:

  * Normal Attack
  * Power Attack (probability-based)
  * Defend (reduces damage + heal)
* Enemy and Boss with scalable difficulty

### 📈 Progression System

* EXP-based leveling system
* Stat growth:

  * HP
  * ATK
  * DEF

### 🎲 Random Event System

* Enemy encounters
* Merchant encounters (expandable)
* Helpful NPC (healing)
* Empty zones

### 🧭 Gameplay Mechanics

* WASD movement
* Key-door system (must find key before winning)
* Traps and treasure chests

---

## 🧱 Project Structure

```bash
main.cpp
```

Key components:

* `Player struct` → stores player stats
* `fight()` → enemy combat logic
* `bossFight()` → boss battle system
* `event()` → random event generator
* `movePlayer()` → movement + interaction
* `displayMap()` → map rendering

---

## 🚀 How to Run

### Compile

```bash
g++ comp2113.cpp -o game
```

### Run

```bash
./game
```

---

## 🧠 Technical Highlights

* Use of **2D arrays** for map representation
* **Random number generation** (`rand()`) for procedural gameplay
* **State management** (player stats, discovered map)
* Input validation and error handling
* Modular function-based architecture

---

## 📸 Example Gameplay

```
🗺 MAP
? ? ? ? ?
? # . . ?
? . P . ?
? . . . ?
? ? ? ? G
```

```
HP=100 ATK=8 DEF=5 GOLD=10 EXP=0 LV=1 KEY=N
Move (WASD): d
👉 Entering new area...
⚔️ You encountered an enemy!
```

---

## 🔧 Future Improvements

* 🛒 Fully implemented shop system
* 🧥 Equipment system (weapons & armor)
* 👾 Multiple enemy types with AI behaviors
* 🎨 Colored terminal UI
* 💾 Save / load system
* 🗺️ Smarter maze generation (guaranteed solvability)

---

## 📚 Learning Outcomes

This project strengthened my understanding of:

* Game system design
* Procedural generation
* C++ programming fundamentals
* Struct-based data modeling
* Debugging and iterative development

---

## 👤 Author

**Shuyi Yuan**

* Aspiring Electrical & Electronic Engineer
* Interested in **embedded systems, robotics, and intelligent devices**
* Passionate about building **interactive and system-level projects**

---
