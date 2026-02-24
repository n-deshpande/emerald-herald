# POKEMON: EMERALD HERALD
## Bearer of the Curse
### Complete Design & Feature Bible

*Version 2.0 — February 2026*

---

# 1. Executive Summary

## Project Vision

Pokemon Emerald Herald is a romhack that transforms Pokemon Emerald into a challenging, replayable experience inspired by Soulsborne design philosophy and roguelike games (Balatro, Risk of Rain 2). The hack maintains the core Hoenn journey while introducing a layered Relic system, Mark-based difficulty selection, and competitive-grade authored battles.

## Core Pillars

- **Challenging but Fair:** Elden Ring-style difficulty with powerful tools available to players. Difficulty through strategy, not tedium.
- **High Replayability:** Relic acquisition and mark selection create meaningfully different runs without requiring massive content pools.
- **Competitive Battles:** Static, thoughtfully authored trainer teams with full competitive sets. Difficulty tiers via marks.
- **Quality of Life First:** Minimise grinding, maximise team experimentation.
- **Respectful Enhancement:** Preserve Emerald's story beats while elevating gameplay.

## Key Differentiators

- **Mark System:** Pre-game difficulty ladder of permanent global obligations — opt-in, additive, player-authored challenge. Think Balatro's stake system of layered difficulty increases.
- **Relic System:** 5-slot in-run progression inspired by Balatro's joker economy. Acquire, upgrade, and trade relics across milestones.
- **Mark-Tiered Trainer Teams:** Major trainers have three authored team tiers selected by active mark level. Simple conditional logic, significant variety.
- **Wild Encounter Variance:** Expanded encounter tables with seed-influenced slots provide team-building diversity across runs.
- **Legacy Dungeons:** Three optional challenge dungeons rewarding rare Pokemon encounters.

## Target Audience

Experienced Pokemon players seeking challenge and replayability. Players who enjoy Soulsborne games (Dark Souls, Elden Ring), Balatro and roguelike card-builders, competitive Pokemon formats, and Nuzlocke challenges.

---

# 2. The Mark System

## Overview

Marks are permanent, global obligations chosen before the run begins at the new game screen when speaking to Professor Birch. They are the difficulty slider. Each mark is a known, predetermined effect — no surprises, no randomness. Players know exactly what they're signing up for.

Marks do **not** occupy relic slots. They are a separate layer that modifies the run's ruleset globally. Marks are additive,  taking mark 3 means mark 1 and 2 are also active.
##  The Mark Ladder

Note this is still subject to change

| Mark | Name           | Effect                                                                                                    |
| ---- | -------------- | --------------------------------------------------------------------------------------------------------- |
| 1    | *Worn Stone*   | Bag healing items (Potions, Revives) banned during all trainer battles. Baseline expectation of the hack. |
| 2    | *Hollow Bones* | All trainer Pokemon have competitive EVs, optimal natures, and held items.                                |
| 3    | *Iron Shackle* | Relic slot cap reduces from 5 to 4. Every relic choice matters more.                                      |
| 4    | *Blood Tithe*  | Pokemon Center healing costs money, scaling with badge count.                                             |
| 5    | *Cursed Blood* | Starter is randomly assigned from the full starter pool, unknown until received.                          |
| 6    | *The Abyss*    | Permadeath. Fainted Pokemon are released. Full Nuzlocke rules active.                                     |

Marks 1–3 represent the intended experience for the target audience. Marks 4–5 are for veterans seeking pressure. Mark 6 is the true souls-equivalent — brutal, self-imposed, unforgiving.

## Mark-Tiered Trainer Teams

Major trainers (gym leaders, rivals, Elite Four) have three authored team tiers selected by the player's active mark level. This is implemented as a simple conditional check against the mark bitmask — one flag read per trainer encounter, no randomness required.

- **Low tier** (mark 0–2): Tough but fair. Type-focused, competitive movesets, no legendaries.
- **Mid tier** (mark 3–4): Full competitive sun/rain/trick room strategies, coverage moves, some legendaries on ace slots.
- **High tier** (mark 5–6): Optimised OU/Uber teams. Legendaries throughout. Designed to punish.

