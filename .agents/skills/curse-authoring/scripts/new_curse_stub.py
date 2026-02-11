#!/usr/bin/env python3
"""
Generate curse authoring stubs: effect array, CurseDef entry, and test skeleton.

Usage (examples):
  uv run python scripts/new_curse_stub.py --id CURSE_BOON_PYROWARD --name Pyroward --desc "Your party takes 50% less Fire damage." --move MOVE_EMBER --move-type TYPE_FIRE --reduction-pct 50
  uv run python scripts/new_curse_stub.py --id CURSE_BANE_ARCANE_WEAKNESS --name "Arcane Weakness" --mult-pct 150
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

PREFIXES = ("CURSE_BOON_", "CURSE_BANE_")


def slugify(value: str) -> str:
    value = value.strip().lower()
    value = re.sub(r"[^a-z0-9]+", "_", value)
    value = value.strip("_")
    value = re.sub(r"_{2,}", "_", value)
    return value or "curse"


def pascalize(value: str) -> str:
    parts = re.split(r"[^a-zA-Z0-9]+", value.strip())
    return "".join(p.capitalize() for p in parts if p)


def id_base(curse_id: str) -> str:
    base = curse_id.strip()
    for prefix in PREFIXES:
        if base.startswith(prefix):
            base = base[len(prefix):]
            break
    return base


def is_boon(curse_id: str) -> bool:
    return curse_id.startswith("CURSE_BOON_")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate curse authoring stubs.")
    parser.add_argument("--id", required=True, help="Curse ID (e.g., CURSE_BOON_PYROWARD)")
    parser.add_argument("--name", required=True, help="Display name (e.g., Pyroward)")
    parser.add_argument("--desc", default=None, help="Description text for COMPOUND_STRING")
    parser.add_argument("--move", default="MOVE_EMBER", help="Positive-case test move")
    parser.add_argument("--move-negative", default="MOVE_SCRATCH", help="Negative-case test move")
    parser.add_argument("--move-type", default="TYPE_NONE", help="Selector move type (TYPE_*)")
    parser.add_argument("--category", default="CURSE_MOVE_CATEGORY_ANY", help="Selector move category")
    parser.add_argument("--side", default="CURSE_SIDE_PLAYER", help="Selector side")
    parser.add_argument("--min-hp-pct", type=int, default=0, help="Selector min HP percent")

    mult_group = parser.add_mutually_exclusive_group()
    mult_group.add_argument("--reduction-pct", type=int, help="Damage reduction percent")
    mult_group.add_argument("--mult-pct", type=int, help="Damage multiplier percent")

    parser.add_argument("--write-test", default=None, help="Write test file to path")
    return parser.parse_args()


def multiplier_expr(args: argparse.Namespace) -> str:
    if args.reduction_pct is not None:
        return f"CURSE_REDUCTION_PCT({args.reduction_pct})"
    if args.mult_pct is not None:
        return f"CURSE_MULT_PCT({args.mult_pct})"
    return "CURSE_MULT_PCT(100)"  # TODO: set multiplier


def build_effects(name_ident: str, args: argparse.Namespace) -> str:
    return f"""static const struct CurseEffect sCurseEffects_{name_ident}[] =
{{
    {{
        .type = CURSE_EFF_DAMAGE_TAKEN_MULT,
        .stacking = CURSE_STACK_MULTIPLY,
        .selector =
        {{
            .side = {args.side},
            .moveType = {args.move_type},
            .moveCategory = {args.category},
            .minHpPct = {args.min_hp_pct},
        }},
        .multiplier = {multiplier_expr(args)},
    }},
}};
"""


def build_def(curse_id: str, name: str, desc: str, name_ident: str) -> str:
    return f"""[{curse_id}] =
{{
    .name = COMPOUND_STRING(\"{name}\"),
    .description = COMPOUND_STRING(\"{desc}\"),
    .effects = sCurseEffects_{name_ident},
    .effectCount = ARRAY_COUNT(sCurseEffects_{name_ident}),
}},
"""


def build_tests(curse_id: str, name: str, args: argparse.Namespace) -> str:
    set_block = """            Curse_SetActiveBoon(0, {curse_id});"""
    if not is_boon(curse_id):
        set_block = """            gSaveBlock2Ptr->curses.activeBanes[0] = {curse_id};"""

    assume_positive = ""
    if args.move_type != "TYPE_NONE":
        assume_positive = f"        ASSUME(GetMoveType({args.move}) == {args.move_type});\n"

    assume_negative = ""
    if args.move_type != "TYPE_NONE":
        assume_negative = f"        ASSUME(GetMoveType({args.move_negative}) != {args.move_type});\n"

    return f"""#include \"global.h\"
#include \"test/battle.h\"
#include \"curse.h\"

SINGLE_BATTLE_TEST(\"{name} applies when selector matches\", s16 damage)
{{
    bool32 cursed;
    PARAMETRIZE {{ cursed = FALSE; }}
    PARAMETRIZE {{ cursed = TRUE; }}
    GIVEN {{
        if (cursed)
{set_block.format(curse_id=curse_id)}
        else
            Curse_ClearActive();
{assume_positive}        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    }} WHEN {{
        TURN {{ MOVE(opponent, {args.move}); }}
    }} SCENE {{
        HP_BAR(player, captureDamage: &results[i].damage);
    }} FINALLY {{
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage); // TODO: set expected multiplier
    }}
}}

SINGLE_BATTLE_TEST(\"{name} does not apply when selector does not match\", s16 damage)
{{
    bool32 cursed;
    PARAMETRIZE {{ cursed = FALSE; }}
    PARAMETRIZE {{ cursed = TRUE; }}
    GIVEN {{
        if (cursed)
{set_block.format(curse_id=curse_id)}
        else
            Curse_ClearActive();
{assume_negative}        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    }} WHEN {{
        TURN {{ MOVE(opponent, {args.move_negative}); }}
    }} SCENE {{
        HP_BAR(player, captureDamage: &results[i].damage);
    }} FINALLY {{
        EXPECT_EQ(results[0].damage, results[1].damage);
    }}
}}
"""


def main() -> int:
    args = parse_args()
    curse_id = args.id.strip()
    display_name = args.name.strip()
    desc = args.desc or f"TODO: Describe {display_name}."

    name_ident = pascalize(display_name)
    if not name_ident:
        name_ident = pascalize(id_base(curse_id))

    effects = build_effects(name_ident, args)
    curse_def = build_def(curse_id, display_name, desc, name_ident)
    tests = build_tests(curse_id, display_name, args)

    output = []
    output.append("// === src/data/curses.h: effect array ===\n")
    output.append(effects)
    output.append("\n// === src/data/curses.h: sCurseDefs entry ===\n")
    output.append(curse_def)
    output.append("\n// === test/battle/curse/<name>.c ===\n")
    output.append(tests)

    sys.stdout.write("\n".join(output))

    if args.write_test:
        path = Path(args.write_test)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(tests)
        sys.stderr.write(f"[OK] Wrote test file: {path}\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
