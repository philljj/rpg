// TODO: 1. Implement Time Gauge. Weapon and job type modifies speed
//          gauge fills. E.g. Knight with Dagger in mh fills time gauge
//          faster than with Sword in mh.
//       2. Job based weapon/armor restrictions.
//       3. gold? vendors, can accumulate gold?
//       4. talent points? specializations?
//       5. Boss fights every 5 levels? Must beat boss to unlock
//       6  next span of levels?
//       7. Unlockable special abilities.
//       8. Weather.
#include <assert.h>
#include <curses.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "names.h"
#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "item.h"
#include "tui.h"
#include "combat_stats.h"
#include "ability_callbacks.h"
#include "safer_rand.h"

static char msg_buf[256];
static char prefix[256];
static char postfix[256];

int
main(int    argc   __attribute__((unused)),
     char * argv[] __attribute__((unused)))
{
    hero_t hero;
    hero_t enemy;
    size_t h_lvl = 2;
    size_t e_lvl = 1;

    assert(REGEN < MAX_COOLDOWNS);
    assert(HOT < MAX_DEBUFFS);

    rpg_rand_init();
    rpg_tui_init();

    rpg_roll_player(&hero, h_lvl);

    rpg_tui_print_portrait(&hero, PORTRAIT_ROW, PORTRAIT_COL);

    for (;;) {
        rpg_roll_mob(&enemy, 0, e_lvl, RANDOM_M);
        rpg_tui_print_portrait(&enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        battle(&hero, &enemy);

        if (!hero.hp) {
            break;
        }

        rpg_reset_hp_mp_bp(&hero);

        printw("%s gains %zu xp and %zu gold\n", hero.name, enemy.xp_rew,
               enemy.gold);
        hero.xp += enemy.xp_rew;
        hero.gold += enemy.gold;
        sleep(1);

        if (hero.xp >= hero.xp_req) {
            level_up(&hero);
            e_lvl++;
        }
    }

    return EXIT_SUCCESS;
}


hero_t *
rpg_roll_mob(hero_t *     hero,
             const char * name,
             const size_t lvl,
             mob_t        mob)
{
    if (mob == RANDOM_M)
        mob = rpg_rand(0, DRAGON);

    switch (mob) {
    case HUMANOID:
        return rpg_roll_humanoid(hero, name, lvl);
    case ANIMAL:
        return rpg_roll_animal(hero, name, lvl);
    case DRAGON:
        return rpg_roll_dragon(hero, name, lvl);
    default:
        return rpg_roll_humanoid(hero, name, lvl);
    }
}

static void
rpg_input_name(hero_t * h)
{
    int  done = 0;
    char choice = '\0';
    int  regen_name = 1;

    while (!done) {
        if (regen_name) {
            memset(h->name, '\0', MAX_NAME_LEN);
            strcat(h->name, prefix_list[rpg_rand(0, MAX_PREFIX - 1)]);
            strcat(h->name, suffix_list[rpg_rand(0, MAX_SUFFIX - 1)]);

            rpg_tui_clear_screen();
            printw("character name: %s\n", h->name);
            printw("y to accept, n to regenerate\n");

            regen_name = 0;
        }

        choice = rpg_tui_getch();

        switch (choice) {
        case 'y':
        case 'Y':
            done = 1;
            break;

        case 'n':
        case 'N':
            regen_name = 1;
            break;

        default:
            break;
        }
    }

    rpg_tui_clear_screen();

    return;
}

static void
rpg_equip_job(hero_t * h)
{
    switch (h->sub_type) {
    case SQUIRE:
        h->items[MAIN_HAND] = gen_item("sword", h->level, COMMON, 1, WEAPON,
                                       MAIN_HAND, BLUNT);
        h->items[OFF_HAND] = gen_item("shield", h->level, COMMON, 0, SHIELD,
                                      OFF_HAND, NOT_WEAPON);
        gen_item_set(h, h->level, COMMON, LEATHER);
        break;

    case CHEMIST:
        h->items[MAIN_HAND] = gen_item("dagger", h->level, COMMON, 1, WEAPON,
                                       MAIN_HAND, PIERCING);
        gen_item_set(h, h->level, COMMON, LEATHER);
        break;

    case THIEF:
        h->items[MAIN_HAND] = gen_item("dagger", h->level, COMMON, 1, WEAPON,
                                       MAIN_HAND, PIERCING);
        gen_item_set(h, h->level, COMMON, LEATHER);
        break;

    case CLERIC:
        h->items[MAIN_HAND] = gen_item("dagger", h->level, COMMON, 1, WEAPON,
                                       MAIN_HAND, PIERCING);
        gen_item_set(h, h->level, COMMON, LEATHER);
        break;

    case KNIGHT:
        h->items[MAIN_HAND] = gen_item("sword", h->level, COMMON, 1, WEAPON,
                                      MAIN_HAND, RANDOM_W);
        h->items[OFF_HAND] = gen_item("shield", h->level, COMMON, 0, SHIELD,
                                      OFF_HAND, NOT_WEAPON);
        gen_item_set(h, h->level, COMMON, MAIL);
        break;
    }

    return;
}

int
rpg_set_job(hero_t * h,
            job_t    job,
            int      i)
{
    if (!h->jobs[job].unlocked)
        return 0;

    h->actjobs[i] = &h->jobs[job];

    if (i == 0) {
        h->sub_type = job;
    }

    return 1;
}

void
thief_steal_gold(void * h,
                 void * e)
{

}

void
rpg_unlock_job(hero_t * h,
               job_t    job)
{
    if (h->jobs[job].unlocked)
        return;

    switch (job) {
    case SQUIRE:
        strcpy(h->jobs[job].name, "Basic Skill");
        h->jobs[job].color = COLOR_RED;
        h->jobs[job].job_cb = squire_skills;

        strcpy(h->jobs[job].skills[0].name, "Steal Gold");
        h->jobs[job].skills[0].skill_cb = thief_steal_gold;
        h->jobs[job].skills[0].unlocked = 1;

        break;
    case CHEMIST:
        strcpy(h->jobs[job].name, "Item");
        h->jobs[job].color = COLOR_RED;
        h->jobs[job].job_cb = chemist_skills;
        break;
    case THIEF:
        strcpy(h->jobs[job].name, "Steal");
        h->jobs[job].color = COLOR_RED;
        h->jobs[job].job_cb = thief_skills;
        break;
    case CLERIC:
        strcpy(h->jobs[job].name, "CLERIC");
        h->jobs[job].color = COLOR_RED;
        h->jobs[job].job_cb = cleric_skills;
        break;
    case KNIGHT:
        strcpy(h->jobs[job].name, "KNIGHT");
        h->jobs[job].color = COLOR_RED;
        h->jobs[job].job_cb = knight_skills;
        break;
    }

    h->jobs[job].unlocked = 1;
    h->jobs[job].job = job;
    h->jobs[job].job_level = 1;
    h->jobs[job].jp = 1;
    h->jobs[job].jp_req = 1;

    return;
}

hero_t *
rpg_roll_player(hero_t *     h,
                const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->gold = 10;
    h->attack = weapon_attack_cb;
    h->level = lvl ? lvl : 1;
    h->xp_req = 1;

    rpg_gen_base_stats(h);
    rpg_input_name(h);

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        h->items[i].slot = NO_ITEM;
    }

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        h->inventory[i].slot = NO_ITEM;
    }

    job_t job;

    for (;;) {
        rpg_tui_clear_screen();
        printw("choose specialization:\n");
        printw("  s - Squire. Unlocks back stab.\n");
        printw("  c - Chemist. Unlocks Use / Throw / Mix items.\n");
        printw("  t - Thief. Steal gold.\n");

        size_t done = 0;
        char   choice = rpg_tui_getch();

        switch (choice) {
        case 't': /* thief */
        case 'T':
            job = THIEF;
            done = 1;
            break;

        case 'c': /* chemist */
        case 'C':
            job = CHEMIST;
            done = 1;
            break;

        case 's': /* squire */
        case 'S':
            job = SQUIRE;
            done = 1;
            break;

        default:
            break;
        }

        if (done) { break; }
    }

    rpg_unlock_job(h, job);
    rpg_set_job(h, job, 0);

    rpg_tui_clear_screen();
    rpg_equip_job(h);
    rpg_reset_hp_mp_bp(h);

    return h;
}

