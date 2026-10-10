# POKEMON: EMERALD HERALD
## Design Direction v3: The Pivot

*October 2026. Supersedes the strategic direction of Design Bible v2.0. Intended as the input to a design-tree session with the codebase attached.*

---

## How to Use This Document

This document records what the game *is*, not how it is built. Each system section separates **locked** decisions (do not relitigate without good reason) from **open** questions (the branches session 2 should map until no ambiguity remains). Where a design choice has an implementation consequence, it is flagged so the design is not undone during implementation.

---

## 1. The Pivot in One Paragraph

Emerald Herald is a curated, roughly one-hour Pokémon roguelike built on the pokeemerald-expansion engine. The player moves through four antes of hand-authored rooms, chosen at branching doors that show their reward and difficulty, and each ante ends at a gym leader who acts as a Balatro-style boss blind with a rule twist revealed in advance. Runs vary through encounter pools, gym team pools, shop stock, and a small set of build-defining relics. Difficulty is climbed through sequential stakes. The tone is Souls: a moody Hoenn ruined by a primal clash that never ended, cryptic NPCs, lore told through item text, and the Emerald Herald levelling you at bonfires.

*The Pokémon are your deck. The relics are your jokers. The gym leaders are your boss blinds. The stakes are how much you're willing to lose.*

---

## 2. Why the Pivot

The v2 bible was a feature list without a core loop. Its main contradictions:

| Problem in v2 | Why it failed | Resolution in v3 |
|---|---|---|
| A relic economy in a 15–20 hour game | Draft and joker systems only pay off over many runs; players would see the pool about once | Runs are about one hour |
| Relics as small stat buffs (+10% STAB) | Lost inside Pokémon's 85–100% damage roll, so effectively invisible | Relics change rules and shape builds |
| Infinite Rare Candies with no level cap | Overlevelling bypasses all authored difficulty | The bonfire sets levels to the ante cap; no grind exists |
| Marks mixing difficulty with variance (random starter) | Not a coherent difficulty ladder | Stakes, each taxing one system |
| Soulslike determinism vs. roguelike variance | They pull in opposite directions | Bosses are fixed and learnable; the variance is in the player's build |
| A full Hoenn of authored content | Scope stalled the project | About 10–12 authored rooms plus 8 gyms, reusing existing maps |

The engine is kept almost entirely: battle system, AI, overworld, script VM, menus. Only the progression glue is replaced. Emerald's story scripts become dead content that the player is never routed into.

---

## 3. Design Principles

**One lever.** The ante number is the single tier value that drives encounter pools, trainer rosters, gym party pools, shop stock, and levels.

**Telegraph everything.** Boss rules are shown at the start of the ante. Doors show reward and difficulty. DexNav shows room pools. Losses should be explainable.

**Author the bosses, randomise the build.** Leaders are learnable; the variance lives in the player's team and relics.

**Decision density.** Every step of an ante asks for a meaningful choice: which door, what to catch, fight or skip, buy or save.

**Respect time.** One hour per run. No grind loops, no farming for rare spawns, no backtracking.

**Rule of cool.** Late antes should offer and use extremely powerful Pokémon. If not then, when?

**Anti-patterns to avoid:** layering mechanics because they exist in games we love; difficulty made of number padding or HP sponges; modifiers too small to perceive; mechanics that reward tedious optimal play over fun play; mid-run surprises the player couldn't have planned for.

---

## 4. Run Structure

### Locked

A run consists of **4 antes followed by a finale**. Each ante follows this flow:

| Step | What happens |
|---|---|
| **Gate** | The ante's boss and its rule are revealed (via a ruined gym sign or a cryptic NPC). Two doors appear. |
| **Room 1** | A themed room: catch within the encounter budget, fight trainers, collect seed-placed items. Its exit forks. |
| **Room 2** | Another themed room, Route or Elite depending on the door chosen. |
| **Bonfire** | The Herald heals the party and raises it to the ante level. Shop and PC are available here. |
| **Gym** | A trimmed gym map, then the leader fight as a boss blind. |

The target is about 15 minutes per ante. The ante count is a data constant so playtesting can tune it.

### Open

| Question | Current lean |
|---|---|
| Fork depth per ante: is two rooms right, or should it vary per ante? | Two rooms; validate in the slice |
| Are the 4 gyms per run drawn from all 8, and is the order shuffled? | Yes, drawn from 8 and normalised to the ante tier |
| Is the Gate its own room or a section of the first room? | Undecided |

