# Exposed T3 Test Failure

## Status
Resolved on February 24, 2026.

## Root Cause
The relic runtime was correct. The failing assertion came from the test fixture saturating damage at target max HP.

- The old Exposed tests used `Eevee` as the defender.
- In the T2/T3 scenarios, boosted damage reached or exceeded Eevee's max HP.
- `captureDamage` records only damage actually applied, so values were capped by remaining HP.
- Once capped, increasing the multiplier in relic data (even to 2.0x) could not change observed damage.

This produced the misleading symptom where T3 looked like it was behaving as ~1.3x.

## What Was Changed
- Updated [`test/battle/relic/exposed.c`](/home/nnd/emerald-herald/test/battle/relic/exposed.c) to avoid KO saturation in multiplier assertions.
- Switched attacker in super-effective tests from `Machop` (high damage) to `Wobbuffet` using `Karate Chop` (lower, non-STAB damage).
- Kept a Normal-type defender so matchup behavior remains correct.
- Updated the "does not increase damage dealt to opponents" case to the same low-damage fixture pattern.

## Validation
- `make check TESTS="test/battle/relic/exposed.c"` now passes all 5 Exposed battle tests.
- A direct runtime check during diagnosis confirmed `Relic_GetDamageTakenModifier` returns `UQ_4_12(1.4)` for Exposed T3 in a super-effective context.

## Takeaway
For multiplier tests that use `captureDamage`, ensure the baseline and modified damage both stay below defender HP. Otherwise, HP cap can mask scaling changes and create false negatives.
