#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safer_rand.h"
#include "item_names.h"
#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "item.h"
#include "tui.h"

#define ITEM_DROP_THRESH     (10)  // 100 minus this number is drop rate.

static tier_t gen_item_tier(void);
static size_t gen_item_armor(const size_t level, armor_t armor_type);
static void   gen_item_name(char * name, const armor_t armor_type,
                            const slot_t slot, weapon_t weapon_type);


item_t
gen_item(const char * name,
         const size_t level,
         tier_t       tier,
         size_t       is_weapon,
         armor_t      armor_type,
         slot_t       slot,
         weapon_t     weapon_type)
{
    item_t item;
    size_t shift_lvl = level + 1;
    size_t attr_lvl = shift_lvl > 0 ? shift_lvl - 0 : 1;
    size_t res_lvl = shift_lvl > 10 ? shift_lvl - 10 : 1;
    size_t pow_lvl = shift_lvl > 15 ? shift_lvl - 15 : 1;
    size_t procs_lvl = 20;

    memset(&item, 0, sizeof(item));

    if (tier == RANDOM_TIER) {
        item.tier = gen_item_tier();
    }
    else {
        item.tier = tier;
    }

    // is_weapon has priority over slot_t and armor_t.
    // armor_t has priority over slot_t.

    if (is_weapon) {
        armor_type = WEAPON;
    }
    else {
        if (armor_type == RANDOM_A) {
            armor_type = safer_rand(0, MAX_ARMOR_TYPES);
        }
    }

    if (slot == RANDOM_S) {
        // Determine slot from armor_type.
        switch (armor_type) {
        case WEAPON:
            // Any of 0-2 slot_t.
            slot = safer_rand(0, TWO_HAND);
            if (weapon_type == RANDOM_W) {
                weapon_type = safer_rand(0, BLUNT);
            }
            break;

        case SHIELD:
            slot = OFF_HAND;
            break;

        case CLOTH:
        case LEATHER:
        case MAIL:
        case PLATE:
            // Any of 3-8 slot_t.
            slot = 3 + (safer_rand(0, 5));
            break;

        case MISC:
        default:
            // Any of 9-10 slot_t.
            slot = 9 + safer_rand(0, 1);
            break;
        }
    }

    item.slot = slot;
    item.armor_type = armor_type;

    if (name && *name) {
        strcpy(item.name, name);
    }
    else {
        gen_item_name(item.name, armor_type, slot, weapon_type);
    }

    float tier_mult = 0;

    switch (item.tier) {
    case GOOD:
        tier_mult = 0.5;
        break;
    case RARE:
        tier_mult = 1.0;
        break;
    case EPIC:
        tier_mult = 2.0;
        break;
    case COMMON:
    default:
        tier_mult = 0.25;
        break;
    }

    item.armor = (size_t) floor(tier_mult * (float) gen_item_armor(level,
                                                                   armor_type));

    float sta_mult = 1;
    float str_mult = 1;
    float agi_mult = 1;
    float wis_mult = 1;
    float spr_mult = 1;

    switch (armor_type) {
        case PLATE:         // Plate is slow.
            sta_mult = 2.0;
            str_mult = 1.5;
            agi_mult = 0.5;
            break;
        case MAIL:          // Mail is a compromise.
            sta_mult = 1.5;
            str_mult = 1.5;
            agi_mult = 1.5;
            break;
        case LEATHER:       // Max dodge and melee damage.
            str_mult = 1.5;
            agi_mult = 2.0;
            wis_mult = 1.5;
            break;
        case CLOTH:
            agi_mult = 1.5; // Max regen, and spell power, resist.
            wis_mult = 2.0;
            spr_mult = 1.5;
            break;
        default:
            break;
    }

    if (level > 0) {
        item.attr.sta = floor(sta_mult * tier_mult * (float) safer_rand(0, (attr_lvl)));
        item.attr.str = floor(str_mult * tier_mult * (float) safer_rand(0, (attr_lvl)));
        item.attr.agi = floor(agi_mult * tier_mult * (float) safer_rand(0, (attr_lvl)));
        item.attr.wis = floor(wis_mult * tier_mult * (float) safer_rand(0, (attr_lvl)));
        item.attr.spr = floor(spr_mult * tier_mult * (float) safer_rand(0, (attr_lvl)));
    }

    if (level > 10) {
        item.resist.fire = floor(wis_mult * tier_mult * (float) safer_rand(0, (res_lvl)));
        item.resist.frost = floor(wis_mult * tier_mult * (float) safer_rand(0, (res_lvl)));
        item.resist.shadow = floor(wis_mult * tier_mult * (float) safer_rand(0, (res_lvl)));
    }

    if (level > 15) {
        item.power.fire = floor(wis_mult * tier_mult * (float) safer_rand(0, (pow_lvl)));
        item.power.frost = floor(wis_mult * tier_mult * (float) safer_rand(0, (pow_lvl)));
        item.power.shadow = floor(wis_mult * tier_mult * (float) safer_rand(0, (pow_lvl)));
    }

    if (level > procs_lvl) {
        // Do nothing for now...
    }

    return item;
}


tier_t
gen_item_tier(void)
{
    size_t trigger = safer_rand(0, 100);

    if (trigger < 50) {
        return COMMON;
    }
    else if (trigger < 85) {
        return GOOD;
    }
    else if (trigger < 95) {
        return RARE;
    }
    else {
        return EPIC;
    }
}


