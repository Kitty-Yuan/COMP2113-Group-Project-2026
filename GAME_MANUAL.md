# 🛡️ Knight Maze RPG - Comprehensive Game Manual

## Table of Contents
1. [Game Overview](#game-overview)
2. [Player Statistics](#player-statistics)
3. [Monster Types & Encounters](#monster-types--encounters)
4. [Game Mechanics](#game-mechanics)
5. [Game World Symbols](#game-world-symbols)
6. [Events System](#events-system)
7. [Items & Collectibles](#items--collectibles)
8. [Difficulty Levels](#difficulty-levels)
9. [Battle System](#battle-system)
10. [Progression System](#progression-system)

---

## Game Overview

**Knight Maze RPG** is a text-based roguelike RPG where you play as a knight trapped in a dangerous maze. Your mission is to:
- 🔍 Explore the procedurally generated maze
- 🔑 Find the key hidden in the maze
- ⚔️ Defeat monsters and the boss
- 👑 Rescue the princess to win

The game features turn-based combat, random events, stat progression, and permadeath mechanics.

---

## Player Statistics

### Base Stats
Each player has the following core statistics:

| Stat | Initial Value | Description | Effect |
|------|---------------|-------------|--------|
| **HP** (Health Points) | 100 | Your current health | Determining survival. Drops to 0 = Game Over |
| **ATK** (Attack Power) | 8 | Your physical damage output | Higher ATK = More damage per hit |
| **DEF** (Defense) | 5 | Your damage reduction | Reduces incoming damage; affects defend success chance |
| **GOLD** ($) | 10 | Currency for purchasing items | Used at shops; some events grant gold |
| **EXP** (Experience Points) | 0 | Accumulated from defeating monsters | Reach 100 EXP = Level Up |
| **LEVEL** | 1 | Your overall power progression | Increases with each level up |
| **KEY** | None | Special item needed to win | Must have before reaching the exit (G) |

### Level Up System

When you accumulate **100 EXP**, you automatically level up. Each level up grants:
- **+20 HP** (Increased health pool)
- **+5 ATK** (Increased damage output)
- **+3 DEF** (Better damage reduction)

The EXP counter resets to 0 after leveling, and excess EXP carries over.

### Max HP Mechanic
- Initial max HP is **100**
- Max HP increases by 20 with each level up
- Healing cannot exceed max HP

---

## Monster Types & Encounters

There are **4 unique monster types** in the game, each with distinct abilities and behaviors. Monsters have randomized HP (30-50) on each encounter.

### 1. **Ghost** 👻
**Description:** A spectral entity that haunts the maze corridors.

**Appearance:**
```
 .------.
/  #   # \
|        |
~` ~` ~ `~
```

**Stats:**
- HP: 30-50 (randomized)
- Attack Range: Difficulty-dependent (5-16 damage)
- Special Ability 1: **Incorporeal Form** - Reduces damage taken by 50%
- Special Ability 2: **Haunting Presence** - Chance to frighten enemies, reducing their attack power

**Mechanics:**
- When active ability triggers: Player ATK reduced by **50%** for multiple turns
- Special ability activates with ~25% chance each turn
- Duration: 2-4 rounds

**Rewards:** 
- Gold: +20
- EXP: +50

**Lore:** *"Hehehe, I'm the ghost. Don't be scared of me! I can turn into a mist to avoid physical attacks, and I can also haunt my enemies to make them less effective in battle."*

---

### 2. **Chestnut** 🌰
**Description:** A sentient fungal creature with poisonous spore attacks.

**Appearance:**
```
   .--OO--.
 /--_    _--\
(___@____@___)
    |    |
    (____)
```

**Stats:**
- HP: 30-50 (randomized)
- Attack Range: Difficulty-dependent (5-16 damage)
- Special Ability 1: **Spore Burst** - Releases spores that damage and poison enemies
- Special Ability 2: **Fungal Shield** - Creates a temporary shield that absorbs damage

**Mechanics:**
- When active ability triggers: Damage output increases by **+30%**
- Special ability activates with ~25% chance each turn
- Lasts 2-4 rounds

**Rewards:**
- Gold: +20
- EXP: +50

**Lore:** *"Hi, I'm a chestnut monster! I can release poisonous spores to damage my enemies, and I can also create a shield made of fungus to protect myself from attacks."*

---

### 3. **Owl** 🦉
**Description:** A mystical avian creature with fire breath and night vision abilities.

**Appearance:**
```
  /\ /\
((@ v @))
() ::: ()
  VV VV
```

**Stats:**
- HP: 30-50 (randomized)
- Attack Range: Difficulty-dependent (5-16 damage)
- Special Ability 1: **Fire Blow** - Owl becomes angry and breathes fire on player, dealing increased damage
- Special Ability 2: **Night Vision** - Owl can see in complete darkness; player has 20% chance to dodge attacks in darkness

**Mechanics:**
- When active ability triggers: Owl's damage output increases by **+40%**
- Special ability activates with ~25% chance each turn
- Duration: 2-4 rounds

**Rewards:**
- Gold: +20
- EXP: +50

**Lore:** *"This mysterious owl is known for its silent flight and piercing gaze. It can unleash a fiery breath attack that ignites the target, causing damage over time."*

---

### 4. **Blob** 💧
**Description:** A gelatinous creature with an appetite for gold and reproduction abilities.

**Appearance:**
```
    .----.
   ( @  @ )
   (      )
   `------`
```

**Stats:**
- HP: 30-50 (randomized)
- Attack Range: Difficulty-dependent (5-16 damage)
- Special Ability 1: **Eat Money** - Consumes gold coins to heal
- Special Ability 2: **Split** - At low health, splits into two smaller blobs

**Mechanics:**
- When active ability triggers: Blob's damage output increases by **+20%**
- Special ability activates with ~25% chance each turn
- More durable than other monsters due to splitting mechanic

**Rewards:**
- Gold: +20
- EXP: +50

**Lore:** *"This cute monster is called Blob. It has a gelatinous body and a big appetite for gold coins. When Blob's health drops below a certain threshold, it splits into two smaller blobs."*

---

### Boss: **The Final Guardian** 🏰

**Appearance:** Central maze location marked with 'B' on the map.

**Stats:**
- HP: 100 (Difficulty-independent)
- Attack Range: Difficulty-dependent (10-25 damage)
- Difficulty-based damage scaling:
  - Easy: 10-15 damage
  - Normal: 12-18 damage
  - Hard: 15-22 damage
  - Hell: 18-25 damage

**Mechanics:**
- Fought after navigating the maze to the center (SIZE/2, SIZE/2)
- Uses standard turn-based combat (same as regular monsters)
- Must be defeated before accessing the princess

**Rewards:**
- Unlocks access to the princess room
- No direct rewards; victory leads to final challenge

---

## Game Mechanics

### Combat System

**Combat Flow:**
1. When entering an undiscovered empty space for the first time, an event occurs
2. If the event is a monster encounter, battle begins
3. Battle is turn-based between player and monster

**Player Actions in Combat:**

| Action | Description | Cost | Effect |
|--------|-------------|------|--------|
| **1: Normal Attack** | Basic melee strike | None | Deals 1 to ATK damage (random range) |
| **2: Strong Attack** | Powerful but risky move | -3 HP | 75% success rate; deals 1.3x to 1.69x ATK damage on hit; costs 3 HP |
| **3: Defend** | Defensive stance | None | 40% success rate; reduces incoming damage by 60%; restores +5 HP on success |

**Monster Actions:**
- Each turn, monsters deal random damage within their attack range
- Damage = random(monsterMin, monsterMax)
- Special abilities can increase damage by 20%-50% depending on monster type

**Defend Mechanics:**
- Success Rate: 40%
- Damage Reduction: 60% (incoming damage × 0.4)
- Counter Attack: On successful defend, deal counter damage = (ATK × 0.4) + (monster_dmg × 0.4-0.6)
- HP Recovery: +5 HP restored on successful defense

**Special Abilities:**
- Trigger Chance: ~25% per turn
- Duration: 2-4 rounds
- Effects vary by monster type:
  - Ghost: Player ATK reduced by 50%
  - Chestnut: Monster damage +30%
  - Owl: Monster damage +40%
  - Blob: Monster damage +20%

### Movement System

**Controls:**
- **W or ↑**: Move North (row - 1)
- **S or ↓**: Move South (row + 1)
- **A or ←**: Move West (column - 1)
- **D or →**: Move East (column + 1)

**Movement Rules:**
- Cannot move through walls (#)
- Cannot move out of bounds
- Moving to undiscovered tiles reveals them

**First Visit Mechanic:**
- Visiting a tile for the first time triggers events
- Revisiting tiles does not trigger events

---

## Game World Symbols

### Map Display Symbols

| Symbol | Meaning | Color | Interaction |
|--------|---------|-------|-------------|
| **P** | Player position | Green (Bold) | Current location |
| **#** | Wall/Obstacle | Red (Bold) | Blocks movement; cannot pass through |
| **.** | Empty space | White | Safe to walk; may trigger events on first visit |
| **?** | Undiscovered tile | Yellow (Bold) | Fog of war; reveals when visited |
| **K** | Key location | Yellow | Pickup key - required to win |
| **T** | Trap | Yellow | Deals -15 HP damage |
| **C** | Treasure chest | Yellow | Grants +20 GOLD |
| **B** | Boss location | Yellow | Boss encounter; must defeat to access princess |
| **G** | Goal/Princess | Yellow | Win condition; need KEY to enter |

### Special Locations

1. **Starting Position (P)**: (0, 0) - Upper left corner
2. **Boss Location (B)**: (SIZE/2, SIZE/2) - Center of map
3. **Goal Location (G)**: (SIZE-1, SIZE-1) - Lower right corner
4. **Key Location (K)**: Random, guaranteed path to Goal
5. **Trap Location (T)**: Random dungeon hazard
6. **Chest Location (C)**: Random treasure location

---

## Events System

When you step on an empty tile (.) for the first time, a random event is triggered. The event type is determined by a roll of 1-100:

### Event Distribution (Probability)

| Roll | Event | Probability |
|------|-------|-------------|
| 1-25 | Monster Encounter | 25% |
| 26-40 | Merchant Shop | 15% |
| 41-55 | Mushroom Discovery | 15% |
| 56-100 | Empty Space (Safe) | 45% |

### Event 1: Monster Encounter (25% chance)

**Trigger:** Roll 1-25

**Description:** You encounter a random monster from the 4 available types.

**Monster Selection:** Randomly chosen from: Ghost, Chestnut, Owl, Blob

**Combat Rewards:**
- Victory: +20 GOLD, +50 EXP
- Defeat: Game Over (HP = 0)

**Mechanics:** Standard turn-based combat system applies (see Battle System section)

---

### Event 2: Merchant Shop (15% chance)

**Trigger:** Roll 26-40

**Description:** You encounter a helpful merchant selling mushroom potions.

**Available Items:**

| Item | Cost | Effect | Mechanic |
|------|------|--------|----------|
| **1. Normal Mushroom** | 30 GOLD | +15 HP | Restores health pool |
| **2. Herbal Mushroom** | 70 GOLD | +40 HP | Powerful healing potion |
| **3. Attack Mushroom** | 50 GOLD | +3 ATK | Permanently increases attack |
| **4. Defense Mushroom** | 40 GOLD | +2 DEF | Permanently increases defense |

**Purchase Mechanics:**
- Can only buy if you have sufficient GOLD
- Insufficient funds displays error and returns to main game
- Purchased items take effect immediately
- Stat increases are permanent for current run

---

### Event 3: Mushroom Discovery (15% chance)

**Trigger:** Roll 41-55

**Description:** You discover a wild mushroom in the maze with various effects.

**Random Mushroom Types:**

| Mushroom | Probability | Visual | Effect |
|----------|-------------|--------|--------|
| **Normal Mushroom** | 40% (of 41-55) | White (-) | +15 HP |
| **Herbal Mushroom** | 10% (of 41-55) | Green (O) | +40 HP |
| **Attack Mushroom** | 10% (of 41-55) | Red (X) | +3 ATK |
| **Defense Mushroom** | 40% (of 41-55) | Blue (U) | +2 DEF |

**Mechanics:**
- Free to pick up (no cost)
- Effects apply immediately
- Provides random stat boost

---

### Event 4: Safe Passage (45% chance)

**Trigger:** Roll 56-100

**Description:** The tile is empty and safe. No event occurs.

**Mechanics:** None; continue exploring freely

---

## Items & Collectibles

### Permanent Items

| Item | Symbol | Location | Effect | Requirement |
|------|--------|----------|--------|-------------|
| **Key** | K | Random (non-critical path) | Unlocks access to Goal | Required to win |

### Consumable/Event Items

#### Mushrooms (Found or Purchased)

**Normal Mushroom**
- Cost: 30 GOLD (shop) / Free (found)
- Effect: +15 HP
- Rarity: Common

**Herbal Mushroom**
- Cost: 70 GOLD (shop) / Common (found)
- Effect: +40 HP
- Rarity: Uncommon

**Attack Mushroom**
- Cost: 50 GOLD (shop) / Rare (found)
- Effect: +3 ATK (permanent for run)
- Rarity: Rare

**Defense Mushroom**
- Cost: 40 GOLD (shop) / Rare (found)
- Effect: +2 DEF (permanent for run)
- Rarity: Rare

### Currency

**GOLD ($)**
- Starting Amount: 10
- Uses: Shop purchases
- Sources:
  - Treasure Chests: +20 GOLD
  - Monster Defeats: +20 GOLD
- Cannot be found lying around

### Experience Points (EXP)

- Source: Defeating monsters (+50 EXP per monster, varies by difficulty)
- Accumulation: Automatically collected after battle
- Level Up Threshold: 100 EXP
- Carryover: Excess EXP carries to next level

---

## Difficulty Levels

Choose difficulty before starting a new game. Each difficulty affects:
- Map size
- Monster attack range
- Boss attack range

### Difficulty Settings

| Difficulty | Map Size | Monster ATK Range | Boss ATK Range | Description |
|------------|----------|-------------------|----------------|-------------|
| **1. Easy** | 9×9 | 5-10 | 10-15 | Small maze, weak enemies |
| **2. Normal** | 12×12 | 8-12 | 12-18 | Balanced gameplay |
| **3. Hard** | 15×15 | 10-15 | 15-22 | Large maze, strong enemies |
| **4. Hell** | 20×20 | 12-18 | 18-25 | Largest maze, powerful enemies |

**Map Generation:**
- 25% chance each tile is a wall
- Boss always spawns at (SIZE/2, SIZE/2)
- Goal always spawns at (SIZE-1, SIZE-1)
- Key, Trap, and Chest spawn at random safe locations
- Guaranteed paths connect: Player → Key → Goal, Player → Boss, etc.

---

## Battle System

### Turn Structure

**Player's Turn:**
1. Choose action: 1 (Normal), 2 (Strong), 3 (Defend)
2. Damage/effect is calculated and applied to monster
3. Monster special ability may trigger

**Monster's Turn:**
1. Monster deals damage to player
2. Damage is modified by player's DEF or ability effects
3. Display results

### Damage Calculation

**Player Normal Attack:**
```
Damage = random(1, ATK)
```
- Result includes debuff effects if active

**Player Strong Attack:**
```
Cost: -3 HP
Success Rate: 75%
On Success: Damage = random(1.30 to 1.69 × ATK)
On Failure: Damage = 0
```

**Monster Attack:**
```
Damage = random(monsterMin, monsterMax)
Adjusted by: Monster abilities, Player DEF, Defend success
```

**Defend Success:**
```
Success Rate: 40%
On Success:
  - Incoming Damage × 0.4
  - Counter Damage = (ATK × 0.4) + (Monster_Damage × 0.4-0.6)
  - Player HP + 5 healing
  - Kills monster immediately if counter damage >= monster HP
On Failure:
  - Incoming Damage × 0.4
  - No healing or counter
```

### Monster Special Abilities

**Trigger Mechanic:**
- Check every turn: 25% chance to activate
- Once triggered: Lasts 2-4 rounds
- Duration: Counts down each turn
- Minimum: 1 player attack after ability ends to trigger again

**Ghost's Incorporeal Form:**
- Effect: Player ATK reduced to 50% for duration
- Damage Multiplier: 0.5x on player attacks
- Duration: 2-4 turns

**Chestnut's Spore Burst:**
- Effect: Monster damage increased by 30%
- Damage Multiplier: 1.3x on monster attacks
- Duration: 2-4 turns

**Owl's Fire Blow:**
- Effect: Monster damage increased by 40%
- Damage Multiplier: 1.4x on monster attacks
- Duration: 2-4 turns

**Blob's Eat Money:**
- Effect: Monster damage increased by 20%
- Damage Multiplier: 1.2x on monster attacks
- Duration: 2-4 turns

### Win/Lose Conditions

**Monster Dies When:**
- HP ≤ 0 (from player attack or counter)
- Rewards: +20 GOLD, +50 EXP, Auto Level-Up check

**Player Dies When:**
- HP ≤ 0 (from monster attack)
- Game Over triggered
- Options: Return to Title or Quit Game

---

## Progression System

### Level-Up Mechanics

**How to Level Up:**
1. Collect 100 EXP from defeating monsters
2. EXP counter automatically triggers level up
3. Stat increases applied immediately

**What Happens:**
```
Current Level → Level + 1
EXP: Current - 100 → (Remainder carries over)
HP: Current + 20 (Max also increases by 20)
ATK: Current + 5
DEF: Current + 3
Display: "Level Up! Lv X HP+20 ATK+5 DEF+3"
```

**Level Up Progression Example:**

| Level | HP | ATK | DEF | EXP Needed | Total EXP |
|-------|----|----|-----|------------|-----------|
| 1 | 100 | 8 | 5 | 0 | 0 |
| 2 | 120 | 13 | 8 | 100 | 100 |
| 3 | 140 | 18 | 11 | 100 | 200 |
| 4 | 160 | 23 | 14 | 100 | 300 |
| 5 | 180 | 28 | 17 | 100 | 400 |

### Mid-Game Progression

**Early Game (Levels 1-2):**
- Focus on finding the Key
- Build up GOLD for shop purchases
- Avoid unnecessary fights if possible

**Mid Game (Levels 2-4):**
- Participate in more fights
- Use shops to buy powerful mushrooms
- Prepare for Boss encounter

**Late Game (Level 4+):**
- Seek out Boss encounter
- Have sufficient HP pool (160+)
- Possess Key for final objective

### Win Condition

**Victory Requirements:**
1. Navigate to Goal location (G) at (SIZE-1, SIZE-1)
2. Have Key in inventory
3. Enter Princess room minigame
4. Catch the princess within step limit

**Princess Room Minigame:**

**Map:**
- 15×15 enclosed room with walls and open paths
- Player spawns at (1, 1)
- Princess spawns at (7, 7)

**Controls:**
- W/A/S/D to move
- Arrow keys accepted

**Mechanics:**
- Movement costs 1 step
- Every 2 player steps, princess moves randomly
- Step limit = 12 + (HP / 10) maximum steps
- Easy difficulty (tutorial): 15 step limit

**Win:**
- Catch princess (same position)
- Display: "VICTORY! You caught the princess!"

**Lose:**
- Exceed step limit
- Display: "FAILED! The princess ran away..."
- Result: HP = 0 → Game Over

---

## Additional Game Features

### Save System

**Auto-Save Mechanics:**
- Saves after every move
- Saves character state, map state, difficulty settings
- Login-based save system (username-specific)
- Players can load previous progress

**Save Data Includes:**
- Current HP, ATK, DEF, GOLD, EXP, LEVEL
- Key status
- Map layout
- Discovered tiles
- Map size and difficulty parameters

### Tutorial System

**Tutorial Mode:**
- Offered at game start (optional)
- Guided 5×5 demo maze
- Teaches movement, combat, key pickup
- Demo minigame at exit
- Sample encounters with tutorial monster

**Tutorial Steps:**
1. Learn movement (W/A/S/D)
2. Fight a practice enemy (B)
3. Find the key (K)
4. Reach the goal (G)
5. Play minigame to catch enemy

### User Authentication

**Login/Registration:**
- Players create username/password on startup
- Authentication persists save data
- Failed authentication prevents game access

### Map Rendering

**Display Features:**
- Grid-based ASCII representation
- Cell borders (+ | -)
- Color-coded symbols:
  - Green: Player (P)
  - Red: Walls (#)
  - Yellow: Items/Goals/Undiscovered (?, K, T, C, B, G)
  - White: Empty spaces (.)
- Real-time updates as player moves
- Centered display for various terminal sizes

---

## Game Over & Retry

**Death Triggers:**
- HP drops to 0 or below (monster defeat or trap damage)

**Game Over Options:**
1. **Home**: Return to title screen
   - Start new game
   - Load different save file
2. **Quit**: Exit game completely

**Character Reset:**
- New game restarts with fresh stats
- Previous saves remain available for loading

---

## Controls Quick Reference

| Action | Control |
|--------|---------|
| Move North | W or ↑ |
| Move South | S or ↓ |
| Move West | A or ← |
| Move East | D or → |
| Combat: Normal | 1 |
| Combat: Strong | 2 |
| Combat: Defend | 3 |
| Confirm/Wait | ENTER |
| Help | Click HELP button |
| Home | Click HOME button |
| Quit | Click QUIT button |

---

## Strategy Tips

### Combat Tips
1. **Strong Attack** is risky but powerful (75% success, 1.3-1.69x damage, -3 HP)
2. **Defend** is reliable (40% success, heals +5 HP on success, counters for damage)
3. Use **Defend** when HP is low to potentially heal and counter-kill
4. **Normal Attack** is consistent but lowest damage

### Resource Management
1. Purchase **Herbal Mushrooms** in shops for emergency healing
2. Buy **Attack/Defense Mushrooms** to permanently boost stats
3. Collect GOLD from Chests (+20) and monster defeats (+20)
4. Plan route to minimize dangerous encounters early

### Map Navigation
1. Reveal safe paths first before exploring heavily-walled areas
2. Map paths to Key → Goal to plan optimal route
3. Boss fight is mandatory; prepare beforehand
4. Track locations of beneficial items visually

### Difficulty Scaling
- **Easy (9×9)**: Good for learning game mechanics
- **Normal (12×12)**: Balanced challenge and exploration
- **Hard (15×15)**: Requires strategic stat-building
- **Hell (20×20)**: Advanced players only; high difficulty

---

## Game Balance Notes

### Monster Power Scaling by Difficulty

| Difficulty | Monster Avg Damage | Boss Avg Damage | Max Monster Damage |
|------------|-------------------|-----------------|-------------------|
| Easy | 7.5 | 12.5 | 10 |
| Normal | 10 | 15 | 12 |
| Hard | 12.5 | 18.5 | 15 |
| Hell | 15 | 21.5 | 18 |

### Player Power Growth
- Strong Attack is 25-30% more powerful when it hits
- Defend provides 60% damage reduction (effective 40% damage)
- Counter from Defend can deal massive damage (up to 180% monster damage)
- Level-ups provide consistent +5 ATK per level

### Key Item Balance
- Key is the bottleneck for victory
- Guaranteed reachable path exists from start to key to goal
- Boss doesn't block key path; must be defeated to proceed to princess

---

## Changelog

**Current Version:** 1.0

**Core Features Implemented:**
- ✓ 4 unique monster types with special abilities
- ✓ 4 difficulty levels with scaling parameters
- ✓ Turn-based combat system
- ✓ Event system (25% monsters, 15% shop, 15% mushrooms, 45% safe)
- ✓ Level-up progression system
- ✓ Save/load system with authentication
- ✓ Princess room minigame
- ✓ Tutorial system
- ✓ Special abilities with visual feedback

---

**End of Game Manual**

*For further assistance or bug reports, please contact the development team.*
