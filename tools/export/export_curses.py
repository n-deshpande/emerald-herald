from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
import re

from c_parse import (
    read_text,
    extract_braced_block,
    split_top_level_braces,
    get_simple_field,
    extract_string_literal,
    maybe_int,
)


@dataclass
class Effect:
    type: str | None
    stacking: str | None
    selector: dict
    multiplier_raw: str | None
    multiplier: float | None


@dataclass
class CurseDef:
    symbol: str
    name: str | None
    description: str | None
    effects_name: str | None
    effect_count_raw: str | None


ROOT = Path(__file__).resolve().parents[2]


def parse_curse_constants(path: Path) -> tuple[dict[str, int], int | None]:
    text = read_text(path)
    curse_ids: dict[str, int] = {}
    curse_count: int | None = None
    for line in text.splitlines():
        match = re.match(r"\s*#define\s+(CURSE_[A-Z0-9_]+)\s+(\d+)\b", line)
        if not match:
            continue
        name, value = match.group(1), int(match.group(2))
        if name == "CURSE_COUNT":
            curse_count = value
            break
        else:
            curse_ids[name] = value
    return curse_ids, curse_count


def _eval_multiplier(expr: str | None) -> float | None:
    if expr is None:
        return None
    expr = expr.strip()
    match = re.fullmatch(r"CURSE_REDUCTION_PCT\((\d+(?:\.\d+)?)\)", expr)
    if match:
        return (100.0 - float(match.group(1))) / 100.0
    match = re.fullmatch(r"CURSE_MULT_PCT\((\d+(?:\.\d+)?)\)", expr)
    if match:
        return float(match.group(1)) / 100.0
    match = re.fullmatch(r"UQ_4_12\(([-\d\.]+)\)", expr)
    if match:
        return float(match.group(1))
    return None


def parse_effects_arrays(path: Path) -> dict[str, list[Effect]]:
    text = read_text(path)
    effects: dict[str, list[Effect]] = {}
    pattern = re.compile(
        r"static\s+const\s+struct\s+CurseEffect\s+(sCurseEffects_[A-Za-z0-9_]+)\s*\[\]\s*=\s*{",
        re.M,
    )
    for match in pattern.finditer(text):
        name = match.group(1)
        _, _, block = extract_braced_block(text, match.end() - 1)
        array_body = block[1:-1]
        entries = split_top_level_braces(array_body)
        parsed_entries: list[Effect] = []
        for entry in entries:
            eff_type = get_simple_field(entry, "type")
            eff_stacking = get_simple_field(entry, "stacking")
            mult_raw = get_simple_field(entry, "multiplier")

            selector_block = None
            sel_match = re.search(r"\.selector\s*=\s*{", entry)
            if sel_match:
                _, _, selector_block = extract_braced_block(entry, sel_match.end() - 1)
            selector_body = selector_block[1:-1] if selector_block else ""
            selector = {
                "side": get_simple_field(selector_body, "side"),
                "moveType": get_simple_field(selector_body, "moveType"),
                "moveCategory": get_simple_field(selector_body, "moveCategory"),
                "minHpPct": get_simple_field(selector_body, "minHpPct"),
            }

            parsed_entries.append(
                Effect(
                    type=eff_type,
                    stacking=eff_stacking,
                    selector=selector,
                    multiplier_raw=mult_raw,
                    multiplier=_eval_multiplier(mult_raw),
                )
            )
        effects[name] = parsed_entries
    return effects


def parse_curse_defs(path: Path) -> dict[str, CurseDef]:
    text = read_text(path)
    defs: dict[str, CurseDef] = {}
    pattern = re.compile(r"\[(CURSE_[A-Z0-9_]+)\]\s*=\s*{", re.M)
    for match in pattern.finditer(text):
        symbol = match.group(1)
        if symbol == "CURSE_COUNT":
            continue
        _, _, block = extract_braced_block(text, match.end() - 1)
        name_raw = get_simple_field(block, "name")
        desc_raw = get_simple_field(block, "description")
        effects_name = get_simple_field(block, "effects")
        effect_count_raw = get_simple_field(block, "effectCount")
        defs[symbol] = CurseDef(
            symbol=symbol,
            name=extract_string_literal(name_raw) or (name_raw.strip() if name_raw else None),
            description=extract_string_literal(desc_raw) or (desc_raw.strip() if desc_raw else None),
            effects_name=effects_name,
            effect_count_raw=effect_count_raw,
        )
    return defs


def _normalize_selector(selector: dict) -> dict:
    return {
        "side": selector.get("side"),
        "moveType": selector.get("moveType"),
        "moveCategory": selector.get("moveCategory"),
        "minHpPct": maybe_int(selector.get("minHpPct")) or selector.get("minHpPct"),
    }