Example — Flannery:
- Low: Competitive sun team, Torkoal with SolarBeam, Arcanine, Ninetales, etc.
- Mid: Drought setter, mixed offensive threats, Entei as ace.
- High: Choice Specs Reshiram. She wants you dead.

The design effort lives in the team spreadsheet, not the codebase. Three tiers across ~10 major trainers = ~30 authored teams total.

---

# 3. The Relic System

## Overview

RELICS are what are previously known as curses! This is a very important terminology change.

Relics are the primary in-run progression mechanic, directly inspired by Balatro's joker economy. The Pokemon are your deck. Relics are your jokers. You hold up to 5 simultaneously across dedicated slots, acquiring and refining them across the run's natural milestones.

There is no separate boon/bane distinction. All relics exist in a single pool. Some are straightforwardly positive, some are mixed with meaningful upsides and genuine downsides, and some are run-defining wildcards. Rarity communicates valence at a glance.

## Rarity Tiers

**Common (10 relics):** Simple, numerical, immediately readable. Mostly positive with minor costs. The backbone of most runs. Good for early acquisition and players who want predictability.

**Uncommon (4 relics):** Unique combo effects that interact with game systems in non-obvious ways. Mixed valence — real upside, real downside baked into the same effect. The interesting design space.

**Rare (3–4 relics):** Run-defining, potentially broken, high variance. Things people screenshot and share. Each one fundamentally changes how you approach the game.

A tier 3 common is roughly equivalent in power to a tier 1 uncommon, and a tier 3 uncommon roughly equivalent to a tier 1 rare. This creates a natural balance curve without deliberate tuning.

## Relic Tiers (Scaling)

Every relic has 3 tiers of potency — ideally simple numerical scaling on the same underlying effect. Upgrading increments the tier value in the save struct. Implementation is a lookup table per relic; no new battle hooks required beyond those already written for the base effect.

Example — *Adaptability* (common): Tier 1: +10% damage for STAB moves. Tier 2: +20%. Tier 3: +30%.

## Acquisition Cadence

**Start of game:** Draw 3 relic from the weighted pool, pick 1. This is your identity relic — it should immediately suggest a direction for the run. Draw weighting: 80% common, 15% uncommon, 5% rare.

**Milestone schedule across 8 gyms:**

| Milestone           | Event                                                   |
| ------------------- | ------------------------------------------------------- |
| Gym 1 — Roxanne     | Receive relic, fills slot 2. Passive acquisition.       |
| Gym 2 — Brawly      | Receive relic, fills slot 3. Passive acquisition.       |
| Gym 3 — Wattson     | **Wager opportunity.** First meaningful decision point. |
| Gym 4 — Flannery    | Receive relic, fills slot 4. Passive acquisition.       |
| Gym 5 — Norman      | **Wager opportunity.** Mid-run refinement begins.       |
| Gym 6 — Winona      | Receive relic, fills slot 5. All slots now full.        |
| Gym 7 — Tate & Liza | **Wager opportunity.**                                  |
| Gym 8 — Wallace     | **Wager opportunity.**                                  |
| Elite Four          | **Final wager.** Highest rarity weighting on draws.     |

Passive acquisition draws are weighted 80/15/5. The first half of the run is accumulation. The second half is optimisation. This mirrors how Pokemon team-building naturally feels as your composition crystallises.

## The Wager Mechanic

At wager milestones, two options are available — or take nothing and move on unchanged.

**Trade up:** Sacrifice one held relic, receive a random relic of guaranteed higher rarity at tier 1. You are gambling known value for unknown potential. A tier 3 common you've built around versus a tier 1 rare with untested synergy. This is the core tension.

**Upgrade:** Increment one held relic from its current tier to the next. No new relic, no new risk — pure deepening of an existing commitment. Not available if the relic is already at tier 3.

Taking nothing is always valid and never penalised. Sometimes your build is working and you leave it alone.