void
gen_item_name(char *         name,
              const armor_t  armor_type,
              const slot_t   slot,
              const weapon_t weapon_type)
{
    size_t j = safer_rand(0, NUM_ITEM_NAMES - 1);

    switch (armor_type) {
    case CLOTH:
        switch(slot) {
        case HEAD:
            strcpy(name, cloth_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, cloth_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, cloth_chest[j]);
            return;
        case LEGS:
            strcpy(name, cloth_pants[j]);
            return;
        case HANDS:
            strcpy(name, cloth_gloves[j]);
            return;
        case FEET:
        default:
            strcpy(name, cloth_boots[j]);
            return;
        }
    case LEATHER:
        switch(slot) {
        case HEAD:
            strcpy(name, leather_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, leather_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, leather_chest[j]);
            return;
        case LEGS:
            strcpy(name, leather_pants[j]);
            return;
        case HANDS:
            strcpy(name, leather_gloves[j]);
            return;
        case FEET:
        default:
            strcpy(name, leather_boots[j]);
            return;
        }
    case MAIL:
        switch(slot) {
        case HEAD:
            strcpy(name, mail_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, mail_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, mail_chest[j]);
            return;
        case LEGS:
            strcpy(name, mail_pants[j]);
            return;
        case HANDS:
            strcpy(name, mail_gloves[j]);
            return;
        case FEET:
        default:
            strcpy(name, mail_boots[j]);
            return;
        }
    case PLATE:
        switch(slot) {
        case HEAD:
            strcpy(name, plate_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, plate_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, plate_chest[j]);
            return;
        case LEGS:
            strcpy(name, plate_pants[j]);
            return;
        case HANDS:
            strcpy(name, plate_gloves[j]);
            return;
        case FEET:
        default:
            strcpy(name, plate_boots[j]);
            return;
        }
    case SHIELD:
        strcpy(name, shields[j]);
        return;

    case WEAPON:
        switch (slot) {
        case MAIN_HAND:
        case OFF_HAND:
            switch (weapon_type) {
            case EDGED:
                strcpy(name, one_hand_swords[j]);
                return;
            case PIERCING:
                strcpy(name, one_hand_piercing[j]);
                return;
            case BLUNT:
            default:
                strcpy(name, one_hand_blunt[j]);
                return;
            }
        case TWO_HAND:
        default:
            switch (weapon_type) {
            case EDGED:
                strcpy(name, two_hand_swords[j]);
                return;
            case PIERCING:
                strcpy(name, two_hand_piercing[j]);
                return;
            case BLUNT:
            default:
                strcpy(name, two_hand_blunt[j]);
                return;
            }
        }

    case MISC:
    default:
        switch (slot) {
        case RING:
            strcpy(name, rings[j]);
            return;

        case TRINKET:
        default:
            strcpy(name, trinkets[j]);
            return;
        }
    }

    return;
}


size_t
gen_item_armor(const size_t level,
               armor_t      armor_type)
{
    float base_armor = level + safer_rand(0, level);
    float multiplier = 1.0;

    switch (armor_type) {
    case CLOTH:
        multiplier = 0.6;
        break;
    case LEATHER:
        multiplier = 0.8;
        break;
    case MAIL:
        multiplier = 1.0;
        break;
    case PLATE:
        multiplier = 1.4;
        break;
    case SHIELD:
        multiplier = 5.0;
        break;
    case WEAPON:
        multiplier = 0;
        break;
    case MISC:
    default:
        multiplier = 0.3;
        break;
    }

    size_t final_armor = (size_t) floor(multiplier * base_armor);

    return final_armor;
}


void
gen_item_set(hero_t *      h,
             const size_t  lvl,
             const tier_t  tier,
             const armor_t armor_type)
{
    tier_t  i_tier;
    size_t  i_lvl;
    armor_t i_armor_type;

    if (lvl) {
        i_lvl = lvl;
    }
    else {
        i_lvl = h->level;
    }

    if (tier == RANDOM_TIER) {
        i_tier = gen_item_tier();
    }
    else {
        i_tier = tier;
    }

    if (armor_type == RANDOM_A) {
        i_armor_type = safer_rand(0, PLATE);
    }
    else {
        i_armor_type = armor_type;
    }

    h->items[HEAD] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, HEAD, 0);
    h->items[SHOULDERS] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, SHOULDERS, 0);
    h->items[CHEST] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, CHEST, 0);
    h->items[LEGS] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, LEGS, 0);
    h->items[HANDS] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, HANDS, 0);
    h->items[FEET] = gen_item(0, i_lvl, i_tier, 0, i_armor_type, FEET, 0);

    return;
}


void
spawn_item_drop(hero_t * h)
{
    // Anything from mana potions to armor, swords, etc.
    // Tiers of quality? Rare, epic, legendary?
    // Tier of quality should be influenced by tier of mob?
    size_t trigger = safer_rand(0, 100);

    if (trigger < ITEM_DROP_THRESH) {
        return;
    }

    // This feels goofy.
    size_t drop_type = safer_rand(0, 2);
    item_t new_item;

    switch (drop_type) {
    case 0:
        new_item = gen_item(0, h->level, RANDOM_TIER, 0, RANDOM_A, RANDOM_S, RANDOM_W);

        break;

    case 1:
        memset(&new_item, 0, sizeof(new_item));
        new_item.slot = HP_POTION;
        sprintf(new_item.name, "Health Potion");
        break;

    case 2:
    default:
        memset(&new_item, 0, sizeof(new_item));
        new_item.slot = MP_POTION;
        sprintf(new_item.name, "Mana Potion");
        break;
    }
    new_item = gen_item(0, h->level, RANDOM_TIER, 0, RANDOM_A, RANDOM_S, RANDOM_W);

    choose_inventory(h, &new_item);

    rpg_tui_move_cursor(1, 1);
    rpg_tui_del_eof();

    set_hp_mp_bp(h);

    rpg_tui_move_cursor(1, 1);
    rpg_tui_del_eof();

    return;
}