hero_t *
rpg_roll_humanoid(hero_t *     h,
                  const char * name,
                  const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->mob_type = HUMANOID;
    h->sub_type = rpg_rand(0, KNIGHT);

    h->gold = rpg_rand(5, 15);
    h->xp_rew = rpg_rand(1, 3);
    h->attack = weapon_attack_cb;

    h->level = lvl ? lvl : 1;

    rpg_gen_base_stats(h);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        switch (h->sub_type) {
        case SQUIRE:
            strcpy(h->name, "squire");
            break;
        case CHEMIST:
            strcpy(h->name, "chemist");
            break;
        case THIEF:
            strcpy(h->name, "thief");
            break;
        case KNIGHT:
            strcpy(h->name, "knight");
            break;
        default:
            strcpy(h->name, "thief");
            break;
        }
    }

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        h->items[i].slot = NO_ITEM;
    }

    rpg_equip_job(h);
    rpg_reset_hp_mp_bp(h);

    return h;
}


hero_t *
rpg_roll_animal(hero_t *     h,
                const char * name,
                const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->xp_rew = rpg_rand(1, 3);
    h->mob_type = ANIMAL;
    h->sub_type = rpg_rand(0, BEAR);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        const size_t j = rpg_rand(0, NUM_MOB_SUB_TYPES);
        switch (h->sub_type) {
        case DOG:
            strcpy(h->name, dog_list[j]);
            break;
        case CAT:
            strcpy(h->name, cat_list[j]);
            break;
        case BOAR:
            strcpy(h->name, boar_list[j]);
            break;
        case BEAR:
            strcpy(h->name, bear_list[j]);
            break;
        default:
        case FLYING:
            strcpy(h->name, flying_list[j]);
            break;
        }
    }

    h->attack = weapon_attack_cb;

    h->level = lvl ? lvl : 1;

    rpg_gen_base_stats(h);

    switch (h->sub_type) {
    case BEAR:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     BLUNT);

        gen_item_set(h, h->level, COMMON, PLATE);

        break;

    case BOAR:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     EDGED);

        gen_item_set(h, h->level, COMMON, MAIL);

        break;

    case DOG:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(h, h->level, COMMON, LEATHER);

        break;

    case CAT:
    case FLYING:
    default:
        h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                       PIERCING);
        h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, OFF_HAND,
                                       PIERCING);

        gen_item_set(h, h->level, COMMON, CLOTH);

        break;
    }

    rpg_reset_hp_mp_bp(h);

    return h;
}


