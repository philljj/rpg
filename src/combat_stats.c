#include <math.h>
#include <stdint.h>
#include <unistd.h>

#include "safer_rand.h"
#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "combat_stats.h"


size_t
get_spell_cost(const element_t element,
               const float     mp_mult)

{
    // Get spell cost.
    //   - Instant damage cost more than DOT.
    //   - non elemental costs more than elemental.
    size_t cost = 0;

    switch (element) {
        case NON_ELEM:
            cost = (size_t) floor(32 * mp_mult);
            break;

        default:
            cost = (size_t) floor(16 * mp_mult);
            break;
    }

    return cost;
}


float
get_melee_dmg(const hero_t * h,
              const item_t * weapon,
              smear_t        smear_type)
{
    // Attack damage is
    //
    //   dmg = (S * str + A * agi) * (weapon multiplier)
    //
    // where the weapon multiplier is given by
    //
    //   2 handed = TWO_HAND_MULT
    //   1 handed = ONE_HAND_MULT (per hand)
    //   unarmed  = UNARMED_MULT
    //
    // and S, A are weapon dependent coefficients.

    float  mult = 0;
    float  S = 0.5;  // Component of damage from strength.
    float  A = 0.5;  // Component of damage from agility.

    // 2 hand blunt = 0.9 * strength + 0.1 * agility
    // 1 hand piercing = 0.1 * strength + 0.9 * agility

    if (weapon->slot != NO_ITEM && weapon->armor_type != WEAPON) {
        // Something is equipped that isn't a weapon.
        return 0;
    }

    switch (weapon->slot) {
    case MAIN_HAND:
        S -= 0.2;
        A += 0.2;
        mult = ONE_HAND_MULT;
        break;

    case OFF_HAND:
        S -= 0.2;
        A += 0.2;
        mult = ONE_HAND_MULT;
        break;

    case TWO_HAND:
        if (weapon->armor_type != WEAPON) {
            return 0;
        }

        S += 0.2;
        A -= 0.2;
        mult = TWO_HAND_MULT;
        break;

    default:
        mult = UNARMED_MULT;
    }

    switch (weapon->weapon_type) {
    case PIERCING:
        S -= 0.2;
        A += 0.2;
        break;

    case EDGED:
        // Edged benefits from strength and agility equally.
        break;

    case BLUNT:
        S += 0.2;
        A -= 0.2;
        break;

    default:
        // Unarmed. Do nothing.
        break;
    }

    float dmg = 0;
    float agi = (float) get_total_stat(h, AGILITY);
    float str = (float) get_total_stat(h, STRENGTH);

    dmg = mult * ((S * str) + (A * agi));

    // Add a smear to dmg, to give it some randomness.
    float smear = 1;

    switch (smear_type) {
    case NO_SMEAR:
        smear = 1;
        break;
    case STD_SMEAR:
        smear = 0.01 * (80 + (rpg_rand(0, 40)));
        break;
    }

    return dmg * smear;
}


float
get_spell_dmg(const hero_t *  h,
              const element_t element,
              smear_t         smear_type)
{
    // Spell damage (and spell healing) is
    //
    //   dmg = (total wisdom) * (spell multiplier)
    //
    // where the spell multiplier is SPELL_DMG_MULT.
    //
    float dmg = 0;
    float wis = 0;
    float mult = SPELL_DMG_MULT;
    float spell_power = 0;

    // Get unit's innate elemental power and wisdom.
    spell_power += get_elem_pow(&h->power, element);
    wis += h->base.wis;

    // Then get elemental power and wisdom from gear.
    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to calculate.
            continue;
        }

        wis += h->items[i].attr.wis;

        spell_power += get_elem_pow(&h->items[i].power, element);
    }

    dmg += wis * mult;
    dmg += spell_power;

    // Add a smear to dmg, to give it some randomness.
    float smear;

    switch (smear_type) {
    case NO_SMEAR:
        smear = 1;
        break;
    case STD_SMEAR:
        smear = 0.01 * (80 + (rpg_rand(0, 40)));
        break;
    }

    return dmg * smear;
}


float
get_spell_res(const hero_t *  h,
              const element_t element)
{
    // Get unit's innate elemental resist.
    float res = get_elem_pow(&h->resist, element);
    float mitigation;

    // Then get elemental resist and wisdom from gear.
    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to calculate.
            continue;
        }

        res += get_elem_pow(&h->items[i].resist, element);
    }

    mitigation = 1 - (res / (res + ARMOR_HALF_POINT));

    return mitigation;
}


