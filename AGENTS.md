# AGENTS.md

Guidance for AI coding agents working in this repository.

## What this is

Emerald Herald is a curated, roughly one-hour Pokémon roguelike built on pokeemerald-expansion (a GBA decompilation: C plus assembly). [design.md](design.md) is the source of truth for what the game is.

- Its **Locked** decisions are settled. Don't relitigate them without good reason.
- Its **Open** questions are for the design-tree session (see §18).
- The engine is kept almost entirely; only the progression glue is replaced. Don't delete vanilla story scripts or maps as cleanup, they become dead content.

## Git and remotes

- This repo is a fork of `rh-hideout/pokeemerald-expansion`. The only repo that matters is the user's own: `n-deshpande/emerald-herald` (`origin`, public).
- **Never open a PR against, or push to, `rh-hideout`.** The `upstream` remote is fetch-only (its push URL is `DISABLED`). `gh` defaults to `n-deshpande/emerald-herald`; still pass `--repo n-deshpande/emerald-herald` to `gh pr create`.
- **Frozen at expansion 1.17.1** (tag `expansion/1.17.1`). Don't merge, rebase onto, or cherry-pick from upstream unless asked.
- Commit identity is `n-deshpande <nxdeshpande@gmail.com>`. Never commit under another address, and check `git config user.email` in a fresh clone before the first commit.
- Commits in the `archive/*` tags (pre-pivot work) carry an old work email as their author. Don't merge them into `main`; port code by hand or with `git cherry-pick --reset-author`.
- Don't commit or push unless asked.
- No CI/CD yet. Don't add or change anything under `.github/workflows`.

## Build and test

- `make -j$(nproc)` builds `pokeemerald.gba` with the modern `arm-none-eabi-gcc` toolchain (verified on a clean 1.17.1 tree). `build/` and `*.gba` are gitignored.
- `make check` runs the test suite; filter with `make check TESTS="<name>"`. See `docs/tutorials/how_to_testing_system.md`.
- `make debug` and `make release` build those variants.

## Code style

Follow `docs/STYLEGUIDE.md`. The main points:

- `PascalCase` for functions and structs, `camelCase` for variables and fields, a `g` prefix for globals and `s` for statics, `CAPS_WITH_UNDERSCORES` for macros and constants.
- Don't compare to `0` explicitly unless the situation calls for it (`if (!x)`, not `if (x == 0)`).
- Check config macros inside normal control flow, not with `#ifdef` in function bodies.
- Mark functions that are never called `UNUSED`.
- External data files use JSON.

The styleguide's **Principles** section (minimally invasive, config and save philosophy) is policy for upstream contributions and doesn't bind this hack. For example, the run state struct will change the save layout (design.md §18).

## Layout

- `src/`, `include/` (feature toggles in `include/config/`, constants in `include/constants/`)
- `data/` (maps, scripts, encounters), `graphics/`, `sound/`
- `test/`, `docs/`