hero_t *
rpg_roll_dragon(hero_t *     h,
                const char * name,
                const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    // More powerful dragon sub_type appear at higher levels.
    //
    // TODO: need better way to do this. This will break
    //       at high enough level.
    size_t d_lvl = (lvl / 10) + 1;

    h->gold = rpg_rand(5, 15);
    h->xp_rew = rpg_rand(1, 3);
    h->mob_type = DRAGON;
    h->sub_type = rpg_rand(0, d_lvl);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        const size_t j = rpg_rand(0, NUM_MOB_SUB_TYPES);
        switch (h->sub_type) {
        case FOREST_DRAGON:
            strcpy(h->name, forest_dragon_list[j]);
            break;
        case SAND_DRAGON:
            strcpy(h->name, sand_dragon_list[j]);
            break;
        case WATER_DRAGON:
            strcpy(h->name, water_dragon_list[j]);
            break;
        case FIRE_DRAGON:
            strcpy(h->name, fire_dragon_list[j]);
            break;
        default:
        case WHELPLING:
            strcpy(h->name, whelp_list[j]);
            break;
        }
    }

    h->attack = weapon_attack_cb;

    h->level = lvl ? lvl : 1;

    rpg_gen_base_stats(h);

    switch (h->sub_type) {
    case FOREST_DRAGON:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                      BLUNT);

        gen_item_set(h, h->level, COMMON, LEATHER);

        break;

    case SAND_DRAGON:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     EDGED);

        gen_item_set(h, h->level, COMMON, MAIL);

        break;

    case WATER_DRAGON:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(h, h->level, COMMON, LEATHER);

        break;

    case FIRE_DRAGON:
        // Fire breathing dragon. The real deal.
        h->attack = dragon_breath_cb;

        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(h, h->level, COMMON, PLATE);

        break;

    case WHELPLING:
    default:
        h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                       PIERCING);
        h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, OFF_HAND,
                                       PIERCING);

        gen_item_set(h, h->level, COMMON, CLOTH);

        break;
    }

    rpg_reset_hp_mp_bp(h);

    return h;
}


void
rpg_gen_base_stats(hero_t * h)
{
    h->base.sta = h->level + BASE_STAT + (rpg_rand(0, BASE_STAT_VAR));
    h->base.str = h->level + BASE_STAT + (rpg_rand(0, BASE_STAT_VAR));
    h->base.agi = h->level + BASE_STAT + (rpg_rand(0, BASE_STAT_VAR));
    h->base.wis = h->level + BASE_STAT + (rpg_rand(0, BASE_STAT_VAR));
    h->base.spr = h->level + BASE_STAT + (rpg_rand(0, BASE_STAT_VAR));

    return;
}


void
level_up(hero_t * h)
{
    ++(h->level);
    ++(h->xp_req);
    ++(h->base.sta);
    ++(h->base.str);
    ++(h->base.agi);
    ++(h->base.wis);
    ++(h->base.spr);

    rpg_tui_print_portrait(h, PORTRAIT_ROW, PORTRAIT_COL);

    {
        // Spend a bonus point.
        printw("%s leveled up!\n", h->name);
        printw("\n");
        printw("Choose a stat for your bonus point:\n");
        printw("   1. stamina:  increases max hp\n");
        printw("   2. strength: increases melee damage\n");
        printw("   3. agility:  increases dodge and crit chance\n");
        printw("   4. wisdom:   increases max mp and spell damage\n");
        printw("   5. spirit:   increases hp and mp regen rates\n");

        size_t done = 0;

        for (;;) {
            char choice = rpg_tui_getch();

            switch (choice) {
            case '1':
                ++(h->base.sta);
                done = 1;
                break;

            case '2':
                ++(h->base.str);
                done = 1;
                break;

            case '3':
                ++(h->base.agi);
                done = 1;
                break;

            case '4':
                ++(h->base.wis);
                done = 1;
                break;

            case '5':
                ++(h->base.spr);
                done = 1;
                break;

            default:
                printw("error: invalid input %c\n", choice);
                break;
            }

            if (done) { break; }
        }
    }

    h->xp = 0;

    rpg_reset_hp_mp_bp(h);

    rpg_tui_print_portrait(h, PORTRAIT_ROW, PORTRAIT_COL);

    rpg_tui_del_eof();

    return;
}