## Save Structure

```c
// Per slot: relic identity + current tier
u8 curse_slot_id[5];    // index into curse pool
u8 curse_slot_tier[5];  // 0-2, maps to tier 1-3
u8 active_marks;       // bitmask, marks 1-6
```

Relic effects resolve via lookup: `relic_effects[id][tier]` returns the relevant multiplier or flag. Battle hooks read these at calculation time.

## Relic Pool (To Be Fully Designed)

The target pool is approximately 10 common, 4 uncommon, 3–4 rare. Every entry should feel handcrafted. Examples:

**Common (simple, global ability effects):**
- *Hustle:* +Atk%, -Accuracy%
- *Adaptability:* Increased STAB multiplier
- *Swift Swim / Chlorophyll / Sand Rush:* Speed boost in relevant weather
- *Thick Fat:* Reduced damage from Fire/Ice
- *Quick Learner:* +EXP% gain
- *Critical Fortune:* +Critical hit rate

**Uncommon (complex, mixed valence):**
- *Glass Cannon:* +30% all damage dealt, -30% all HP
- *Momentum:* First move each battle gets +1 priority, all subsequent moves lose 1 priority
- *Blood Price:* Winning a battle restores HP to full; losing a battle costs money equal to prize money

**Rare (run-defining, potentially broken):**
- *Speed Demon:* +1 priority on all attacking moves
- *Cursed Inheritance:* When a Pokemon faints, the next Pokemon out inherits all its stat boosts

---

# 4. Battle & Difficulty Design

## Design Philosophy

Difficulty does not equal grinding. Battles should be strategically challenging, fair and telegraphed, rewarding of player knowledge, and respectful of player time.

## Level Curve

| Milestone | Level Range | Notes |
|-----------|-------------|-------|
| Gym 1 (Roxanne) | 12–14 | Full team available from start |
| Gym 2 (Brawly) | 18–20 | |
| Gym 3 (Wattson) | 24–26 | Skill check / first wall |
| Gym 4 (Flannery) | 32–34 | |
| Gym 5 (Norman) | 40–42 | Major difficulty spike |
| Gym 6 (Winona) | 46–48 | |
| Gym 7 (Tate & Liza) | 52–54 | |
| Gym 8 (Wallace) | 58–60 | |
| Elite Four | 72–75 | Champion at 76–78 |
| E4 Rematch | 95–100 | Full legendary/OU teams |

## Authored Setpiece Battles

A small number of battles receive extra design attention to be genuine Ornstein and Smough moments — encounters so well designed that players talk about them specifically. Candidates: Norman (already has a reputation), Tate & Liza (double battle format is inherently brutal), Sidney (redesigned to be a genuine wall). Everything else is competent and challenging without needing to be legendary. 

## Battle Restrictions

Bag healing items (Potions, Revives) banned during all trainer battles (baseline mark 1 effect). Full heal available between battles via Pokemon Centers. Pokeballs always allowed in wild encounters. X-Items allowed for tactical use. NPCs follow the same restrictions for mutual fairness.

Held items fully available and encouraged. Competitive items prioritised (Life Orb, Choice items, Assault Vest, Sitrus Berry, etc.).

## Intentionally Powerful Tools

Players have access to powerful strategies. Setup sweepers (Shell Smash, Dragon Dance, Quiver Dance) readily available. High-tier Pokemon including pseudo-legendaries available throughout. All Pokemon have access to their competitive movesets. NPCs at higher marks tiers have the same tools — fairness operates in both directions.

---

# 5. Pokemon Availability & Encounters

## Generation Scope

Generations 1–9 fully implemented (Kanto through Alola) via pokeemerald-expansion. Regional forms included. Expanded move pools across all generations.

## Starter Selection

At marks 0–4, player selects from the standard three Hoenn starters. At mark 5 (*Cursed Blood*), starter is randomly assigned from the full multi-generational starter pool, determined at the new game screen and revealed only when received from Birch.

## Wild Encounter System

