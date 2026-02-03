**POKEMON: EMERALD HERALD**

**BEARER OF THE CURSE**

*Complete Design & Feature Bible*

Version 1.0

January 2026

# **1\. Executive Summary**

## **Project Vision**

Pokemon Emerald Herald is a vanilla-plus romhack that transforms Pokemon Emerald into a challenging, replayable experience inspired by Soulsborne design philosophy. The hack maintains the core Hoenn journey while introducing procedurally-generated curses that create unique playthroughs, competitive-grade battles, and mysterious Legacy Dungeons.

## **Core Pillars**

* Challenging but Fair: Elden Ring-style difficulty with powerful tools available to players

* High Replayability: Procedural Curse system creates unique runs from Trainer ID seed

* Competitive Battles: NPCs use full competitive teams with items, EVs, and strategic movesets

* Quality of Life First: Minimize grinding, maximize team experimentation

* Respectful Enhancement: Preserve Emerald's story beats while elevating gameplay

## **Key Differentiators**

* Bearer of the Curse: Unique procedural Bane/Boon system (50 each, pick 3+3)

* Legacy Dungeons: Three impossible-space dungeons with Curse removal rewards

* Seeded Encounters: 20% of wild encounters determined by player's unique seed

* No Item Spam: Competitive-style battles without mid-battle healing items

* Early Access to Power: High-tier Pokemon and legendaries available throughout

## **Target Audience**

Experienced Pokemon players seeking challenge and replayability. Players who enjoy:

* Soulsborne games (Dark Souls, Elden Ring)

* Competitive Pokemon formats

* Roguelike elements and procedural generation

* Nuzlocke challenges

* Strategic team building and optimization

# **2\. The Curse System**

## **Overview**

The Curse System is the signature mechanic that drives replayability. Each new game generates a unique combination of Banes (debuffs) and Boons (buffs) based on the player's Trainer ID, creating dramatically different playthroughs.

## **Activation Flow**

* Player encounters mysterious 'Curse Keeper' NPC before receiving first Pokemon

* Keeper presents 3 randomly-generated Bane/Boon sets based on Trainer ID seed

* Player can mulligan/reroll 2-3 times before committing

* Advanced: Manual seed input for deterministic runs (speedrun routing)

## **Core Mechanics**

Pool Structure:

* 50 unique Banes (debuffs/constraints)

* 50 unique Boons (powerful buffs)

* Each playthrough: 3 Banes \+ 3 Boons

* All effects persist entire playthrough unless removed

Modification System:

* Curse Shop: Voluntarily add Banes for rewards (TMs, rare items)

* Legacy Dungeons: Complete to remove 1 chosen Bane

* Wagering: Add 1 Bane to receive 1 additional random Boon

* Pure RNG: No intentional synergies between Banes/Boons

## **Example Banes (30+ more in full pool)**

* Frail Constitution: Max HP \-20% for all Pokemon

* Arcane Weakness: Super-effective moves deal 1.5x instead of 2x

* Poverty: All shop prices doubled

* Unstable Evolution: Evolution stones required for trade evolutions, evolutions delayed 5 levels

* Type Lock: Cannot use \[randomly selected type\] in battles

* Momentum Loss: Pivot moves (U-turn, Volt Switch) deal half damage

* Sluggish Reflexes: Accuracy multiplied by 0.95

* Slow Learner: Experience gain multiplied by 0.8

* Brittle Items: Held items have 20% chance to break after battle

* Curse of Binding: Cannot switch Pokemon during battle

* Weather Vulnerability: Take 1/16 HP damage per turn in weather

* Shaky Hands: Critical hit ratio reduced by 50%

* Limited Reserves: Can only carry 4 Pokemon (2 slots locked)

* Type Deficiency: One randomly selected type deals \-20% damage

* Stat Curse: One stat (Atk/Def/SpA/SpD/Spe) reduced by 15% across all Pokemon

## **Example Boons (30+ more in full pool)**