void
rpg_reset_hp_mp_bp(hero_t * h)
{
    h->hp = get_max_hp(h);
    h->mp = get_max_mp(h);
    h->bp = 0;

    return;
}


size_t
get_max_hp(const hero_t * h)
{
    size_t stamina = h->base.sta;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to do.
            continue;
        }

        stamina += h->items[i].attr.sta;
    }

    return (HP_MULT * stamina);
}


size_t
get_max_mp(const hero_t * h)
{
    size_t wisdom = h->base.wis;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to do.
            continue;
        }

        wisdom += h->items[i].attr.wis;
    }

    return (MP_MULT * wisdom);
}


size_t
restore_hp(hero_t *     h,
           const size_t amnt)
{
    size_t max_hp = get_max_hp(h);
    size_t old_hp = h->hp;
    size_t hp = h->hp + amnt;

    h->hp = hp < max_hp ? hp : max_hp;

    return (h->hp - old_hp);
}


size_t
restore_mp(hero_t *     h,
           const size_t amnt)
{
    size_t max_mp = get_max_mp(h);
    size_t old_mp = h->mp;
    size_t mp = h->mp + amnt;

    h->mp = mp < max_mp ? mp : max_mp;

    return (h->mp - old_mp);
}


size_t
spend_hp(hero_t *     h,
         const size_t amnt)
{
    size_t hp = h->hp;

    h->hp = hp < amnt ? hp : hp - amnt;

    return (h->hp < hp);
}


size_t
spend_mp(hero_t *     h,
         const size_t amnt)
{
    size_t mp = h->mp;

    h->mp = mp < amnt ? mp : mp - amnt;

    return (h->mp < mp);
}


void
spirit_regen(hero_t * hero)
{
    // Spirit regen is SPIRIT_REGEN_MULT spirit every REGEN_ROUND rounds.
    size_t regen_amnt = 0;
    float  total_spirit = 0;

    total_spirit = (float) get_total_stat(hero, SPIRIT);
    regen_amnt = (size_t) floor(SPIRIT_REGEN_MULT * total_spirit);

    size_t hp_gain = restore_hp(hero, regen_amnt);
    size_t mp_gain = restore_mp(hero, regen_amnt);

    printw("%s regenerated %zu hp, %zu mp\n", hero->name, hp_gain,
           mp_gain);

    return;
}


void
battle(hero_t * hero,
       hero_t * enemy)
{
    rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
    rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

    size_t regen_ctr = 0;

    for (;;) {
        ++regen_ctr;

        // hero's turn.
        process_cooldowns(hero);
        decision_loop(hero, enemy);

        rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
        rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        usleep(USLEEP_INTERVAL);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        // hero's debuffs.
        if (process_debuffs(hero)) {
            rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

            usleep(USLEEP_INTERVAL);

            if (!hero->hp || !enemy->hp) {
                break;
            }
        }

        // enemy's turn.
        process_cooldowns(enemy);
        enemy->attack(enemy, hero);

        rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
        rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        usleep(USLEEP_INTERVAL);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        // enemy's debuffs.
        if (process_debuffs(enemy)) {
            rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

            usleep(USLEEP_INTERVAL);

            if (!hero->hp || !enemy->hp) {
                break;
            }
        }

        if (regen_ctr == REGEN_ROUND) {
            spirit_regen(hero);
            spirit_regen(enemy);

            rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

            regen_ctr = 0;

            usleep(USLEEP_INTERVAL);
        }
    }

    rpg_tui_reset_row();
    reset_cooldowns(hero);
    clear_debuffs(hero);

    if (!hero->hp && enemy->hp) {
        printw("%s has defeated %s!\n\n", enemy->name, hero->name);
    }
    else if (hero->hp && !enemy->hp) {
        printw("%s has defeated %s!\n\n", hero->name, enemy->name);
    }

    sleep(1);

    usleep(USLEEP_INTERVAL);

    return;
}