---

## 5. Rooms and Map

### Locked

The map is **static and authored, with no procedural generation.** Rooms are small hand-made maps, often trimmed existing Hoenn routes reusing their tilesets. A fork is a pair of doors whose scripts set the next node and warp the player.

There is a **pool of about 10–12 rooms, not tied to any ante.** Any room can appear at any depth, with its contents scaled by ante tier.

**Anti-sameness levers**, all contents-only:

- Seeded object placement: each room has more trainer and item spots than are used, and the seed sets which are visible at room entry using object-event flags.
- Room affixes: an occasional modifier shown on the door, such as Sandstorm or Darkness. This reuses the starting weather and terrain machinery from boss rules.
- Room-to-slot assignment varies per run.

**Doors preview the reward**, following the Aghanim's Labyrinth model: room theme, reward icon, and difficulty tag.

**Node types:** Route (catches, trainers, money and item rewards), Elite (one hard trainer, no catches, relic reward), Bonfire, and Gym.

### Open

| Question | Current lean |
|---|---|
| Exact room count for v1 | 10–12 |
| Affixes in the vertical slice or deferred? | Probably deferred |
| Altar node (free double-edged relic offer) | Deferred; only if the relic cadence needs it |
| Non-combat rooms (puzzles, events) | Parking lot; well after the slice is proven |

---

## 6. Levelling: The Bonfire

### Locked

**The Herald levels you.** This is a nod to Dark Souls 2, where the Emerald Herald is the levelling NPC. Resting at a bonfire raises the party to the ante's level cap. XP is effectively off; expansion's level-cap config is the backstop. New catches arrive at the current ante level. Rare Candies do not exist.

### Implementation flag

Setting levels directly, in jumps, skips level-up evolutions and level-up moves. The bonfire must run an evolution check and offer the move relearner.

### Open

| Question | Current lean |
|---|---|
| Level values per ante and for the finale | To be set in session 2 |
| Does the party level up at the bonfire or on entering the next ante's Gate? | At the bonfire (the ritual beat) |

---

## 7. Encounters and Catching

### Locked

**Species tiers.** One table assigns every species a tier, roughly by evolution stage and base stat total, with legendaries at the top tier. A room's pool is *theme ∩ tier ≤ ante*. This prevents a level 5 Moltres or a level 80 Magby.

**Rule of cool.** By ante 4, extremely powerful Pokémon, legendaries included, can be encountered.

**Catching stays close to vanilla.** Balls are bought with money, the PC is available at bonfires, and the player curates their own party. The constraints are money and the encounters themselves. Balls compete with items and relics for the same money.

**Encounter budget.** Each room allows a finite number of wild encounters, after which "the grass falls silent." This prevents players farming a room for its rarest spawn.

**DexNav.** It reveals a room's pool and allows targeted searches. Searches spend the encounter budget, so information is free but targeting costs.

**Telegraph against bricked runs.** Boss rules shown at the Gate, plus themed doors, let players route toward counters, for example a Water room before a Flannery gym.

### Open

| Question | Current lean |
|---|---|
| Encounter budget size per room | Tune in the slice |
| DexNav search cost relative to the budget | Tune in the slice |
| How the species tier table is defined (BST bands, evolution stage, hand overrides) | Session 2 |
| Rare spawn rates | Rare but legible; nothing at 1% |

---

## 8. Gyms: Boss Blinds

### Locked

**8 leaders, each with a boss rule** that is telegraphed at the Gate. Example rules:

| Leader | Example rule |
|---|---|
| Flannery | Fire-types cannot battle |
| Brawly | No switching |
| Norman | All Pokémon set to level 50 |
| Wattson | Electric Terrain from turn 1 |

**Party pools tiered by ante.** Ante 1 Wattson and ante 4 Wattson are the same leader at different strengths, with the same type identity and strategy but a different roster (for example, Electrike and Magnemite at ante 1; Raikou and Zekrom at ante 4). Party pools also give run-to-run variation within a tier.

**Authoring rule.** Every team is authored once at full competitive spec. Low stakes strip EVs and items programmatically, so stakes do not multiply content. Curating these pools is the enjoyable core of the content work and will be iterated as difficulty is tuned.

**Gym maps** are trimmed existing gym maps. Gym puzzles are not required; puzzles and non-combat variety go in the parking lot.

