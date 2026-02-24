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
class RelicDef:
    symbol: str
    name: str | None
    rarity: str | None
    descriptions: list[str | None]
    effects_names: list[str | None]
    effect_count_raws: list[str | None]


ROOT = Path(__file__).resolve().parents[2]


def parse_relic_constants(path: Path) -> tuple[dict[str, int], int | None]:
    text = read_text(path)
    relic_ids: dict[str, int] = {}
    relic_count: int | None = None
    for line in text.splitlines():
        match = re.match(r"\s*#define\s+(RELIC_[A-Z0-9_]+)\s+(\d+)\b", line)
        if not match:
            continue
        name, value = match.group(1), int(match.group(2))
        if name == "RELIC_COUNT":
            relic_count = value
            break
        else:
            relic_ids[name] = value
    return relic_ids, relic_count


def _eval_multiplier(expr: str | None) -> float | None:
    if expr is None:
        return None
    expr = expr.strip()
    match = re.fullmatch(r"RELIC_REDUCTION_PCT\((\d+(?:\.\d+)?)\)", expr)
    if match:
        return (100.0 - float(match.group(1))) / 100.0
    match = re.fullmatch(r"RELIC_MULT_PCT\((\d+(?:\.\d+)?)\)", expr)
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
        r"static\s+const\s+struct\s+RelicEffect\s+(sRelicEffects_[A-Za-z0-9_]+)\s*\[\]\s*=\s*{",
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


def parse_relic_defs(path: Path) -> dict[str, RelicDef]:
    text = read_text(path)
    defs: dict[str, RelicDef] = {}
    pattern = re.compile(r"\[(RELIC_[A-Z0-9_]+)\]\s*=\s*{", re.M)
    for match in pattern.finditer(text):
        symbol = match.group(1)
        if symbol == "RELIC_COUNT":
            continue
        _, _, block = extract_braced_block(text, match.end() - 1)
        name_raw = get_simple_field(block, "name")
        rarity_raw = get_simple_field(block, "rarity")
        defs[symbol] = RelicDef(
            symbol=symbol,
            name=extract_string_literal(name_raw) or (name_raw.strip() if name_raw else None),
            rarity=rarity_raw,
            descriptions=[None, None, None],
            effects_names=[None, None, None],
            effect_count_raws=[None, None, None],
        )
    return defs


def _normalize_selector(selector: dict) -> dict:
    return {
        "side": selector.get("side"),
        "moveType": selector.get("moveType"),
        "moveCategory": selector.get("moveCategory"),
        "minHpPct": v if (v := maybe_int(selector.get("minHpPct"))) is not None else selector.get("minHpPct"),
    }


def build_relics() -> tuple[list[dict], list[str]]:
    constants_path = ROOT / "include/constants/relics.h"
    data_path = ROOT / "src/data/relics.h"

    relic_ids, relic_count = parse_relic_constants(constants_path)
    effects = parse_effects_arrays(data_path)
    defs = parse_relic_defs(data_path)

    errors: list[str] = []
    if relic_count is None:
        errors.append("RELIC_COUNT not found in constants.")
    else:
        if relic_ids:
            max_id = max(relic_ids.values())
            if max_id + 1 != relic_count:
                errors.append(f"RELIC_COUNT={relic_count} but max id is {max_id}.")

    relics: list[dict] = []
    for symbol, relic_id in sorted(relic_ids.items(), key=lambda x: x[1]):
        entry = defs.get(symbol)
        name = entry.name if entry else None
        rarity = entry.rarity if entry else None

        # Collect effects from all tier variants
        tier_effects: list[list[dict]] = []
        for tier_suffix in ["_T1", "_T2", "_T3"]:
            effects_name = f"sRelicEffects_{name}{tier_suffix}" if name else None
            effect_list = effects.get(effects_name, []) if effects_name else []
            tier_effects.append([
                {
                    "type": eff.type,
                    "stacking": eff.stacking,
                    "selector": _normalize_selector(eff.selector),
                    "multiplier": eff.multiplier,
                    "multiplier_raw": eff.multiplier_raw,
                }
                for eff in effect_list
            ])

        implemented = any(len(te) > 0 for te in tier_effects)

        relics.append(
            {
                "id": relic_id,
                "symbol": symbol,
                "name": name,
                "rarity": rarity,
                "implemented": implemented,
                "tiers": [
                    {"tier": t + 1, "effects": tier_effects[t]}
                    for t in range(3)
                ],
            }
        )

        if symbol != "RELIC_NONE" and not implemented:
            errors.append(f"{symbol} has no implemented effects.")

    extra_defs = [k for k in defs.keys() if k not in relic_ids]
    for symbol in extra_defs:
        errors.append(f"Definition {symbol} not declared in constants.")

    return relics, errors