* Ancient Bloodline: Start with random pseudo-legendary egg

* Draconic Heritage: Start with guaranteed Dragon-type legendary/pseudo

* Fey Ancestry: Start with guaranteed Fairy-type legendary

* Mystic Sight: See opponent's moves/ability before battle

* Quick Learner: \+50% EXP gain

* Weather Attunement: Your weather effects last 2 extra turns

* Critical Fortune: \+10% critical hit rate on all moves

* Type Mastery: One randomly selected type deals \+20% damage

* Treasure Hunter: 2x item find rate from trainers/wild Pokemon

* Battle Hardened: All Pokemon gain \+10% to all stats

* Speed Demon: \+1 priority to all attacking moves

* Tank Blessing: \+30% HP for all Pokemon

* Elemental Affinity: Immune to one randomly selected type

* Lucky Charm: Shiny encounter rate 4x higher

* Instant Mastery: Start with 5 random competitive TMs

## **Technical Implementation Notes**

Configuration Approach:

* Use flag system in save data to track active Banes/Boons

* Curse Keeper NPC uses script to generate seed-based selection

* Battle engine hooks to apply multipliers/modifications

* UI display shows active Curses on Trainer Card/Status screen

Seed Generation:

* Trainer ID â†’ Hash function â†’ Deterministic Bane/Boon indices

* Store active curse indices in save file for persistence

* Manual seed input stores custom hash value

# **3\. Battle & Difficulty Design**

## **Design Philosophy**

Difficulty does not equal grinding. Battles should be:

* Strategically challenging (team composition, move selection)

* Fair and telegraphed (no unfair RNG, clear type matchups)

* Rewarding player knowledge (competitive mechanics, coverage)

* Respectful of player time (no forced grinding, instant team building)

## **Level Curve**

| Milestone | Level Range | Notes |
| :---- | :---- | :---- |
| Gym 1 (Roxanne) | 12-14 | Full team available |
| Gym 2 (Brawly) | 18-20 |  |
| Gym 3 (Wattson) | 24-26 | Skill check/wall |
| Gym 4 (Flannery) | 32-34 |  |
| Gym 5 (Norman) | 40-42 | Major difficulty spike |
| Gym 6 (Winona) | 46-48 |  |
| Gym 7 (Tate & Liza) | 52-54 |  |
| Gym 8 (Wallace) | 58-60 |  |
| Elite Four | 72-75 | Champion at 76-78 |
| E4 Rematch | 95-100 | Full legendary/OU teams |

## **Trainer AI & Teams**

Major Battles (Gym Leaders, Elite Four, Rivals):

* Full 6-Pokemon teams from Gym 2+ onwards

* Competitive-grade: Proper EVs, optimal natures, held items