### Implementation flag

Gym party selection takes the tier as an input from day one, even while only tier 1 exists. Boss rules vary in cost: weather and terrain are nearly free via expansion's starting-status variable; type bans can reuse the Battle Frontier choose-party flow; level normalisation has Frontier precedent; no-switching needs a battle hook. Choose slice rules from the cheap end.

### Open

| Question | Current lean |
|---|---|
| The full boss rule for each of the 8 leaders | Session 2 |
| Party pool size per tier | Session 2 |
| Gym trainers before the leader (none, one, or a mini-gauntlet)? | Undecided |

---

## 9. Finale

### Locked

**One strong encounter**, not a multi-battle gauntlet.

**Vertical slice finale:** a Steven and Wallace double battle with Dialga (Steel) and Palkia (Water), following Emerald Imperium's approach.

**Long-term:** finales draw on Elite Four members and champions from multiple generations.

### Flag

If the finale is the only double battle, the format change surprises the player, which breaks the telegraph principle. Either reveal the format at the final Gate or introduce doubles earlier (a Tate & Liza gym, Elite rooms).

---

## 10. Relics

### Locked

The relics are the jokers: **about 15, with no tiers.** They are powerful and build-directing, and they change rules rather than adding small stat bonuses.

**5 slots.** The Origin relic occupies slot 1. No duplicates.

**3–4 relics are double-edged** (the RoR2 lunar idea without the lunar currency). They have a distinct visual mark, such as a cracked icon or different text colour, but share the same pool and slots. No new system is involved.

Relic value should be **contextual**: many relics refer to team composition, so the "best" relic depends on what you caught. Examples of direction: *Weather Herald* (your lead sets weather matching its type), *Inheritance* (stat boosts pass to the switch-in), *Monotype Pact*, *Small Hands* (bonuses for carrying 3 or fewer Pokémon). Double-edged examples: *Glass Cannon*, *Hollow Crown* (double prize money; lose money when your Pokémon faint), *Pact of Ash* (no weather damage; no bonfire healing).

### Open: relic cadence is the key design question for session 2

The core metric is **relics seen per run ÷ pool size.** With 15 relics, generous offers expose the whole pool every run, and the meta converges on one optimal build. The lean is toward fewer, more meaningful offers.

| Lever | Notes |
|---|---|
| Number of offers per run | Fewer pushes toward commitment and variety between runs |
| Choices per offer (pick 1 of 2 or 1 of 3) | Fewer choices means less of the pool seen per run |
| Sources: post-gym, Elite rooms, shop, Altar | Which are guaranteed and which are optional |
| Rarity weighting | Does v2's rarity concept survive in a 15-relic pool? |
| Anti-convergence | Slot cap, no duplicates, contextual value, no multiplicative proc chains between relics |

A rough target to test: the player sees about 50–60% of the pool per run, fills all 5 slots around ante 3–4, and makes 1–2 replacement decisions.

---

## 11. Economy and Shops

### Locked

Money comes from trainers. Shops are at bonfires, with seed-varied stock scaled by ante tier. Shop stock is dynamic (implementation flag: `CreatePokemartMenu` uses static lists, so stock must be built at runtime).

### Open

| Question | Current lean |
|---|---|
| Do shops sell relics? | Probably; this creates balls-vs-relics tension |
| Rerolls (Balatro-style)? | Undecided; adds a money sink but also complexity |
| Bag healing items: when are they useful, given the base stake bans them in trainer battles? | Session 2 |

---

## 12. Embers: Retries

### Locked

Embers are a per-run pool of boss retries. Losing a gym with an ember left spends it, heals the party, and returns the player to the bonfire. Losing with no embers ends the run. Embers do not restock during a run.

### Open

| Question | Current lean |
|---|---|
| Does losing to a room trainer also spend an ember? | Yes: respawn at the room entrance, healed, with beaten trainers staying beaten |
| Do purchases made at the bonfire persist after a retry? | Yes |

---

## 13. Stakes: Meta-Progression

### Locked

Stakes are unlocked sequentially by winning. Each stake is cumulative and taxes a different system. There is no cross-run currency. The reward is a symbol for clearing the hardest stake. The names below are placeholders.

