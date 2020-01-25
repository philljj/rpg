#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "combat_stats.h"
#include "ability_callbacks.h"
#include "tui.h"

// TODO: use return codes and let calling function
//       decide what to print. These functions
//       should merely mutate hero/enemy state.



static void
weapon_attack_i(hero_t *       hero,
                hero_t *       enemy,
                const item_t * weapon)
{
    {
        // Check if attacking unit is incapacitated.
        for (size_t i = 0; i < MAX_DEBUFFS; ++i) {
            if (!hero->debuffs[i].rounds) {
                continue;
            }

            if (hero->debuffs[i].status == STUN ||
                hero->debuffs[i].status == SLEEP) {
                return;
            }
        }
    }

    {
        // Calculate dodge first. If attack misses, nothing left to do.
        float dodge = get_dodge(enemy);
        float trigger = safer_rand(0, 10000);

        if (dodge > trigger) {
            printf("%s attacked %s and missed\n", hero->name,
                   enemy->name);

            goto END_ATTACK;
        }
    }

    float  base_dmg;
    float  mitigation;
    size_t is_crit = 0;
    size_t final_dmg;

    base_dmg = get_melee_dmg(hero, weapon, STD_SMEAR);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = safer_rand(0, 10000);

        if (crit > trigger) {
            is_crit = 1;
            base_dmg = base_dmg * 1.5;
        }
    }

    mitigation = get_mitigation(enemy);

    final_dmg = (size_t) floor(base_dmg * mitigation);

    if (!final_dmg) {
        // Always at least 1 hp of damage.
        ++final_dmg;
    }

    // Reduce enemy barrier and health.
    size_t hp_reduced = attack_barrier(final_dmg, enemy);

    // TODO: print overkill?

    if (is_crit) {
        printf("%s crit %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }
    else {
        printf("%s attacked %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }

END_ATTACK:
    increment_row();

    return;
}




void
weapon_attack_cb(void * h,
                 void * e)
{
    hero_t * hero  = h;
    hero_t * enemy = e;

    size_t main_hand = hero->items[MAIN_HAND].slot != NO_ITEM;
    size_t off_hand = hero->items[OFF_HAND].slot != NO_ITEM;
    size_t two_hand = hero->items[TWO_HAND].slot != NO_ITEM;

    {
        const item_t * weapon = &hero->items[MAIN_HAND];

        if (main_hand && weapon->armor_type == WEAPON) {
            weapon_attack_i(hero, enemy, weapon);
        }
        else if (!main_hand && !two_hand) {
            // Unarmed attack.
            weapon_attack_i(hero, enemy, weapon);
        }
        else {
            // Something in main hand that's not a weapon. Do nothing.
        }
    }

    {
        const item_t * weapon = &hero->items[OFF_HAND];

        if (off_hand && weapon->armor_type == WEAPON) {
            weapon_attack_i(hero, enemy, weapon);
        }
        else if (!off_hand && !two_hand) {
            // Unarmed attack.
            weapon_attack_i(hero, enemy, weapon);
        }
        else {
            // Something in off hand that's not a weapon. Do nothing.
        }
    }

    {
        const item_t * weapon = &hero->items[TWO_HAND];

        if (two_hand && weapon->armor_type == WEAPON) {
            weapon_attack_i(hero, enemy, weapon);
        }
        else {
            // Something in two hand that's not a weapon. Do nothing.
        }
    }


    return;
}



size_t
spell_attack_cb(void *          h,
                void *          e,
                const element_t element,
                const float     dmg_mult,
                const float     mp_mult)
{
    hero_t * hero  = h;
    hero_t * enemy = e;

    {
        size_t cost = get_spell_cost(element, mp_mult);

        if (!spend_mp(hero, cost)) {
            return 0;
        }
    }

    // Spells ignore armor and all procs.
    // Spell resistance functions as armor mitigation.
    // Should spells penetrate barriers?
    float        base_dmg;
    float        resist;
    size_t       is_crit = 0;
    size_t       final_dmg;
    const char * what = elem_to_str(element);

    base_dmg = dmg_mult * get_spell_dmg(hero, element, STD_SMEAR);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = safer_rand(0, 10000);

        if (crit > trigger) {
            is_crit = 1;
            base_dmg = base_dmg * 1.5;
        }
    }

    resist = get_spell_res(enemy, element);

    final_dmg = (size_t) floor(base_dmg * resist);

    if (!final_dmg) {
        // Always at least 1 hp of damage.
        ++final_dmg;
    }

    // TODO: print overkill?

    size_t hp_reduced = final_dmg < enemy->hp ? final_dmg : enemy->hp;

    enemy->hp -= hp_reduced;

    if (is_crit) {
        printf("%s crit for %zu hp damage\n", what, hp_reduced);
    }
    else {
        printf("%s hit for %zu hp damage\n", what, hp_reduced);
    }

    return hp_reduced;
}



size_t
spell_heal_cb(void *      h,
              const float heal_mult,
              const float mp_mult)
{
    hero_t * hero = h;

    {
        size_t cost = get_spell_cost(RESTORATION, mp_mult);

        if (!spend_mp(hero, cost)) {
            return 0;
        }
    }

    const char * what = "cure";

    float heal_amnt = floor(heal_mult * get_spell_dmg(hero, RESTORATION, STD_SMEAR));

    size_t n = restore_hp(hero, (size_t) heal_amnt);

    printf("%s healed %s for %zu hp\n", what, hero->name, n);
    increment_row();

    return heal_amnt;
}



void
dragon_breath_cb(void * h,
                 void * e)
{
    // Breath cannot be dodged or resisted.
    // Breath cannot crit, but has a large variance.
    hero_t * hero = h;
    hero_t * enemy = e;
    float    base_dmg;
    float    smear = 0.01 * (70 + (rand () % 61));

    base_dmg = smear * 2 * (hero->base.str + hero->base.wis);

    size_t hp_reduced = attack_barrier(base_dmg, enemy);

    printf("dragon breath burned %s for %zu hp damage\n",
           enemy->name, hp_reduced);

    increment_row();

    return;
}