def _pretty_selector(selector: dict) -> str:
    side_map = {
        "RELIC_SIDE_PLAYER": "PLAYER",
        "RELIC_SIDE_OPPONENT": "OPPONENT",
        "RELIC_SIDE_BOTH": "BOTH",
    }
    category_map = {
        "DAMAGE_CATEGORY_PHYSICAL": "PHYSICAL",
        "DAMAGE_CATEGORY_SPECIAL": "SPECIAL",
        "DAMAGE_CATEGORY_STATUS": "STATUS",
        "RELIC_MOVE_CATEGORY_ANY": "ANY",
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

    if effect_type == "RELIC_EFF_DAMAGE_TAKEN_MULT":
        mult_text = f"x{mult:.3f}" if isinstance(mult, float) else (mult_raw or "(unknown)")
        return f"Damage Taken {mult_text} ({stacking}) when {_pretty_selector(selector)}"

    mult_text = mult_raw or (f"x{mult:.3f}" if isinstance(mult, float) else "(unknown)")
    return f"{effect_type} {mult_text} ({stacking}) when {_pretty_selector(selector)}"


def write_json(relics: list[dict], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "source_files": [
            "include/constants/relics.h",
            "src/data/relics.h",
        ],
        "relics": relics,
    }
    out_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_md(relics: list[dict], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    lines: list[str] = []
    lines.append("# Relics")
    lines.append("")
    lines.append("## Index")
    for relic in relics:
        name = relic.get("name") or relic.get("symbol")
        anchor = name.lower().replace(" ", "-")
        lines.append(f"- [{name}](#{anchor})")
    lines.append("")

    for relic in relics:
        name = relic.get("name") or relic.get("symbol")
        symbol = relic.get("symbol")
        rid = relic.get("id")
        rarity = relic.get("rarity") or "(none)"
        lines.append(f"## {name}")
        lines.append("")
        lines.append(f"- ID: `{rid}`")
        lines.append(f"- Symbol: `{symbol}`")
        lines.append(f"- Rarity: {rarity}")
        lines.append(f"- Implemented: {str(relic.get('implemented', False))}")
        lines.append("")
        tiers = relic.get("tiers", [])
        for tier_data in tiers:
            tier_num = tier_data.get("tier", "?")
            effects = tier_data.get("effects", [])
            lines.append(f"### Tier {tier_num}")
            if effects:
                for effect in effects:
                    lines.append(f"- {_effect_sentence(effect)}")
            else:
                lines.append("- (none)")
            lines.append("")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Export relic data to JSON/Markdown")
    parser.add_argument("--json", dest="json_path", default="exports/relics.json")
    parser.add_argument("--md", dest="md_path", default="exports/relics.md")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    relics, errors = build_relics()

    if args.check:
        if errors:
            for err in errors:
                print(f"error: {err}")
            return 1
        print("ok: relics registry checks passed")
        return 0

    if errors:
        for err in errors:
            print(f"warning: {err}")

    json_path = Path(args.json_path)
    md_path = Path(args.md_path)
    write_json(relics, json_path)
    write_md(relics, md_path)
    print(f"Exported {len(relics)} relics -> {json_path}, {md_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
