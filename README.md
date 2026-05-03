# 🎮 Maze Knight - A Roguelike Dungeon Crawler

A terminal-based dungeon crawler game built with C++ and ncurses. Navigate through procedurally generated mazes, battle fearsome monsters, collect treasures, and face challenging bosses across four difficulty levels.

---

## 📋 Table of Contents
- [Features](#features)
- [Game Overview](#game-overview)
- [Getting Started](#getting-started)
- [How to Play](#how-to-play)
- [Game Mechanics](#game-mechanics)
- [Difficulty Levels](#difficulty-levels)
- [Enemies](#enemies)
- [Building & Running](#building--running)
- [Controls](#controls)
- [Save System](#save-system)

---

## ✨ Features

### Core Gameplay
- **Procedurally Generated Mazes** - Each playthrough generates unique maze layouts with 4 difficulty levels
- **RPG Progression** - Gain experience, level up, and improve character stats
- **Combat System** - Battle unique monsters with special abilities and weaknesses
- **Item Collection** - Find keys, treasures, and gold to aid your journey
- **Boss Encounters** - Face challenging boss fights and mini-games
- **Save & Load System** - Resume your progress across game sessions

### User Interface
- **Colorful Terminal UI** - ASCII art graphics with color support
- **Player Stats Dashboard** - Real-time display of HP, ATK, DEF, EXP, Level, and Gold
- **Mouse Support** - Interactive buttons and UI elements
- **Tutorial System** - Learn the basics before starting
- **Responsive Design** - Adapts to different terminal sizes

### Game Features
- **Difficulty Progression** - Track cleared levels and unlock harder challenges
- **Multiple Monster Types** - Each with unique abilities and strategies
- **Princess Chase Minigame** - Time-based escape sequence with dynamic difficulty
- **Random Events** - Shops, mysterious strangers, and unexpected encounters
- **Level-up Rewards** - Stat increases and special unlocks

---

## 🎯 Game Overview

**Maze Knight** is a roguelike dungeon exploration game where you navigate through dark mazes, encounter various monsters, collect items, and ultimately find the exit. The game features:

- **Player Start** at position (0,0) marked as 'P'
- **Goal/Exit** at the bottom-right corner marked as 'G'
- **Key Item** required to unlock the exit
- **Various Collectibles** scattered throughout (Treasure, Coins, Special Items)
- **Boss Fight** at the center of the map
- **Multiple Difficulty Modes** for varied challenge levels
- **Fog of War System** - Only discovered tiles are visible

---

## 🚀 Getting Started

### Prerequisites
- C++11 or later
- ncurses library (`libncurses-dev` on Linux, included on macOS)
- Make build system
- POSIX-compliant system (Linux, macOS, or WSL on Windows)

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd COMP2113-Group-Project-2026
   ```

2. **Build the project**
   ```bash
   make
   ```

3. **Run the game**
   ```bash
   ./build/comp2113
   ```

---

##  API Documentation

For a comprehensive reference of all functions, structures, and game mechanics:

### Access the Documentation

**Local (Recommended):**
1. Clone/download the repository
2. Open `API_REFERENCE.html` directly in your browser
   ```bash
   # macOS
   open API_REFERENCE.html
   
   # Linux
   xdg-open API_REFERENCE.html
   
   # Or just double-click the file
   ```

### Documentation Features
- 📖 All function signatures and parameters
- 🏗️ Data structure definitions  
- ⚔️ Monster ability mechanics with detailed effects
- 📊 Game flow diagrams and state machines
- 🔍 Quick search functionality (use the search box at the top)
- 📝 Usage examples for all major systems

### How to Use the Documentation
1. **Search** - Use the search box to quickly find functions or structures by name
2. **Navigation** - Click menu items in the left sidebar to jump to sections
3. **Details** - Each entry includes full signatures, parameters, and explanations

---

## �🕹️ How to Play

### Main Game Loop

1. **Start Screen** - Choose to begin a new game or load a previous save
2. **Difficulty Selection** - Pick your challenge level (1-4)
3. **Tutorial (Optional)** - Learn basic controls and mechanics by pressing 'T'
4. **Exploration** - Navigate the maze using WASD or arrow keys
5. **Combat** - Encounter and battle monsters
6. **Collection** - Find items, treasures, and gold
7. **Boss Fight** - Defeat the boss guarding the center of the maze
8. **Princess Chase** - Complete the mini-game challenge
9. **Victory** - Escape the maze with the key and reach the goal

### Winning Conditions
- Find and obtain the **Key** scattered in the maze
- Navigate to the **Goal** marked 'G' at the bottom-right corner
- Survive all encounters

### Losing Conditions
- Your **HP drops to 0** during combat or events

---

## ⚙️ Game Mechanics

### Player Stats

| Stat | Purpose | Initial Value | Growth |
|------|---------|-------|--------|
| **HP** | Health points - reach 0 to die | 100 | +10 per level |
| **ATK** | Attack power - determines damage dealt | 8 | +2 per level |
| **DEF** | Defense - reduces incoming damage | 5 | +1 per level |
| **EXP** | Experience points toward leveling up | 0 | Gained from monster defeats |
| **GOLD** | Currency for shops and purchases | 10 | Earned in combat or found |
| **LEVEL** | Current power level | 1 | Increases after gaining 100 EXP |
| **KEY** | Required to unlock the exit | No | Found in the maze |

### Combat System
- **Damage Calculation**: `Attacker ATK vs. Defender DEF`
- **Monster Special Abilities**: Each monster has unique debuffs and attacks
- **Stat Scaling**: Difficulty levels scale monster stats dynamically
- **Percentage-Based Effects**: Abilities have probability-based triggers

### Maze Navigation
- **Fog of War** - Only discovered tiles are revealed on the map
- **Pathfinding** - Critical locations (start, key, goal, boss) are always accessible
- **Environmental Obstacles** - Walls (`#`) block movement
- **Random Encounters** - Events trigger while exploring
- **Discovery Tracking** - Visited tiles remain visible

---

## 🔥 Difficulty Levels

| Level | Map Size | Monster ATK Range | Boss ATK Range | Player Scaling | Recommended For |
|-------|----------|----------|----------|----------|-----------------|
| **1 - Easy** | 9×9 | 5-10 | 10-15 | Base stats | Newcomers & Learning |
| **2 - Normal** | 12×12 | 8-12 | 12-18 | Standard difficulty | Regular Players |
| **3 - Hard** | 15×15 | 10-15 | 15-22 | Increased monster aggression | Experienced Players |
| **4 - Hell** | 20×20 | 12-18 | 18-25 | Extreme scaling | Hardcore Challengers |

Each difficulty also tracks completion status independently, allowing you to progress through the game and unlock harder modes.

---

## 👹 Enemies

### Ghost
- **Appearance**: Ethereal spectral form
- **Special Ability 1**: *Incorporeal Form* - Reduces your attack damage by 50%
- **Special Ability 2**: *Spectral Drain* - Reduces your attack damage by 30%
- **Strategy**: This ghost can turn into mist to avoid physical attacks and haunt enemies to reduce their effectiveness. Watch for its evasive tactics.

### Chestnut
- **Appearance**: Mischievous chestnut creature  
- **Special Ability 1**: *Spore Burst* - Reduces your attack damage by 50%
- **Special Ability 2**: *Toxic Cloud* - 70% chance your attack will miss
- **Strategy**: Releases poisonous spores and creates protective fungal shields. Be wary of ongoing damage effects and avoid missing your attacks.

### Owl
- **Appearance**: Mysterious winged predator
- **Special Ability 1**: *Fire Blow* - Unleashes blazing wave dealing extra 10 damage
- **Special Ability 2**: *Flash Blindness* - 30% chance your attack will miss
- **Strategy**: Known for silent flight and piercing gaze. Can ignite targets for damage-over-time effects. Use defensive tactics when blinded.

### Blob
- **Appearance**: Cute gelatinous entity with big appetite
- **Special Ability 1**: *Consume Gold* - Steals up to 15 gold coins from you
- **Special Ability 2**: *Gelatinous Body* - Your attack passes through, causing 100% miss
- **Strategy**: Can split into smaller blobs when damaged. Great at prolonging battles and overwhelming through numbers. Manage your gold carefully.

---

## 🛠️ Building & Running

### Build Commands
```bash
# Full build and run
make && ./build/comp2113

# Build only
make

# Clean build artifacts
make clean

# Run tests
make test
```

### Project Structure
```
COMP2113-Group-Project-2026/
├── comp2113.cpp              # Main game logic and core engine
├── test.cpp                  # Unit tests
├── ui/
│   ├── ui_ux.cpp            # UI rendering and visual elements
│   └── ui_ux.h              # UI interface definitions
├── user_save_system/
│   ├── user_save_system.cpp  # Save/load functionality
│   ├── user_save_system.h    # Save system interface
│   └── data/
│       ├── users.txt         # User accounts
│       └── saves/            # Individual save files (.save)
├── build/
│   └── comp2113              # Compiled executable
├── Makefile                  # Build configuration
├── README.md                 # This file
├── GAME_MANUAL.md           # Detailed game guide
├── API_REFERENCE.html       # Code documentation
└── LICENSE                  # Project license
```

---

## 🎮 Controls

### Movement
| Key | Action |
|-----|--------|
| **W** / **↑** | Move up |
| **A** / **←** | Move left |
| **S** / **↓** | Move down |
| **D** / **→** | Move right |

### Menu Navigation
| Key | Action |
|-----|--------|
| **1-4** | Select difficulty |
| **T** | Toggle tutorial on difficulty selection |
| **ENTER** | Confirm selection / Continue |
| **Mouse Click** | Interact with buttons (HOME, QUIT) |

### General
| Key | Action |
|-----|--------|
| **Q** | Quit game (in some menus) |
| **HOME** | Return to difficulty selection menu |

---

## 💾 Save System

### Features
- **Automatic Saves** - Progress is saved after loading/starting a game session
- **Difficulty-Specific Saves** - Each difficulty level has its own save slot
- **Completion Tracking** - Track which difficulties you've cleared with badges
- **Progress Restoration** - Resume exactly where you left off with all stats and map state

### Save Data Includes
- Player position (px, py) and stats
- Complete maze layout and discovery state
- Monster difficulty configuration
- Collected items and gold amount
- Current experience and level
- Boss status and cleared flag

### Save File Location
Save files are stored in: `user_save_system/data/saves/`

Format: `{username}__difficulty_{level}.save`

Example: `player1__difficulty_2.save`

---

## 📊 Technical Highlights

### Data Structures
- **2D Arrays** - Grid-based maze representation (`char grid[50][50]`)
- **Struct-Based Models** - Player stats, Monster data, SaveData
- **Queue-Based Pathfinding** - BFS for maze generation and path validation
- **State Management** - Global variables for game state and difficulty

### Key Algorithms
- **Procedural Maze Generation** - Random 25% wall placement with guaranteed paths
- **Breadth-First Search (BFS)** - Pathfinding between critical locations
- **Damage Calculation** - Stat-based combat with scaling
- **Difficulty Scaling** - Dynamic stat adjustment based on difficulty level

### Code Organization
- Modular function design with clear separation of concerns
- UI layer (`ui_ux.cpp`) separated from game logic (`comp2113.cpp`)
- Save system (`user_save_system.cpp`) for persistence
- ncurses integration for terminal UI rendering

### Memory Management
- **Dynamic Memory Allocation** - Battle monsters are dynamically created using `new` for state-isolated combat instances (`Monster* battleMonster = new Monster(...)`)
- **Manual Cleanup** - Explicit `delete` calls release allocated memory at the end of each battle, preventing memory leaks
- **Stack Allocation** - Game state objects (Player, SaveData) allocated on stack for fast access
- **STL Containers** - `std::vector` and `std::string` for automatic memory management of dynamic collections
- **RAII Principle** - Resource cleanup tied to scope lifetime where appropriate
- **Mixed Memory Strategy** - Combines stack, heap, and container allocation based on use case

---

## 📖 Additional Resources

- See [GAME_MANUAL.md](GAME_MANUAL.md) for detailed gameplay instructions and strategies
- Check [API_REFERENCE.html](API_REFERENCE.html) for code documentation
- Review [Makefile](Makefile) for build configuration

---

## 🔧 Future Improvements

- **Equipment System** - Weapons and armor for stat customization
- **Advanced Shop System** - Buy/sell items and healing
- **More Monster Types** - Expand enemy roster with unique mechanics
- **Map Themes** - Different visual styles for each difficulty
- **Sound Effects** - Audio feedback for actions
- **Leaderboard System** - Track high scores across playthroughs
- **Procedural Boss Variations** - Randomized boss abilities

---

## 📚 Learning Outcomes

This project demonstrates:

- **Game Architecture** - Modular design separating UI, logic, and persistence
- **Procedural Generation** - Algorithms for creating varied gameplay
- **C++17 Features** - Structured bindings, modern C++ practices
- **Memory Management** - RAII principles and stack-based allocation patterns
- **Data Persistence** - File I/O and serialization
- **Terminal Graphics** - ncurses library for interactive UI
- **Combat Systems** - Stat-based gameplay mechanics
- **State Management** - Tracking complex game states across sessions

---

## 📝 License

MIT Licences. This project is licensed under the terms specified in the [LICENSE](LICENSE) file.

---

## 👥 Project Team

COMP2113 Group 68 2026 - The University of Hong Kong
Yuan Shuyi, Yin Yuqiong, Lu Xinqi, Zhou Hanyu

---

**Embark on your quest! The maze awaits. ⚔️🗡️✨**