def build_curses() -> tuple[list[dict], list[str]]:
    constants_path = ROOT / "include/constants/curses.h"
    data_path = ROOT / "src/data/curses.h"

    curse_ids, curse_count = parse_curse_constants(constants_path)
    effects = parse_effects_arrays(data_path)
    defs = parse_curse_defs(data_path)

    errors: list[str] = []
    if curse_count is None:
        errors.append("CURSE_COUNT not found in constants.")
    else:
        if curse_ids:
            max_id = max(curse_ids.values())
            if max_id + 1 != curse_count:
                errors.append(f"CURSE_COUNT={curse_count} but max id is {max_id}.")

    curses: list[dict] = []
    for symbol, curse_id in sorted(curse_ids.items(), key=lambda x: x[1]):
        entry = defs.get(symbol)
        name = entry.name if entry else None
        description = entry.description if entry else None
        effects_name = entry.effects_name if entry else None
        effect_list = effects.get(effects_name, []) if effects_name else []
        implemented = bool(effect_list)

        curses.append(
            {
                "id": curse_id,
                "symbol": symbol,
                "name": name,
                "description": description,
                "implemented": implemented,
                "effects": [
                    {
                        "type": eff.type,
                        "stacking": eff.stacking,
                        "selector": _normalize_selector(eff.selector),
                        "multiplier": eff.multiplier,
                        "multiplier_raw": eff.multiplier_raw,
                    }
                    for eff in effect_list
                ],
            }
        )

        if symbol != "CURSE_NONE" and not implemented:
            errors.append(f"{symbol} has no implemented effects.")

    extra_defs = [k for k in defs.keys() if k not in curse_ids]
    for symbol in extra_defs:
        errors.append(f"Definition {symbol} not declared in constants.")

    return curses, errors


def _pretty_selector(selector: dict) -> str:
    side_map = {
        "CURSE_SIDE_PLAYER": "PLAYER",
        "CURSE_SIDE_OPPONENT": "OPPONENT",
        "CURSE_SIDE_BOTH": "BOTH",
    }
    category_map = {
        "DAMAGE_CATEGORY_PHYSICAL": "PHYSICAL",
        "DAMAGE_CATEGORY_SPECIAL": "SPECIAL",
        "DAMAGE_CATEGORY_STATUS": "STATUS",
        "CURSE_MOVE_CATEGORY_ANY": "ANY",
    }

    side = side_map.get(selector.get("side"), selector.get("side") or "ANY")
    move_type = selector.get("moveType") or "ANY"
    if move_type == "TYPE_NONE":
        move_type = "ANY"
    elif move_type.startswith("TYPE_"):
        move_type = move_type[5:]

    move_cat = category_map.get(selector.get("moveCategory"), selector.get("moveCategory") or "ANY")
    min_hp = selector.get("minHpPct", 0)

    return f"side={side}, moveType={move_type}, category={move_cat}, minHpPct={min_hp}"


def _effect_sentence(effect: dict) -> str:
    effect_type = effect.get("type") or "UNKNOWN"
    stacking = effect.get("stacking") or "UNKNOWN"
    mult = effect.get("multiplier")
    mult_raw = effect.get("multiplier_raw")
    selector = effect.get("selector") or {}

    if effect_type == "CURSE_EFF_DAMAGE_TAKEN_MULT":
        mult_text = f"x{mult:.3f}" if isinstance(mult, float) else (mult_raw or "(unknown)")
        return f"Damage Taken {mult_text} ({stacking}) when {_pretty_selector(selector)}"

    mult_text = mult_raw or (f"x{mult:.3f}" if isinstance(mult, float) else "(unknown)")
    return f"{effect_type} {mult_text} ({stacking}) when {_pretty_selector(selector)}"


def write_json(curses: list[dict], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "source_files": [
            "include/constants/curses.h",
            "src/data/curses.h",
        ],
        "curses": curses,
    }
    out_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_md(curses: list[dict], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    lines.append("# Curses")
    lines.append("")
    lines.append("## Index")
    for curse in curses:
        name = curse.get("name") or curse.get("symbol")
        anchor = name.lower().replace(" ", "-")
        lines.append(f"- [{name}](#{anchor})")
    lines.append("")

    for curse in curses:
        name = curse.get("name") or curse.get("symbol")
        symbol = curse.get("symbol")
        cid = curse.get("id")
        lines.append(f"## {name}")
        lines.append("")
        lines.append(f"- ID: `{cid}`")
        lines.append(f"- Symbol: `{symbol}`")
        desc = curse.get("description") or "(no description)"
        lines.append(f"- Description: {desc}")
        lines.append(f"- Implemented: {str(curse.get('implemented', False))}")
        lines.append("")
        effects = curse.get("effects", [])
        if effects:
            lines.append("Effects:")
            for effect in effects:
                lines.append(f"- {_effect_sentence(effect)}")
        else:
            lines.append("Effects: (none)")
        lines.append("")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Export curse data to JSON/Markdown")
    parser.add_argument("--json", dest="json_path", default="exports/curses.json")
    parser.add_argument("--md", dest="md_path", default="exports/curses.md")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    curses, errors = build_curses()

    if args.check:
        if errors:
            for err in errors:
                print(f"error: {err}")
            return 1
        print("ok: curses registry checks passed")
        return 0

    if errors:
        for err in errors:
            print(f"warning: {err}")

    json_path = Path(args.json_path)
    md_path = Path(args.md_path)
    write_json(curses, json_path)
    write_md(curses, md_path)
    print(f"Exported {len(curses)} curses -> {json_path}, {md_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