Encounter tables use a hybrid slot model: 8 fixed slots per route (curated for type and level diversity) and 2 seed-influenced slots (determined by Trainer ID hash, unique per run). This provides meaningful team-building variance across runs without full randomisation.

Early routes (101–103) have expanded encounter pools with genuine type diversity from the start — no Zigzagoon/Wurmple spam. Rare slots include pseudo-legendaries (e.g., Bagon on Route 115).

## Shiny Odds

Adjustable via Options menu. Default: 1/512. *Critical Fortune* relic at tier 3 can include a shiny rate bonus. Accessibility for shiny hunters without compromising challenge.

---

# 6. Legacy Dungeons

## Overview

Three optional dungeons built on expanded versions of existing Emerald maps. Not required for main story progression. Accessible early but scale in difficulty. Reward: rare legendary Pokemon encounter. No Bane removal mechanic — the mark and relic systems handle all progression rewards.

## Dungeon 1: The Forgotten Vault (New Mauville)

Recommended level 30–35. Electric/Steel type focus. Expanded maze layout with power grid puzzle. Boss reward: choice of Raikou, Zapdos, or Thundurus encounter.

## Dungeon 2: The Tidal Abyss (Shoal Cave)

Recommended level 50–55. Water/Ice type focus. Tide-cycle mechanic opens different paths. Boss reward: choice of Suicune, Kyurem, or Lugia encounter.

## Dungeon 3: The Labyrinth of Echoes (Granite Cave)

Recommended level 65–70. Multi-floor structure, mixed types, boss trainers every 3 floors. Cannot leave mid-run. Boss reward: player choice of any non-Gen3 box legendary.

---

# 7. Quality of Life Features

| Feature | Description                                   | Priority |
|---------|-------------|----------|
| Physical/Special Split | Gen 4+ move categorisation                    | P0 |
| Infinite TMs | All TMs reusable                              | P0 |
| Infinite Rare Candies | Available early for instant levelling         | P0 |
| HM Removal | No field move requirements                    | P0 |
| Toggleable EXP Share | Modern always-on style, can be disabled       | P0 |
| Move Relearner Free | Free, available from Fallarbor                | P0 |
| Move Deleter Early | Available from Gym 1                          | P0 |
| Evolution Stones | Purchasable at all Poke Marts                 | P0 |
| EV Training Items | Power items early, vitamins remove EV cap     | P0 |
| Fast Battle Text | Instant text option in settings               | P1 |
| Running Indoors | Run everywhere                                | P1 |
| Quick PC Access | Portable PC key item                          | P1 |
| Visible IVs/EVs | Display in summary screen                     | P1 |
| Name Rater | Rename Pokemon anytime                        | P1 |
| Perma-Death Option | Nuzlocke mode (also activated by mark 6)      | P1 |
| Nature Mints | Change nature post-catch (expensive)          | P2 |
| Ability Capsule | Change ability (expensive)                    | P2 |
| Breeding QoL | Faster eggs, guaranteed gender/nature options | P2 |

P0 = Critical. P1 = High priority. P2 = Nice to have.

---

# 8. Postgame Content

## Elite Four Rematch

Level 95–100 teams. Full legendary/OU competitive sets. Perfect IVs, optimal EVs. Steven's team includes box legendaries. Uses high-mark tier authored teams regardless of player's mark selection.

## Superboss House

NPC house with 10+ developer/playtester trainers. Each has a unique themed team (Monotype, Weather, Trick Room, etc.). Level 100, fully optimised. Rewards: Master Balls, Rare Candies, competitive items. One NPC equivalent to Red — the ultimate challenge.

## Battle Frontier

Existing Battle Frontier preserved with updated AI using competitive strategies. Low priority — not a focus area for v1.

## The Sealed Chamber *(Stretch Goal)*

Postgame-exclusive 50+ floor roguelike dungeon. Requires completion of all three Legacy Dungeons to unlock. Boss encounters every 10 floors. Final floor: Arceus at level 100.

---

# 9. Technical Implementation

## Base Framework