| Stake | System taxed | Effect (cumulative) |
|---|---|---|
| 1 | — | Base rules: no bag healing in trainer battles, 3 embers |
| 2 | Bosses | Trainers keep full competitive sets (EVs, natures, held items) |
| 3 | Economy | Shops stock one fewer item; prices rise |
| 4 | Build | Relic slots drop from 5 to 4 |
| 5 | Retries | 1 ember |
| 6 | *The Abyss* | 0 embers; each gym gains a second telegraphed boss rule |

### Open

| Question | Current lean |
|---|---|
| Final stake names | Rename later, away from Balatro's |
| Stake stickers per Origin | Optional; costs one save bit each |

---

## 14. Origins: Starters

### Locked

An Origin is a starting Pokémon plus a starting relic in slot 1, in the spirit of Dark Souls starting classes and Balatro decks. It reuses existing systems and replaces v2's "draw 3, pick 1 starting relic" step.

| Origin | Start | Identity |
|---|---|---|
| Hoenn Native | Treecko, Torchic, or Mudkip + a generalist relic | The baseline |
| Draconic Ancestry | A random baby dragon (Bagon, Gible, Dratini…) + a relic that rewards Dragon-types | Slow start, high ceiling |
| Wanderer | No Pokémon, extra money, pick 1 of 3 random relics. The first room uses Safari Zone-style no-battle catching | The most roguelike |

The vertical slice uses one Origin.

### Open

| Question | Current lean |
|---|---|
| The final Origin list | Session 2 |
| Are Origins unlocked by winning (Balatro decks)? | Optional; deferred |

---

## 15. Tone and Lore

### Locked

**Setting:** a Hoenn where the Groudon/Kyogre clash never ended. Broken weather is the world's hazard, gym leaders are lords holding the ruins together, and small clues refer back to the long-destroyed world of the original Emerald.

**Delivery:** cryptic, sparse NPCs; lore in item and relic descriptions; ruined landmarks. No cutscenes.

**The Herald:** a sprite nod to the Dark Souls 2 Emerald Herald ("Bearer of the curse…"). She keeps the bonfires and levels the party.

Weather themes tie lore, room affixes, and boss rules together.

---

## 16. Vertical Slice Scope

Goal: answer "is this fun?" in about 15–20 minutes of play, repeated about 10 times, before any further content is built.

| In the slice | Deferred |
|---|---|
| Custom start: New Game → run init → Gate | Stakes |
| One Origin (Hoenn Native) | Additional Origins, Wanderer's Safari catching |
| Gate with boss telegraph, two rooms with a fork | Room affixes, Altar node |
| Encounter budget and DexNav | Seeded object placement (nice to have) |
| Bonfire: Herald heal and level, evolution check, relearner, shop, PC | Shop rerolls |
| Wattson, tier 1 party pool, one cheap boss rule | Other gyms, tiers 2–4, gym shuffle |
| 3 relics implemented, with the relic menu | The remaining ~12 relics |
| Embers and the run-end flow | Multi-gen finale rotation |
| Finale: Steven/Wallace double, Dialga/Palkia (level set to fit) | Puzzles and non-combat rooms |

---

## 17. Carried Over From v2

**Kept:** the relic save struct and effect lookup table (rename `curse_*` to `relic_*`); the mark bitmask, repurposed as the stake; the trainer tier selection logic; encounter seeding from the Trainer ID; expansion QoL features (Physical/Special split, modern mechanics, and so on); the existing relic menu WIP.

**Removed:** relic tiers and upgrades; the wager mechanic; the mark ladder (replaced by stakes); full Hoenn progression; Legacy Dungeons; Rare Candies and the XP grind; Superboss House, Battle Frontier, and the Sealed Chamber.

**Parking lot (post-slice):** puzzles and non-combat rooms; a multi-gen finale pool; Origin unlocks; a longer endless or tower mode (explicitly resisted for now).

---

## 18. Session 2 Agenda

Map every open question above into a decision tree, with the codebase attached. Suggested priority order, driven by what the vertical slice needs first:

1. Run state struct and save layout (verify free save space)
2. The run flow state machine: start, Gate, rooms, bonfire, gym, finale, end
3. Bonfire levelling, including evolution and move handling
4. The encounter tier table and room pool format; budget and DexNav integration
5. Gym party pool data format with tier input; the slice's boss rule
6. Relic cadence model and the first 3 relics
7. Ember semantics and fail states
8. Finale battle setup

*May your relics be strong and your embers last.*