void
decision_loop(hero_t * hero,
              hero_t * enemy)
{
    size_t done = 0;
    char   act_var;

    for (;;) {
        rpg_tui_print_act_prompt(hero, ACTION_ROW, ACTION_COL);
        act_var = rpg_tui_getch();
        rpg_tui_clear_act_prompt(hero, ACTION_ROW, ACTION_COL);

        switch (act_var) {
        case 'a':
            hero->attack(hero, enemy);
            done = 1;
            break;

        case 's':
            if (!hero->actjobs[0] || !hero->actjobs[0]->unlocked)
                continue;

            hero->actjobs[0]->job_cb(hero, enemy);

            break;

        case 'd':
            if (!hero->actjobs[1] || !hero->actjobs[1]->unlocked)
                continue;

            hero->actjobs[1]->job_cb(hero, enemy);

            break;

        default:
            printw("error: invalid input %c\n", act_var);
            break;
        }

        if (done) { break; }
    }

    return;
}

size_t
attack_barrier(size_t   final_dmg,
               hero_t * enemy)
{
    size_t bp = enemy->bp;
    size_t hp = enemy->hp;
    size_t hp_reduced = 0;

    if (bp) {
        /* Reduce the barrier first. */
        size_t dmg = final_dmg < bp ? final_dmg : bp;
        final_dmg -= dmg;
        enemy->bp -= dmg;
    }

    if (final_dmg) {
        /* Then the health pool with remaining dmg. */
        hp_reduced = final_dmg < hp ? final_dmg : enemy->hp;
        enemy->hp -= hp_reduced;
    }

    return hp_reduced;
}


const char *
slot_to_str(slot_t s)
{
    switch(s) {
    case MAIN_HAND:
        return "main hand";

    case OFF_HAND:
        return "off hand";

    case TWO_HAND:
        return "two hand";

    case HEAD:
        return "head";

    case SHOULDERS:
        return "shoulders";

    case CHEST:
        return "chest";

    case LEGS:
        return "legs";

    case HANDS:
        return "hands";

    case FEET:
        return "feet";

    case RING:
        return "ring";

    case TRINKET:
        return "trinket";

    case HP_POTION:
    case MP_POTION:
        return "consumable";

    case NO_ITEM:
        return "empty";

    default:
        return "NA";
    }
}


const char *
armor_to_str(armor_t a)
{
    switch (a) {
    case CLOTH:
        return "cloth";
    case LEATHER:
        return "leather";
    case MAIL:
        return "mail";
    case PLATE:
        return "plate";
    default:
        return "";
    }
}


const char *
mob_to_str(const mob_t m)
{
    switch(m) {
    case HERO:
        return "hero";

    case HUMANOID:
        return "humanoid";

    case DRAGON:
        return "dragon";

    default:
        return "monster";
    }
}

item_t *
add_to_inventory(hero_t * h,
                 item_t * new_item)
{
    // TODO: need to make consumables stack.
    if (!new_item || new_item->slot == NO_ITEM) {
        // No item to add.
        return 0;
    }

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        if (h->inventory[i].slot == NO_ITEM) {
            h->inventory[i] = *new_item;

            memset(new_item, 0, sizeof(item_t));
            new_item->slot = NO_ITEM;

            return 0;
        }
    }

    // Inventory full.
    return new_item;
}

void
chemist_skills(void * h,
              void * e)
{
    hero_t * hero = h;

    (void) e;
}

void
squire_skills(void * h,
             void * e)
{
    hero_t * hero = h;
    hero_t * enemy = e;

}

void
thief_skills(void * h,
             void * e)
{
    hero_t * hero = h;
    hero_t * enemy = e;
    char     button = 'a';
    // steal gold first ability

    for (int i = 0; i < NUM_SKILLS; ++i) {
        if (hero->jobs[THIEF].skills[i].unlocked) {
            mvprintw(i, 0, "    %c: %s\n", button + i,
                     hero->jobs[THIEF].skills[i].name);

        }
    }
}

void
knight_skills(void * h,
             void * e)
{
    hero_t * hero = h;
    hero_t * enemy = e;

}

void
cleric_skills(void * h,
             void * e)
{
    hero_t * hero = h;
    hero_t * enemy = e;

}

stats_t
get_total_stats(const hero_t * h)
{
    // Should pass stats_t by pointer?
    stats_t stats;

    stats.sta = h->base.sta;
    stats.str = h->base.str;
    stats.agi = h->base.agi;
    stats.wis = h->base.wis;
    stats.spr = h->base.spr;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            // Nothing to do.
            continue;
        }

        stats.sta += h->items[i].attr.sta;
        stats.str += h->items[i].attr.str;
        stats.agi += h->items[i].attr.agi;
        stats.wis += h->items[i].attr.wis;
        stats.spr += h->items[i].attr.spr;
    }

    return stats;
}


size_t
fire_strike(hero_t * hero,
            hero_t * enemy)
{
    // Burn the target for X fire damage, and then 10%
    // additional damage on the target's next turn.
    size_t fire_dmg = 10;
    (void) hero;

    if (!fire_dmg) {
        // Spell cast failed for some reason.
        return 0;
    }
    // else, spell succeeded. Apply burn debuff.

    float burn_amnt = 0.1 * ((float) fire_dmg);

    apply_debuff(enemy, "burn", DOT, FIRE, burn_amnt, 1, 0);

    return fire_dmg;
}