pokeemerald-expansion (pret/pokeemerald-expansion). Provides Physical/Special split, Fairy type, Gen 1–9 Pokemon, modern moves, proven stability, active community documentation.

## Relic System

(previously known as curses)

```c
u8 curse_slot_id[5];    // index into relic pool (0-255)
u8 curse_slot_tier[5];  // 0-2, tier 1-3
u8 active_marks;       // bitmask for marks 1-6
```

Effects resolve via `curse_effects[id][tier]` lookup table. Battle hooks read this at damage calculation, EXP gain, accuracy calculation, and shop price points. UI displays active relics.

## Mark-Tiered Trainer Teams

Trainer team selection reads `active_marks` bitmask at battle start. Three team arrays per major trainer: `trainer_teams_low[id]`, `trainer_teams_mid[id]`, `trainer_teams_high[id]`. Selection: low for marks 0–2, mid for 3–4, high for 5–6. Minimal runtime overhead, all complexity in data.

## Seeded Encounters

Trainer ID hashed to generate deterministic indices for the 2 seed-influenced encounter slots per route. Seed-influenced slots pull from a tier-appropriate species pool. Hash stored in save file; manual seed input supported for deterministic runs and speedrun routing.

## Legacy Dungeons

Expand existing map data. Add connection tiles to new sections. Legendary encounters use standard wild battle system triggered by NPC script after boss defeat. No Bane removal logic required — dungeons reward Pokemon only.

## AI Improvements

Modify AI evaluation functions to prioritise setup moves. Implement switch logic for bad type matchups. Improve coverage move selection. Reference: Radical Red AI implementation.

---

# 10. Design Guidelines & Philosophy

## Guiding Principles

**Respect Player Time.** No forced grinding. Rare Candies available immediately. Fast iteration via infinite TMs and easy team building.

**Difficulty Through Depth.** Strategic challenge via type coverage, team composition, and move selection. Understanding competitive mechanics gives a genuine advantage. No bloated HP pools, no evasion spam.

**Replayability Through Variance, Not Volume.** A small, well-designed relic pool with mark-tiered authored opponents creates more meaningful runs than a large shallow pool. Every cursor entry should feel handcrafted and worth reading.

**Fair but Punishing.** Players have access to the same tools as NPCs. Challenges are telegraphed. Optional difficulty systems (marks, Legacy Dungeons) are always player-authored.

## What to Avoid

- Forced backtracking or HM gatekeeping
- Unclear objectives or cryptic puzzles
- Unavoidable damage or unfair RNG
- Artificially restricting player options
- Excessive story changes that alter Emerald's feel
- Scope creep on systems before core loop is stable

## Balance Philosophy

Relics should create interesting playstyles, not unfun restrictions. Common relics should feel impactful at tier 3. Rare relics should feel occasionally broken — that's the point. Marks should feel like a meaningful contract, not a punishment. The difficulty arc: early game teaches mechanics, mid game tests knowledge, late game expects mastery, postgame is for masochists.

---

# Appendices

## Appendix A: Recommended Testing Tools

- mGBA with debug features
- PoryMap for map editing
- Encounter table spreadsheet for balance tracking
- Pokemon Showdown damage calculator for competitive balance
- Community playtester Discord for feedback

## Appendix B: Resources & References

- pokeemerald-expansion: https://github.com/rh-hideout/pokeemerald-expansion
- Radical Red AI implementation
- Balatro joker design philosophy
- Elden Ring Legacy Dungeon design
- Smogon competitive tier lists

---

# Conclusion

Pokemon Emerald Herald is a romhack that earns its replayability through tight systemic design rather than content volume. The Mark system gives players a honest difficulty contract before a run begins. The Relic system gives them a joker-economy progression loop that rewards both bold gambling and patient optimisation. Authored competitive trainer teams — tiered by marks — provide the handcrafted Ornstein and Smough moments that make overcoming a significant adversary feel earned.

The Pokemon are your deck. The relics are your jokers. The marks are how much you're willing to lose.

*May your curses be light and your boons be strong.*