* Coverage moves to counter weaknesses (e.g., Flannery's Torkoal with SolarBeam)

* Strategic AI: Switches on bad matchups, uses setup moves intelligently

* Gym 3+ can include legendaries (e.g., Flannery with Entei, Winona with Articuno)

Regular Trainers:

* Scale with location and story progression

* 2-4 Pokemon teams with some held items

* Improved movesets (no Tackle spam)

## **Boss Rematches**

* Fixed high-level challenges (not level-scaled)

* Available immediately after unlocking

* Can be attempted at any level (Elden Ring's Tree Sentinel approach)

* Elite Four rematch at level 95-100 with top-tier OU/Uber teams

## **Battle Restrictions**

Item Usage:

* Bag healing items (Potions, Revives) BANNED during trainer battles

* Full heal available between battles (mini Pokemon Center concept)

* Pokeballs always allowed (wild encounters)

* X-Items (stat boosters) allowed for tactical use

* NPCs follow same restrictions (mutual fairness)

Held Items:

* All held items available and encouraged

* Healing berries (Sitrus, Oran) remain powerful

* Competitive items prioritized (Life Orb, Choice items, Assault Vest, etc.)

## **Intentionally Powerful Tools**

Players have access to 'broken' strategies:

* Setup sweepers: Shell Smash, Dragon Dance, Quiver Dance readily available

* High-tier Pokemon: Pseudo-legendaries, legendaries available early

* Competitive movesets: All Pokemon get their best moves

* NPCs have the same tools: Fairness in both directions

# **4\. Pokemon Availability & Encounters**

## **Generation Scope**

* Generations 1-7 fully implemented (Kanto through Alola)

* Gen 8-9 stretch goal for future updates

* Regional forms included (Alolan variants)

* Expanded move pools with all generations' moves

## **Starter Selection**

* Randomly determined by Curse seed (not player choice)

* Bloodline Boons override: Draconic gives Dragon starter, Fey gives Fairy, etc.

* Example pool: All generation starters \+ select other Pokemon

## **Wild Encounter System**

Hybrid Encounter Tables:

* 80% slots: Standardized encounters (curated for type/level diversity)

* 20% slots: Seed-determined encounters (unique to each player's Curse)

* Implementation: Each route has 10 slots; 8 fixed, 2 procedural

Early Route Improvements:

* Route 101-103: 15+ possible encounters (no more Zigzagoon/Wurmple spam)

* Type diversity from the start (Water, Fire, Grass, Electric, Fighting all available early)

* Rare encounters include pseudo-legendaries (e.g., Bagon on Route 115\)

## **Legendary Availability**

Story Legendaries:

* Groudon/Kyogre/Rayquaza remain story-locked (preserve Emerald narrative)

* Lati@s events unchanged

Other Legendaries:

* Integrated into gym leader teams from Gym 3+ onwards

* Catchable in Legacy Dungeons as rewards

* Some available as rare wild encounters in postgame areas

* Elite Four rematch teams include box legendaries

## **Shiny Odds**

* Adjustable via in-game setting (Options menu)

* Default: 1/512 (increased from vanilla 1/8192)

* Lucky Charm Boon: 1/128

* Accessibility for shiny hunters without compromising challenge

# **5\. Legacy Dungeons**

## **Overview**

Three major optional dungeons inspired by Elden Ring's Legacy Dungeons and classic cRPG dungeon crawls. These are sprawling, challenging areas with unique encounters and substantial rewards.

## **Design Principles**

* Optional side content (not required for main story)

* Accessible early but scale in difficulty

* Focus on expanded layouts and encounters (GBA technical feasibility)

* No story/event scripting changes (minimize romhacking complexity)

* Unique encounter tables with cross-gen legendaries

## **Dungeon 1: The Forgotten Vault (New Mauville)**

Location: New Mauville (expanded interior)

Recommended Level: 30-35

Theme: Abandoned electrical facility with maze-like corridors

Unique Mechanic: Power grid puzzle \- activate generators to unlock areas

Encounters:

* Electric/Steel type focus

* Rotating trainers with competitive Electric teams

* Wild encounters: Rotom forms, Magnezone line, Electivire, etc.

Rewards:

* Boss: Legendary encounter (Raikou, Zapdos, or Thundurus)

* Remove 1 chosen Bane

* TM: Thunder

* Item: Magnet

## **Dungeon 2: The Tidal Abyss (Shoal Cave)**

Location: Shoal Cave (significantly expanded)

Recommended Level: 50-55

Theme: Tide-based dungeon with accessible and flooded sections

Unique Mechanic: Tide cycles open different paths (time-based or script trigger)

Encounters:

* Water/Ice type focus

* High-level swimmers and deep-sea trainers

* Wild encounters: Legendary beasts (Suicune), Fossil Pokemon, Kyurem

Rewards:

* Boss: Choice of Suicune, Kyurem, or Lugia

* Remove 1 chosen Bane

* TM: Hydro Pump

* Item: Never-Melt Ice

## **Dungeon 3: The Labyrinth of Echoes (Granite Cave)**

Location: Granite Cave (massive expansion, multiple floors)

Recommended Level: 65-70

Theme: Multi-floor roguelike dungeon with randomized elements

Unique Mechanic: Mini-roguelike \- random encounters each floor, cannot leave mid-run

Encounters:

* Mixed types, competitive-tier trainers

* Boss trainers every 3 floors

* Wild encounters: All non-Gen3 box legendaries (Dialga, Palkia, Giratina, etc.)

Rewards:

* Boss: Player choice of any non-Gen3 box legendary

* Remove 1 chosen Bane

* TM: Choice of powerful move (Draco Meteor, Earth Power, etc.)

* Item: Master Ball

## **Technical Considerations**

* Expand existing maps rather than create new ones (reduce scripting)

* Reuse trainer classes with modified teams

* Legendary encounters use standard wild battle system

* Bane removal via NPC dialogue after boss defeat

# **6\. Quality of Life Features**

## **Overview**

Extensive QoL improvements to respect player time and enable experimentation. The motto: Difficulty through strategy, not tedium.

| Feature | Description | Priority |
| :---- | :---- | ----- |
| **Physical/Special Split** | Gen 4+ move categorization implemented | **P0** |
| **Infinite TMs** | All TMs reusable | **P0** |
| **Infinite Rare Candies** | Available early for instant leveling | **P0** |
| **HM Removal** | No field move requirements \- all Water types Surf, Flying types Fly, etc. | **P0** |
| **Toggleable EXP Share** | Modern always-on style, can be disabled | **P0** |
| **Reusable Items** | Key items never consumed | **P0** |
| **Fast Battle Text** | Instant text option in settings | **P1** |
| **Running Indoors** | Run everywhere | **P1** |
| **Quick PC Access** | Portable PC key item | **P1** |
| **Move Relearner Free** | Free and available early (Fallarbor) | **P0** |
| **Move Deleter Early** | Available from Gym 1 | **P0** |
| **Name Rater Available** | Rename Pokemon anytime | **P1** |
| **Evolution Stones Accessible** | Purchasable at all Poke Marts | **P0** |
| **Breeding QoL** | Eggs hatch faster, guaranteed gender/nature options | **P2** |
| **Visible IVs/EVs** | Display in summary screen | **P1** |
| **EV Training Items** | Power items available early, vitamins remove EV limit | **P0** |
| **Nature Mints** | Change nature post-catch (expensive) | **P2** |
| **Ability Capsule** | Change ability (expensive) | **P2** |
| **Perma-Death Option** | Nuzlocke mode \- fainted Pokemon auto-released | **P1** |

## **Implementation Priority**

P0 \= Critical (core experience)

P1 \= High Priority (significant QoL)

P2 \= Nice to Have (polish/stretch goals)

# **7\. Postgame Content**

## **Elite Four Rematch**

* Level 95-100 teams

* Full legendary/OU competitive teams

* Perfect IVs, optimal EVs, competitive movesets

* Steven's team includes box legendaries (Dialga, etc.)

## **Battle Frontier**

* Keep existing Battle Frontier functionality

* Update AI to use competitive strategies

* Low priority \- not a focus area

## **Superboss House**

Inspired by Pokemon Emerald Kaizo/Imperium:

* NPC house with 10+ developer/playtester trainers

* Each has a unique themed team (Monotype, Weather, Trick Room, etc.)

* Level 100, fully optimized teams

* Rewards: Master Balls, Rare Candies, competitive items

* One NPC equivalent to 'Red' \- ultimate challenge fight

## **The Sealed Chamber (Stretch Goal)**

Postgame-exclusive dungeon:

* Multi-floor challenge dungeon (50+ floors)

* Roguelike elements: random encounters, escalating difficulty

* Boss encounters every 10 floors

* Final floor: Arceus encounter at Level 100

* Requires completion of all 3 Legacy Dungeons to unlock

# **8\. Technical Implementation**

## **Base Framework**

* Pokemon Emerald Expansion (pret/pokeemerald-expansion)

* Provides: Physical/Special split, Fairy type, Gen 1-9 Pokemon, modern moves

* Benefits: Proven stable, extensive documentation, active community

## **Curse System Implementation**

Data Structure:

* Save file stores: Active Bane indices (3), Active Boon indices (3), Seed value

* Flag array: CURSE\_BANE\_1, CURSE\_BANE\_2, CURSE\_BANE\_3, CURSE\_BOON\_1, etc.

Battle Hooks:

* Modify damage calculation for multipliers (Frail Constitution, Type Mastery)

* Hook experience gain for Quick Learner/Slow Learner

* Hook shop prices for Poverty

* Hook accuracy calculation for Sluggish Reflexes

UI Display:

* New Trainer Card screen showing active Curses

* Party menu overlay for active buffs/debuffs

## **Seeded Encounters**

* Hash Trainer ID to generate deterministic species indices

* Route encounter tables: 8 fixed slots \+ 2 seeded slots

* Seeded slots pull from tier-appropriate species pool

* Bane/Boon modifiers can further filter/boost seed pool

## **Legacy Dungeons**

* Expand existing map data (New Mauville, Shoal Cave, Granite Cave)

* Add connection tiles to new sections

* Populate with trainers using existing trainer classes

* Boss encounters: NPC script triggers wild legendary battle

* Bane removal: Post-battle NPC dialogue sets flag, removes curse

## **AI Improvements**

* Modify AI evaluation functions to prioritize setup moves

* Implement switch logic for bad type matchups

* Improve move selection for coverage

* Reference: Radical Red's AI improvements

## **Testing & Balance**

Playtesting Focus Areas:

* Curse balance: Are any Banes/Boons game-breaking?

* Level curve: Can player progress without grinding?

* Difficulty spikes: Are walls fair and telegraphed?

* Seed variety: Do different seeds feel meaningfully different?

Metrics to Track:

* Average clear time for each gym

* Most/least used Pokemon

* Most/least rerolled Banes/Boons

* Legacy Dungeon completion rates

# **9\. Development Roadmap**

## **Phase 1: Foundation (Weeks 1-4)**

* Set up pokeemerald-expansion environment

* Implement basic Curse System data structures

* Create Curse Keeper NPC and selection UI

* Implement 10 Banes and 10 Boons (prototype pool)

* Test seed generation and persistence

## **Phase 2: QoL & Encounters (Weeks 5-8)**

* Implement all P0 QoL features

* Expand Bane/Boon pools to 30 each

* Create seeded encounter system

* Revise early route encounter tables

* Add infinite Rare Candies and evolution stones

## **Phase 3: Trainer Teams (Weeks 9-12)**

* Update all gym leader teams to level 6 Pokemon

* Add competitive movesets, items, EVs to major battles

* Improve AI for gym leaders and Elite Four

* Adjust level curve and difficulty

* Implement item usage restrictions

## **Phase 4: Legacy Dungeons (Weeks 13-16)**

* Design and implement Dungeon 1 (New Mauville)

* Design and implement Dungeon 2 (Shoal Cave)

* Design and implement Dungeon 3 (Granite Cave)

* Add legendary encounters and Bane removal rewards

* Populate with trainers and unique encounters

## **Phase 5: Polish & Balance (Weeks 17-20)**

* Complete Bane/Boon pools (50 each)

* Implement Curse Shop NPC

* Add Elite Four rematch teams

* Extensive playtesting and balance adjustments

* Implement P1 QoL features

## **Phase 6: Postgame & Release (Weeks 21-24)**

* Implement Superboss House

* Polish Battle Frontier integration

* Final balance pass on all major battles

* Documentation and player guide

* Public release and community feedback

## **Post-Release: Updates & Stretch Goals**

* The Sealed Chamber dungeon (if feasible)

* Expand to Gen 8-9 Pokemon

* Additional Banes/Boons based on feedback

* Community-requested features

* Bug fixes and balance patches

# **10\. Design Guidelines & Philosophy**

## **Guiding Principles**

### **Respect Player Time**

* No forced grinding: Rare Candies available immediately

* No RNG frustration: Good shiny odds, no item breakage without Bane

* Fast iteration: Infinite TMs, easy team building

### **Difficulty Through Depth**

* Strategic challenge: Type coverage, team composition, move selection

* Knowledge rewards: Understanding competitive mechanics gives advantage

* No artificial inflation: No bloated HP pools, no evasion spam

### **Replayability First**

* Curse System ensures every run feels different

* Seeded encounters create unique team compositions

* Multiple viable strategies for each challenge

### **Fair but Punishing**

* Players have access to the same tools as NPCs

* Telegraphed challenges: Type specialists use their type well

* Optional difficulty: Legacy Dungeons and Curses are player choice

## **What to Avoid**

* Forced backtracking or excessive HM usage

* Unclear objectives or cryptic puzzles

* Unavoidable damage or unfair RNG

* Artificially restricting player options

* Excessive story changes that alter Emerald's feel

## **Balance Philosophy**

Bane/Boon Design:

* Banes should create interesting constraints, not unfun restrictions

* Boons should feel powerful but not trivialize entire game

* Aim for 'meaningful impact' not 'complete invalidation'

Difficulty Curve:

* Early game: Accessible, teach mechanics

* Mid game: Introduce walls, test player knowledge

* Late game: Expect player mastery, full competitive strategies

* Postgame: Superboss-tier challenges for veteran players

## **Community Feedback Integration**

* Monitor difficulty spikes via community playthroughs

* Track most/least popular Curses for balance

* Be willing to nerf overperforming Boons or buff weak Banes

* Maintain open communication about design decisions

# **Appendices**

## **Appendix A: Complete Bane Pool (50 Examples)**

(Full implementation requires 50 unique Banes. Below are additional examples beyond the 15 listed earlier.)

* Curse of Vulnerability: \+20% damage taken from all attacks

* Restricted Arsenal: TMs cannot be used (only level-up moves)

* Weakened Instincts: Cannot see wild Pokemon levels before encounter

* Expensive Healing: Pokemon Center healing costs money

* Fragile Equipment: Pokeballs have 50% catch rate reduction

* ...and 30 more to be fully designed

## **Appendix B: Complete Boon Pool (50 Examples)**

(Full implementation requires 50 unique Boons. Below are additional examples beyond the 15 listed earlier.)

* Item Mastery: Start with all evolution stones

* Breeder's Touch: All eggs hatch with perfect IVs

* Nature Control: Choose nature when catching any Pokemon

* Sync Master: Synchronize works 100% of the time

* Shiny Magnet: Shiny clause \- guaranteed shiny on each route

* ...and 30 more to be fully designed

## **Appendix C: Recommended Testing Tools**

* mGBA emulator with debug features

* PoryMap for map editing

* Encounter table spreadsheet for balance tracking

* Damage calculator for competitive balance

* Community playtester Discord for feedback

## **Appendix D: Resources & References**

* pokeemerald-expansion GitHub: https://github.com/rh-hideout/pokeemerald-expansion

* Radical Red design philosophy

* Elden Ring Legacy Dungeon design principles

* Pokemon Showdown damage calculator

* Smogon competitive tier lists

# **Conclusion**

Pokemon Emerald: Bearer of the Curse aims to deliver a fresh, challenging, and endlessly replayable take on the classic Hoenn adventure. By combining Soulsborne difficulty philosophy, roguelike procedural generation, and respect for player time, this romhack creates a unique experience that rewards skill, knowledge, and experimentation.

The Curse System ensures no two playthroughs are identical, while Legacy Dungeons provide optional challenges for players seeking the ultimate test. With competitive-grade battles, extensive QoL improvements, and a commitment to fairness, Bearer of the Curse respects both veterans and newcomers alike.

This design bible serves as the definitive reference for development. As the project evolves, this document will be updated to reflect new features, balance changes, and community feedback.

*May your Curses be light and your Boons be strong.*