/* Main hand piercing attack that does 3-4x damage, depending
 * on weapon quality. Requires MAIN_HAND of PIERCING type. */
size_t
back_stab(hero_t * hero,
          hero_t * enemy)
{
    if (!hero->cooldowns[BACK_STAB].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[BACK_STAB].rounds) {
        sprintf(msg_buf, "back stab on cooldown for %zu rounds\n",
                hero->cooldowns[BACK_STAB].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    if (hero->items[MAIN_HAND].slot == NO_ITEM ||
        hero->items[MAIN_HAND].weapon_type != PIERCING) {
        sprintf(msg_buf, "no dagger equipped\n");
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    float  mult = 3.0;

    switch (hero->items[MAIN_HAND].tier) {
    case GOOD:
        mult = 3.3;
        break;

    case RARE:
        mult = 3.6;
        break;

    case EPIC:
        mult = 3.9;
        break;

    case COMMON:
    default:
        break;
    }

    float  bs_dmg = get_melee_dmg(hero, &hero->items[MAIN_HAND], STD_SMEAR);
    float  mitigation = get_mitigation(enemy);
    size_t final_dmg = (size_t) floor(mult * bs_dmg * mitigation);

    if (!final_dmg) {
        /* Back stab failed for some reason. */
        return 0;
    }

    size_t hp_reduced = attack_barrier(final_dmg, enemy);

    sprintf(msg_buf, "back stab hit %s for %zu melee damage\n",
            enemy->name, hp_reduced);
    rpg_tui_print_combat_txt(msg_buf);

    hero->cooldowns[BACK_STAB].rounds = 4;

    return hp_reduced;
}

/* Two hand attack that does 2-3x damage, depending on weapon
 * quality. Bypasses %50 of armor mitigation */
size_t
crushing_blow(hero_t * hero,
              hero_t * enemy)
{
    if (!hero->cooldowns[CRUSHING_BLOW].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[CRUSHING_BLOW].rounds) {
        printw("crushing blow on cooldown for %zu rounds\n",
               hero->cooldowns[CRUSHING_BLOW].rounds);
        return 0;
    }

    if (hero->items[TWO_HAND].slot == NO_ITEM) {
        printw("no two hand weapon equipped\n");
        return 0;
    }

    float  mult = 2.0;

    switch (hero->items[TWO_HAND].tier) {
    case GOOD:
        mult = 2.33;
        break;

    case RARE:
        mult = 2.66;
        break;

    case EPIC:
        mult = 2.99;
        break;

    case COMMON:
    default:
        break;
    }

    float  cb_dmg = get_melee_dmg(hero, &hero->items[TWO_HAND], STD_SMEAR);
    float  mitigation = get_mitigation_w_bypass(enemy, 0.5);
    size_t final_dmg = (size_t) floor(mult * cb_dmg * mitigation);

    if (!final_dmg) {
        // failed for some reason.
        return 0;
    }
    // else, succeeded.

    // Reduce enemy barrier and health.
    size_t hp_reduced = attack_barrier(final_dmg, enemy);

    sprintf(msg_buf, "crushing blow hit %s for %zu melee damage\n",
            enemy->name, hp_reduced);
    rpg_tui_print_combat_txt(msg_buf);

    hero->cooldowns[CRUSHING_BLOW].rounds = 4;

    return hp_reduced;
}

/*
 * A necromantic weapon strike that does weapon damage as
 * shadow damage, plus any bonus shadow spell power.
 *
 * The caster is healed for %50 of damage dealt.
 */
size_t
drain_touch(hero_t * hero,
            hero_t * enemy)
{
    if (!hero->cooldowns[DRAIN_TOUCH].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[DRAIN_TOUCH].rounds) {
        sprintf(msg_buf, "drain touch on cooldown for %zu rounds\n",
               hero->cooldowns[DRAIN_TOUCH].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    size_t total_dmg = 0;

    /* Check 2 hand weapon first. If it is equipped, then stop
     * after it does damage. Otherwise process both hands.*/
    slot_t weaps[3] = { TWO_HAND, MAIN_HAND, OFF_HAND };

    for (size_t i = 0; i < 3; ++i) {
        float drain_touch_dmg = get_melee_dmg(hero, &hero->items[weaps[i]],
                                              STD_SMEAR);

        if (drain_touch_dmg == 0) { continue; }

        /* factor in melee mitigation */
        drain_touch_dmg *= get_mitigation(enemy);

         /* add in shadow spell power */
        drain_touch_dmg += get_elem_pow(&hero->power, SHADOW);

        size_t final_dmg = (size_t) floor(drain_touch_dmg);
        size_t hp_reduced = attack_barrier(final_dmg, enemy);

        sprintf(prefix, "drain touch hit %s for %zu ",
               enemy->name, hp_reduced);
        sprintf(postfix, " damage\n");
        rpg_tui_print_combat_color_txt(prefix, SHADOW, postfix);

        total_dmg += hp_reduced;

        hp_reduced /= 2;

        if (hp_reduced == 0) { ++hp_reduced; }

        size_t n = restore_hp(hero, hp_reduced);
        sprintf(msg_buf, "drain touch healed %s for %zu hp\n", hero->name, n);
        rpg_tui_print_combat_txt(msg_buf);

        if (total_dmg && weaps[i] == TWO_HAND) {
            // A 2 hand weapon was equipped.
            break;
        }
    }

    // Reduce enemy barrier and health.

    hero->cooldowns[DRAIN_TOUCH].rounds = 2;

    return total_dmg;
}

/* Bash the target for 0.6 main hand dmg,
 * and stuns target for one round. */
size_t
shield_bash(hero_t * hero,
            hero_t * enemy)
{
    if (!hero->cooldowns[SHIELD_BASH].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[SHIELD_BASH].rounds) {
        sprintf(msg_buf, "shield bash on cooldown for %zu rounds\n",
               hero->cooldowns[SHIELD_BASH].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    if (hero->items[OFF_HAND].slot == NO_ITEM ||
        hero->items[OFF_HAND].armor_type != SHIELD) {
        sprintf(msg_buf, "no shield equipped\n");
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    float  mult = 0.5;

    switch (hero->items[OFF_HAND].tier) {
        case GOOD:
            mult = 0.7;
            break;

        case RARE:
            mult = 0.9;
            break;

        case EPIC:
            mult = 1.1;
            break;

        case COMMON:
        default:
            break;
    }

    float  sb_dmg = get_melee_dmg(hero, &hero->items[MAIN_HAND], STD_SMEAR);
    float  mitigation = get_mitigation(enemy);
    size_t final_dmg = (size_t) floor(mult * sb_dmg * mitigation);

    if (!final_dmg) {
        // Bash failed for some reason.
        return 0;
    }
    // else, succeeded. Apply stun debuff.

    // Reduce enemy barrier and health.
    size_t hp_reduced = attack_barrier(final_dmg, enemy);

    sprintf(msg_buf, "shield bash hit %s for %zu hp damage\n",
            enemy->name, hp_reduced);
    rpg_tui_print_combat_txt(msg_buf);

    apply_debuff(enemy, "shield bash", STUN, NON_ELEM, 0, 1, 0);

    hero->cooldowns[SHIELD_BASH].rounds = 4;

    return hp_reduced;
}


size_t
fireball(hero_t * hero,
         hero_t * enemy)
{
    // Burn the target for %150 fire damage, and then 50%
    // additional damage over 3 rounds.

    if (!hero->cooldowns[FIREBALL].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[FIREBALL].rounds) {
        sprintf(msg_buf, "fireball on cooldown for %zu rounds\n",
                hero->cooldowns[FIREBALL].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    size_t fire_dmg = 20;

    if (!fire_dmg) {
        // Spell cast failed for some reason.
        return 0;
    }
    // else, spell succeeded. Apply burn debuff.

    size_t rounds = 3;
    float  dot_mult = 0.5;
    float  burn_amnt = (dot_mult * ((float) fire_dmg)) / rounds;

    apply_debuff(enemy, "burn", DOT, FIRE, burn_amnt, rounds, 0);

    hero->cooldowns[FIREBALL].rounds = 4;

    return fire_dmg;
}

/* Strike target for holy damage, proportional to total spirit.
 * No mana cost. */
size_t
holy_smite(hero_t * hero,
           hero_t * enemy)
{
    if (!hero->cooldowns[HOLY_SMITE].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[HOLY_SMITE].rounds) {
        sprintf(msg_buf, "holy smite on cooldown for %zu rounds\n",
               hero->cooldowns[HOLY_SMITE].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    size_t total_spirit = get_total_stat(hero, SPIRIT);
    size_t hp_reduced = attack_barrier(total_spirit, enemy);

    sprintf(msg_buf, "smite hit %s for %zu hp damage\n",
            enemy->name, hp_reduced);
    rpg_tui_print_combat_txt(msg_buf);

    hero->cooldowns[HOLY_SMITE].rounds = 2;

    return hp_reduced;
}

/*
 * Swarm the enemy target with insects, dealing damage over time.
 *   - Damage proportional to strength + spirit
 *   - Damage lasts for 5 rounds.
 *
 * This debuff is stackable, and has no mana cost.
 */
size_t
insect_swarm(hero_t * hero,
             hero_t * enemy)
{
    if (!hero->cooldowns[INSECT_SWARM].unlocked) {
        /* Haven't learned this ability yet. */
        return 0;
    }

    if (hero->cooldowns[INSECT_SWARM].rounds) {
        sprintf(msg_buf, "insect swarm on cooldown for %zu rounds\n",
                hero->cooldowns[HOLY_SMITE].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    float total_spirit = (float) get_total_stat(hero, SPIRIT);
    float total_str = (float) get_total_stat(hero, STRENGTH);

    size_t rounds = 5;
    float  dot_mult = 0.5;
    float  dot_amnt = (dot_mult * (total_spirit + total_str)) / rounds;

    apply_debuff(enemy, "insect swarm", DOT, NATURE, dot_amnt, rounds, 0);

    hero->cooldowns[INSECT_SWARM].rounds = 1;

    return dot_amnt;
}


size_t
time_mage_regen(hero_t * hero)
{
    // Heal caster for 50%, and then 100%
    // additional over 2 rounds.

    if (!hero->cooldowns[REGEN].unlocked) {
        // Haven't learned this ability yet.
        return 0;
    }

    if (hero->cooldowns[REGEN].rounds) {
        sprintf(msg_buf, "regen on cooldown for %zu rounds\n",
                hero->cooldowns[REGEN].rounds);
        rpg_tui_print_combat_txt(msg_buf);
        return 0;
    }

    size_t heal_amnt = 10;

    if (!heal_amnt) {
        // Spell cast failed for some reason.
        return 0;
    }
    // else, spell succeeded. Apply restore buff.

    size_t wait = 1;
    size_t rounds = 2;
    float  hot_mult = 2;
    float  hot_amnt = (hot_mult * ((float) heal_amnt)) / rounds;

    apply_debuff(hero, "regen", HOT, RESTORATION,
                 hot_amnt, rounds, wait);

    hero->cooldowns[REGEN].rounds = 4;

    return heal_amnt;
}


void
apply_debuff(hero_t *          enemy,
             const char *      name,
             const db_status_t status,
             const element_t   element,
             const float       amnt,
             const size_t      rounds,
             const size_t      delay)
{
    debuff_t * db = 0;

    // Find the next empty debuff slot, and set ptr to it.
    for (size_t i = 0; i < MAX_DEBUFFS; ++i) {
        if (!enemy->debuffs[i].rounds) {
            // This debuff has expired (or never existed).
            db = &enemy->debuffs[i];
            break;
        }
    }

    if (!db) {
        // Target enemy already has MAX_DEBUFFS applied.
        return;
    }

    memset(db, 0, sizeof(debuff_t));

    strcpy(db->name, name);
    db->status = status;
    db->element = element;
    db->amnt = amnt;
    db->rounds = rounds;
    db->wait = delay;

    return;
}


size_t
process_debuffs(hero_t * enemy)
{
    size_t status = 0;

    for (size_t i = 0; i < MAX_DEBUFFS; ++i) {
        if (!enemy->debuffs[i].rounds) {
            continue;
        }

        process_debuffs_i(enemy, &enemy->debuffs[i]);
        status = 1;
    }

    return status;
}


void
process_debuffs_i(hero_t *   enemy,
                  debuff_t * debuff)
{
    if (debuff->wait) {
        debuff->wait--;
        return;
    }

    const char * name = debuff->name;
    db_status_t  db_status = debuff->status;
    element_t    element = debuff->element;
    float        amnt = debuff->amnt;
    size_t       dmg = 0;
    char         prefix[256];
    char         postfix[256];

    switch (db_status) {
    case DOT:
        // We don't have to consider spell resist or armor
        // here, because mitigation was already taken into
        // account when the debuff was calculated and applied.
        dmg = attack_barrier(amnt, enemy);
        sprintf(prefix, "%s did %zu ", name, dmg);
        sprintf(postfix, " damage to %s\n", enemy->name);
        rpg_tui_print_combat_color_txt(prefix, element, postfix);
        break;

    case STUN:
        sprintf(msg_buf, "%s stunned %s\n", name, enemy->name);
        rpg_tui_print_combat_txt(msg_buf);
        break;

    case HOT:
        dmg = restore_hp(enemy, (size_t) floor(amnt));
        sprintf(msg_buf, "%s did %zu healing to %s\n", name, dmg, enemy->name);
        rpg_tui_print_combat_txt(msg_buf);
        break;

    default:
        // Do nothing
        break;
    }

    debuff->rounds--;

    return;
}


void
clear_debuffs(hero_t * enemy)
{
    for (size_t i = 0; i < MAX_DEBUFFS; ++i) {
        if (!enemy->debuffs[i].rounds) {
            continue;
        }

        memset(&enemy->debuffs[i], 0, sizeof(debuff_t));
    }

    return;
}


void
process_cooldowns(hero_t * h)
{
    for (size_t i = 0; i < MAX_COOLDOWNS; ++i) {
        if(h->cooldowns[i].rounds) {
            h->cooldowns[i].rounds--;
        }
    }

    return;
}


void
reset_cooldowns(hero_t * h)
{
    for (size_t i = 0; i < MAX_COOLDOWNS; ++i) {
        h->cooldowns[i].rounds = 0;
    }

    return;
}