float
get_elem_pow(const spell_t * power,
             const element_t element)
{
    float  spell_power = 0;

    switch (element) {
    case FIRE:
        spell_power += power->fire;
        spell_power += 0.5 * power->non_elemental;
        break;
    case FROST:
        spell_power += power->frost;
        spell_power += 0.5 * power->non_elemental;
        break;
    case SHADOW:
        spell_power += power->shadow;
        spell_power += 0.5 * power->non_elemental;
        break;
    case NON_ELEM:
        spell_power += power->non_elemental;
        spell_power += 0.5 * power->fire;
        spell_power += 0.5 * power->frost;
        spell_power += 0.5 * power->shadow;
        break;
    case HOLY:
        /* There is no holy spell resist. Holy pierces all defenses.
         * Bonus holy power items will be rare. */
        spell_power += power->holy;
        break;
    case NATURE:
        spell_power += power->nature;
        break;
    case MELEE:
        /* not applicable */
        break;
    case RESTORATION:
        spell_power += power->restoration;
        break;
    }

    return spell_power;
}


float
get_elem_res(const spell_t * resist,
             const element_t element)
{
    float  spell_resist = 0;

    switch (element) {
    case FIRE:
        spell_resist += resist->fire;
        spell_resist += 0.5 * resist->non_elemental;
        break;
    case FROST:
        spell_resist += resist->frost;
        spell_resist += 0.5 * resist->non_elemental;
        break;
    case SHADOW:
        spell_resist += resist->shadow;
        spell_resist += 0.5 * resist->non_elemental;
        break;
    case NON_ELEM:
        spell_resist += resist->non_elemental;
        spell_resist += 0.5 * resist->fire;
        spell_resist += 0.5 * resist->frost;
        spell_resist += 0.5 * resist->shadow;
        break;
    case HOLY:
        // There is no holy spell resist. Holy pierces all defenses.
        spell_resist = 0;
        break;
    case NATURE:
        spell_resist += resist->nature;
        break;
    case MELEE:
        /* not applicable */
        break;
    case RESTORATION:
        break;
    }

    return spell_resist;
}


/* Given a hero, calculate it's melee damage mitigation from armor. */
float
get_mitigation(const hero_t * h)
{
    float armor = (float) get_armor(h);
    float mitigation = 1 - (armor / (armor + ARMOR_HALF_POINT));

    return mitigation;
}

/* Used when a special melee attack bypasses a % of armor. */
float
get_mitigation_w_bypass(const hero_t * h,
                        const float    bypass)
{
    float armor = (float) get_armor(h);
    armor *= bypass;

    float mitigation = 1 - (armor / (armor + ARMOR_HALF_POINT));

    return mitigation;
}

size_t
get_armor(const hero_t * h)
{
    // TODO: check for temporary armor buffs?
    size_t armor = h->armor;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to do.
            continue;
        }

        armor += h->items[i].armor;
    }

    return armor;
}


float
get_dodge(const hero_t * h)
{
    // Dodge is four digit float, e.g. 48.38%
    // 0 - 9999.
    float agi;
    float dodge;

    agi = (float) get_total_stat(h, AGILITY);
    dodge = floor(10000 * agi / (agi + DODGE_HALF_POINT));

    return dodge;
}


float
get_spell_crit(const hero_t * h)
{
    float crit = 0;
    float  wis = 0;

    wis = (float) get_total_stat(h, WISDOM);
    crit = floor(100 * wis / (wis + DODGE_HALF_POINT));

    return crit;
}


size_t
get_total_stat(const hero_t * h,
               stat_type_t    stat_type)
{
    size_t total = 0;

    switch (stat_type) {
    case STAMINA:
        total += h->base.sta;
        break;
    case STRENGTH:
        total += h->base.str;
        break;
    case AGILITY:
        total += h->base.agi;
        break;
    case WISDOM:
        total += h->base.wis;
        break;
    case SPIRIT:
        total += h->base.spr;
        break;
    }

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            continue;
        }

        switch (stat_type) {
        case STAMINA:
            total += h->items[i].attr.sta;
            break;
        case STRENGTH:
            total += h->items[i].attr.str;
            break;
        case AGILITY:
            total += h->items[i].attr.agi;
            break;
        case WISDOM:
            total += h->items[i].attr.wis;
            break;
        case SPIRIT:
            total += h->items[i].attr.spr;
            break;
        }
    }

    return total;
}
