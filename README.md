# Pokemon Emerald Herald

[WIP]

A vanilla-plus Pokemon Emerald romhack with Soulsborne-inspired challenge, high replayability, and competitive-grade battles. The core Hoenn journey remains intact, but each run is shaped by a unique procedural Curse system.

This project is built on top of pret’s `pokeemerald` decompilation via `pokeemerald-expansion`.

## Design Goals

- Challenging but fair battles with strong player tools
- High replayability via seeded Curses and encounters
- Competitive-grade NPC teams (items, EVs, movesets)
- Quality-of-life first to minimize grinding
- Respect Emerald’s story beats while elevating gameplay

## Signature Features (Design Targets)

- **Bearer of the Curse**: 3 Banes + 3 Boons chosen from seed-based rolls
- **Legacy Dungeons**: three optional, high-difficulty dungeons that remove a Bane
- **Seeded Encounters**: 20% of wild slots are unique to each run
- **Competitive Battles**: no mid-battle healing item spam; smarter AI
- **Early Access to Power**: strong Pokemon and tools available early

Full design details live in `docs/GAMEDESIGN.md`.

## Current Status

This repo is a fork and a work in progress. The design bible is the source of truth; implementation is iterative. See `CHANGELOG.md` for active changes.

## Getting Started

- Build: see `INSTALL.md`
- Test: see `CLAUDE.md` for the test command matrix

## Credits

- Based on `pokeemerald-expansion` (RHH) and pret’s `pokeemerald`
- Please credit the upstream projects and contributors when redistributing

Example credit line:

```
Based on RHH's pokeemerald-expansion (https://github.com/rh-hideout/pokeemerald-expansion/)
```

## Disclaimer

This is a fan-made romhack project and is not affiliated with Nintendo, Game Freak, or The Pokemon Company.
