// TODO: 1. This needs serious cleanup. Refactor and clean this up.
//       2. need way to track temporary buffs/debuffs.
//       3. gold? vendors, can accumulate gold?
//       4. talent points? specializations?
//       5. Boss fights every 5 levels? Must beat boss to unlock
//          next span of levels?
//       6. Unlockable special abilities. Druids shadeshift to animal
//          forms, but can't equip plate or weapons to use this ability.
//       7. Weather.
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "safer_rand.h"
#include "names.h"
#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "item.h"
#include "tui.h"
#include "combat_stats.h"
#include "ability_callbacks.h"
#include "color.h"

#include <curses.h>

static char msg_buf[256];

int
main(int    argc   __attribute__((unused)),
     char * argv[] __attribute__((unused)))
{
    initscr();
    noecho();
    cbreak();
    timeout(-1);
    start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(1));

    assert(REGEN < MAX_COOLDOWNS);
    assert(HOT < MAX_DEBUFFS);

    init_rand();
    rpg_tui_clear_screen();

    size_t h_lvl = 2;
    size_t e_lvl = 1;

    hero_t hero;
    hero_t enemy;
    rpg_roll_player(&hero, h_lvl);

    rpg_tui_print_portrait(&hero, PORTRAIT_ROW, PORTRAIT_COL);

    hero.xp_req = 1;

    for (;;) {
        rpg_roll_mob(&enemy, 0, e_lvl, RANDOM_M);
        rpg_tui_print_portrait(&enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        battle(&hero, &enemy);

        if (!hero.hp) {
            break;
        }

        set_hp_mp_bp(&hero);

        printw("%s gains %zu xp and %zu gold\n", hero.name, enemy.xp_rew,
               enemy.gold);
        hero.xp += enemy.xp_rew;
        hero.gold += enemy.gold;
        sleep(1);

        spawn_item_drop(&hero);

        if (hero.xp >= hero.xp_req) {
            level_up(&hero);

            hero.xp_req++;

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
        mob = safer_rand(0, DRAGON);

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
            strcat(h->name, prefix_list[safer_rand(0, MAX_PREFIX - 1)]);
            strcat(h->name, suffix_list[safer_rand(0, MAX_SUFFIX - 1)]);

            rpg_tui_clear_screen();
            printw("character name: %s\n", h->name);
            printw("y to accept, n to regenerate\n");

            regen_name = 0;
        }

        choice = rpg_tui_safer_fgetc();

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

        //rpg_tui_clear_stdin();
    }

    rpg_tui_clear_screen();

    return;
}

hero_t *
rpg_roll_player(hero_t *     h,
                const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->gold = 10;
    h->attack = weapon_attack_cb;
    h->spell = spell_attack_cb;
    h->heal = spell_heal_cb;

    h->level = lvl ? lvl : 1;

    rpg_gen_base_stats(h);

    rpg_input_name(h);

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        h->items[i].slot = NO_ITEM;
    }

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        h->inventory[i].slot = NO_ITEM;
    }

    for (;;) {
        rpg_tui_clear_screen();
        printw("choose specialization:\n");
        printw("  t - Thief. Unlocks back stab.\n");
        printw("  c - Chemist. Unlocks Use / Throw / Mix items.\n");
        printw("  b - Barbarian. Unlocks crushing blow.\n");
        printw("  s - Soldier. Unlocks shield bash.\n");
        printw("  p - Priest. Unlocks holy smite.\n");
        printw("  d - Druid. Unlocks insect swarm.\n");
        printw("  w - Wizard. Unlocks fireball.\n");
        printw("  n - Necromancer. Unlocks drain touch.\n");
        printw("  k - Knight. Unlocks shield wall.\n");
        printw("  g - Geomancer. Unlocks elemental attack.\n");

        size_t done = 0;
        char   choice = rpg_tui_safer_fgetc();

        switch (choice) {
        case 't': /* thief */
        case 'T':
            h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                           PIERCING);
            h->cooldowns[BACK_STAB].unlocked = 1;
            done = 1;
            break;

        case 'b': /* barbarian */
        case 'B':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         RANDOM_W);
            h->cooldowns[CRUSHING_BLOW].unlocked = 1;
            done = 1;
            break;

        case 'c': /* chemist */
        case 'C':
            h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                           PIERCING);

            h->cooldowns[USE_ITEM].unlocked = 1;
            done = 1;
            break;

        case 's': /* soldier */
        case 'S':
            h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                          RANDOM_W);
            h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 0, SHIELD, OFF_HAND,
                                          NO_WEAPON);

            h->cooldowns[SHIELD_BASH].unlocked = 1;
            done = 1;
            break;

        case 'p': /* priest */
        case 'P':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[HOLY_SMITE].unlocked = 1;
            done = 1;
            break;

        case 'd': /* druid */
        case 'D':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[INSECT_SWARM].unlocked = 1;
            done = 1;
            break;

        case 'w': /* wizard */
        case 'W':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[FIREBALL].unlocked = 1;
            done = 1;
            break;

        case 'n': /* necromancer */
        case 'N':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[DRAIN_TOUCH].unlocked = 1;
            done = 1;
            break;

        case 'k': /* knight */
        case 'K':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[SHIELD_WALL].unlocked = 1;
            done = 1;
            break;

        case 'g': /* geomancer */
        case 'G':
            h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                         BLUNT);

            h->cooldowns[ELEMENTAL].unlocked = 1;
            done = 1;
            break;
        }

        //rpg_tui_clear_stdin();

        if (done) { break; }
    }

    rpg_tui_clear_screen();

    gen_item_set(h, h->level, COMMON, CLOTH);

    set_hp_mp_bp(h);

    return h;
}


hero_t *
rpg_roll_humanoid(hero_t *     h,
                  const char * name,
                  const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->mob_type = HUMANOID;
    h->sub_type = safer_rand(0, KNIGHT);

    h->gold = safer_rand(5, 15);
    h->xp_rew = safer_rand(1, 3);
    h->attack = weapon_attack_cb;
    h->spell = spell_attack_cb;
    h->heal = spell_heal_cb;

    h->level = lvl ? lvl : 1;

    rpg_gen_base_stats(h);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        switch (h->sub_type) {
        case BARBARIAN:
            strcpy(h->name, "barbarian");
            break;
        case SOLDIER:
            strcpy(h->name, "soldier");
            break;
        case PRIEST:
            strcpy(h->name, "priest");
            break;
        case WIZARD:
            strcpy(h->name, "wizard");
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

    switch (h->sub_type) {
    case BARBARIAN:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     RANDOM_W);
        gen_item_set(h, h->level, COMMON, LEATHER);

        break;

    case SOLDIER:
        h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                     RANDOM_W);
        h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 0, SHIELD, OFF_HAND,
                                     NO_WEAPON);

        gen_item_set(h, h->level, COMMON, MAIL);

        break;

    case WIZARD:
    case PRIEST:
        h->items[TWO_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, TWO_HAND,
                                     BLUNT);

        gen_item_set(h, h->level, COMMON, CLOTH);

        break;

    case KNIGHT:
        h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                      RANDOM_W);
        h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 0, SHIELD, OFF_HAND,
                                     NO_WEAPON);

        gen_item_set(h, h->level, COMMON, PLATE);

        break;

    case THIEF:
    default:
        h->items[MAIN_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, MAIN_HAND,
                                     RANDOM_W);
        h->items[OFF_HAND] = gen_item(0, h->level, COMMON, 1, WEAPON, OFF_HAND,
                                     PIERCING);

        gen_item_set(h, h->level, COMMON, LEATHER);

        break;

    }

    set_hp_mp_bp(h);

    return h;
}


hero_t *
rpg_roll_animal(hero_t *     h,
                const char * name,
                const size_t lvl)
{
    memset(h, 0, sizeof(hero_t));

    h->xp_rew = safer_rand(1, 3);
    h->mob_type = ANIMAL;
    h->sub_type = safer_rand(0, BEAR);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        const size_t j = safer_rand(0, NUM_MOB_SUB_TYPES);
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

    set_hp_mp_bp(h);

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

    h->gold = safer_rand(5, 15);
    h->xp_rew = safer_rand(1, 3);
    h->mob_type = DRAGON;
    h->sub_type = safer_rand(0, d_lvl);

    if (name && *name) {
        strcpy(h->name, name);
    }
    else {
        const size_t j = safer_rand(0, NUM_MOB_SUB_TYPES);
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

    set_hp_mp_bp(h);

    return h;
}


void
rpg_gen_base_stats(hero_t * h)
{
    h->base.sta = h->level + BASE_STAT + (safer_rand(0, BASE_STAT_VAR));
    h->base.str = h->level + BASE_STAT + (safer_rand(0, BASE_STAT_VAR));
    h->base.agi = h->level + BASE_STAT + (safer_rand(0, BASE_STAT_VAR));
    h->base.wis = h->level + BASE_STAT + (safer_rand(0, BASE_STAT_VAR));
    h->base.spr = h->level + BASE_STAT + (safer_rand(0, BASE_STAT_VAR));

    return;
}


void
level_up(hero_t * h)
{
    ++(h->level);
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
            char choice = rpg_tui_safer_fgetc();

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

            //rpg_tui_clear_stdin();

            if (done) { break; }
        }
    }

    h->xp = 0;

    set_hp_mp_bp(h);

    rpg_tui_print_portrait(h, PORTRAIT_ROW, PORTRAIT_COL);

    rpg_tui_del_eof();

    return;
}


void
set_hp_mp_bp(hero_t * h)
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
        act_var = rpg_tui_safer_fgetc();
        rpg_tui_clear_act_prompt(hero, ACTION_ROW, ACTION_COL);

        switch (act_var) {
        case 'a':
            if (choose_attack(hero, enemy)) {
                done = 1;
            }

            break;

        case 's':
            if (choose_spell(hero, enemy)) {
                done = 1;
            }
            else {
                printw("%s is out of mana\n", hero->name);
            }

            break;

        case 'h':
            if (choose_heal(hero)) {
                done = 1;
            }
            else {
                printw("%s is out of mana\n", hero->name);
            }

            break;

        case 'u':
            if (!hero->cooldowns[USE_ITEM].unlocked)
                break;

            choose_inventory(hero, 0);

            rpg_tui_move_cursor(1, 1);
            rpg_tui_del_eof();

            rpg_tui_print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            rpg_tui_print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

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
choose_spell(hero_t * hero,
             hero_t * enemy)
{
    size_t done = 0;
    size_t status;

    for (;;) {
        rpg_tui_print_spell_prompt(hero, ACTION_ROW, ACTION_COL);

        char act_var = rpg_tui_safer_fgetc();

        rpg_tui_clear_spell_prompt(hero, ACTION_ROW, ACTION_COL);

        switch (act_var) {
        case 'f':
            status = fire_strike(hero, enemy);
            done = 1;
            break;

        case 'i':
            status = hero->spell(hero, enemy, FROST, 1, 1);
            done = 1;
            break;

        case 's':
            status = shadow_bolt(hero, enemy);
            done = 1;
            break;

        case 'u':
            status = hero->spell(hero, enemy, NON_ELEM, 1, 1);
            done = 1;
            break;

        case 'b':
            status = fireball(hero, enemy);
            done = 1;
            break;

        case 'h':
            status = holy_smite(hero, enemy);
            done = 1;
            break;

        case 'n':
            status = insect_swarm(hero, enemy);
            done = 1;
            break;

        default:
            printw("error: invalid input %c\n", act_var);
            break;
        }

        if (done) { break; }
    }

    return status;
}


size_t
choose_heal(hero_t * hero)
{
    size_t done = 0;
    size_t status;

    for (;;) {
        rpg_tui_print_heal_prompt(hero, ACTION_ROW, ACTION_COL);

        char act_var = rpg_tui_safer_fgetc();

        rpg_tui_clear_heal_prompt(hero, ACTION_ROW, ACTION_COL);

        switch (act_var) {
        case 'h':
            status = hero->heal(hero, 1.0, 1.0);
            done = 1;
            break;

        default:
            printw("error: invalid input %c\n", act_var);
            break;
        }

        if (done) { break; }
    }

    return status;
}


size_t
choose_attack(hero_t * hero,
              hero_t * enemy)
{
    size_t done = 0;
    size_t status;
    char   act_var;

    for (;;) {
        rpg_tui_print_attack_prompt(hero, ACTION_ROW, ACTION_COL);

        act_var = rpg_tui_safer_fgetc();

        rpg_tui_clear_attack_prompt(hero, ACTION_ROW, ACTION_COL);

        switch (act_var) {
        case 'a':
            hero->attack(hero, enemy);
            done = 1;
            break;

        case 'b':
            status = back_stab(hero, enemy);
            done = 1;
            break;

        case 'c':
            status = crushing_blow(hero, enemy);
            done = 1;
            break;

        case 'd':
            status = drain_touch(hero, enemy);
            done = 1;
            break;

        case 's':
            status = shield_bash(hero, enemy);
            done = 1;
            break;

        default:
            printw("error: invalid input %c\n", act_var);
            break;
        }

        if (done) { break; }
    }

    return status;
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


const char *
elem_to_str(char *          str,
            const element_t elem)
{
    if (!str) {
        /* simple, no color formatting */
        switch (elem) {
        case FIRE:
            return "fire";
        case FROST:
            return "frost";
        case SHADOW:
            return "shadow";
        case NON_ELEM:
            return "non-elemental";
        case HOLY:
            return "holy";
        case NATURE:
            return "nature";
        case MELEE:
            return "melee";
        case RESTORATION:
            return "restoration";
        }
    }

    switch (elem) {
    case FIRE:
        sprintf(str, "%s%s%s", BRED, "fire", reset);
        break;
    case FROST:
        sprintf(str, "%s%s%s", BBLU, "frost", reset);
        break;
    case SHADOW:
        sprintf(str, "%s%s%s", BMAG, "shadow", reset);
        break;
    case NON_ELEM:
        sprintf(str, "%s%s%s", BMAG, "non-elemental", reset);
        break;
    case HOLY:
        sprintf(str, "%s%s%s", BYEL, "holy", reset);
        break;
    case NATURE:
        sprintf(str, "%s%s%s", BGRN, "nature", reset);
        break;
    case MELEE:
        sprintf(str, "%s%s%s", BWHT, "melee", reset);
        break;
    case RESTORATION:
        sprintf(str, "%s%s%s", BGRN, "restoration", reset);
        break;
    }

    return str;
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
choose_inventory(hero_t * h,
                 item_t * new_item)
{
    // TODO: what is new_item is null because this menu
    //       was pulled in battle.
    size_t done = 0;
    int    selection = -1;

    rpg_tui_move_cursor(1, 1);
    rpg_tui_del_eof();

    rpg_tui_print_equip(h);
    print_selection(h, selection, new_item);
    print_inventory(h, selection, new_item);

    for (;;) {
        rpg_tui_print_inventory_prompt();

        char act_var = rpg_tui_safer_fgetc();

        if (act_var == '\033') {
            rpg_tui_safer_fgetc();
            act_var = rpg_tui_safer_fgetc();

            switch (act_var) {
            case 'A':
                selection--;
                if (selection < -1) { selection = -1; }
                break;

            case 'B':
                selection++;
                if (selection > MAX_INVENTORY) {
                    selection = MAX_INVENTORY;
                }
                break;

            default:
                break;
            }
        }
        else {
            switch (act_var) {
            case 'a':
                new_item = add_to_inventory(h, new_item);
                break;

            case 'd':
                if (selection < 0) {
                    if (new_item) {
                        memset(new_item, 0, sizeof(item_t));
                        new_item->slot = NO_ITEM;
                    }
                }
                else {
                    memset(&h->inventory[selection], 0, sizeof(item_t));
                    h->inventory[selection].slot = NO_ITEM;
                }

                break;

            case 'e':
                equip_from_inventory(h, selection, new_item);
                break;

            case 'u':
                use_from_inventory(h, selection, new_item);
                break;

            case 'q':
                done = 1;
                break;

            default:
                break;
            }
        }

        rpg_tui_move_cursor(1, 1);
        rpg_tui_del_eof();

        rpg_tui_print_equip(h);
        print_selection(h, selection, new_item);
        print_inventory(h, selection, new_item);

        //rpg_tui_clear_stdin();

        if (done) { break; }
    }

    return;
}


void
equip_from_inventory(hero_t *  h,
                     const int selection,
                     item_t *  new_item)
{
    // TODO: need to handle 2 hand being exclusive with one
    //       hand and offhand.
    if (selection >= 0) {
        if (h->inventory[selection].slot == NO_ITEM) {
            // Can't equip something from inventory
            // that's not there.
            return;
        }
    }

    // Get pointer to selected item, and swap equipped item
    // with selected item.
    item_t * s_i;
    slot_t   s_i_slot;

    if (selection < 0) {
        if (!new_item) {
            return;
        }

        s_i = new_item;
        s_i_slot = new_item->slot;
    }
    else {
        s_i = &h->inventory[selection];
        s_i_slot = h->inventory[selection].slot;
    }

    if (s_i_slot > MAX_ITEMS - 1) {
        // This isn't an equippable item (It's a potion or NO_ITEM).
        return;
    }

    if (s_i_slot == TWO_HAND) {
        // Equipping a two hand means de-equipping one handers.
        item_t * old_mh = &h->items[MAIN_HAND];
        item_t * old_oh = &h->items[OFF_HAND];

        old_mh = add_to_inventory(h, old_mh);
        old_oh = add_to_inventory(h, old_oh);

        if (old_mh || old_oh) {
            // Not enough space to dequip main and off hands.
            return;
        }

        h->items[MAIN_HAND].slot = NO_ITEM;
        h->items[OFF_HAND].slot = NO_ITEM;
    }
    else if (s_i_slot == MAIN_HAND || s_i_slot == OFF_HAND) {
        // Equipping one hand means de-equipping two hand.
        item_t * old_th = &h->items[TWO_HAND];

        old_th = add_to_inventory(h, old_th);

        if (old_th) {
            // Not enough space to dequip two hand.
            return;
        }

        h->items[TWO_HAND].slot = NO_ITEM;
    }

    item_t old_item = h->items[s_i_slot];
    size_t have_old_item = h->items[s_i_slot].slot = NO_ITEM;

    h->items[s_i_slot] = *s_i;

    *s_i = old_item;

    if (!have_old_item) {
        memset(s_i, 0, sizeof(item_t));
        s_i->slot = NO_ITEM;
    }

    return;
}


void
use_from_inventory(hero_t *  h,
                   const int selection,
                   item_t *  new_item)
{
    if (selection >= 0) {
        if (h->inventory[selection].slot == NO_ITEM) {
            // Can't use something from inventory
            // that's not there.
            return;
        }
    }

    // Get pointer to selected item.
    item_t * s_i;
    slot_t   s_i_slot;

    if (selection < 0) {
        if (!new_item) {
            return;
        }

        s_i = new_item;
        s_i_slot = new_item->slot;
    }
    else {
        s_i = &h->inventory[selection];
        s_i_slot = h->inventory[selection].slot;
    }

    if (s_i_slot != HP_POTION && s_i_slot != MP_POTION) {
        // This isn't a consumable.
        return;
    }

    size_t amnt = 0;
    switch (s_i_slot) {
    case HP_POTION:
        amnt = (size_t) floor(0.5 * ((float) get_max_hp(h)));

        if (restore_hp(h, amnt)) {
            memset(s_i, 0, sizeof(item_t));
            s_i->slot = NO_ITEM;
        }

        break;

    case MP_POTION:
        amnt = (size_t) floor(0.5 * ((float) get_max_mp(h)));

        if (restore_mp(h, amnt)) {
            memset(s_i, 0, sizeof(item_t));
            s_i->slot = NO_ITEM;
        }

        break;

    default:
        break;
    }

    return;
}


void
print_inventory(const hero_t * h,
                const int      selected,
                const item_t * new_item)
{
    char   pretty_name[MAX_NAME_LEN + 1];

    // Print the new item in hand.
    if (new_item && new_item->slot != NO_ITEM) {
        size_t row = NEW_ITEM_ROW;
        size_t col = NEW_ITEM_COL;
        size_t shift = 1;

        sprintf_item_name(pretty_name, new_item);

        printw("\033[%zu;%zuH New Item", row, col);

        if (-1 == selected) {
            printw("\033[%zu;%zuH * %s: %s", row + shift, col,
                   slot_to_str(new_item->slot), pretty_name);
        }
        else {
            printw("\033[%zu;%zuH   %s: %s", row + shift, col,
                   slot_to_str(new_item->slot), pretty_name);
        }

    }

    // Print the rest of the inventory.
    size_t row = INVENTORY_ROW;
    size_t col = INVENTORY_COL;
    size_t shift = 1;

    printw("\033[%zu;%zuH Inventory", row, col);

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        const item_t * item = &h->inventory[i];

        sprintf_item_name(pretty_name, item);

        if ((int) i == selected) {
            printw("\033[%zu;%zuH * %s: %s", row + shift, col,
                   slot_to_str(item->slot), pretty_name);
        }
        else {
            printw("\033[%zu;%zuH   %s: %s", row + shift, col,
                   slot_to_str(item->slot), pretty_name);
        }

        ++shift;
    }

    fflush(stdout);

    return;
}


void
print_selection(const hero_t * h,
                const int      selected,
                const item_t * new_item)
{
    size_t row = SELECT_ROW;
    size_t col = SELECT_COL;

    const item_t * item;

    if (selected < 0) {
        // New item in hand is selected.
        item = new_item;
    }
    else {
        // Old item in inventory is selected.
        item = &h->inventory[selected];
    }

    printw("\033[%zu;%zuH Item Select", row, col);

    if (!item || item->slot == NO_ITEM) {
        // Nothing to print.
        return;
    }

    char   pretty_name[MAX_NAME_LEN + 1];

    sprintf_item_name(pretty_name, item);

    printw("\033[%zu;%zuH %s: %s", row + 2, col,
           slot_to_str(item->slot), pretty_name);

    if (item->slot == HP_POTION || item->slot == MP_POTION) {
        // No other stats to print.
        return;
    }

    printw("\033[%zu;%zuH armor: %zu", row + 3, col, item->armor);
    printw("\033[%zu;%zuH sta:   %zu", row + 4, col, item->attr.sta);
    printw("\033[%zu;%zuH str:   %zu", row + 5, col, item->attr.str);
    printw("\033[%zu;%zuH agi:   %zu", row + 6, col, item->attr.agi);
    printw("\033[%zu;%zuH wis:   %zu", row + 7, col, item->attr.wis);
    printw("\033[%zu;%zuH spr:   %zu", row + 8, col, item->attr.spr);

    fflush(stdout);

    return;
}



void
sprintf_item_name(char *         name,
                  const item_t * item)
{
    switch (item->tier) {
    case GOOD:
        sprintf(name, "%s%s%s", BGRN, item->name, reset);
        break;

    case RARE:
        sprintf(name, "%s%s%s", BBLU, item->name, reset);
        break;

    case EPIC:
        sprintf(name, "%s%s%s", BMAG, item->name, reset);
        break;

    case COMMON:
    default:
        sprintf(name, "\e[1;0m%s\e[0m", item->name);
        break;
    }

    return;
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
    size_t fire_dmg = hero->spell(hero, enemy, FIRE, 1, 1);

    if (!fire_dmg) {
        // Spell cast failed for some reason.
        return 0;
    }
    // else, spell succeeded. Apply burn debuff.

    float burn_amnt = 0.1 * ((float) fire_dmg);

    apply_debuff(enemy, "burn", DOT, FIRE, burn_amnt, 1, 0);

    return fire_dmg;
}


size_t
shadow_bolt(hero_t * hero,
            hero_t * enemy)
{
    // Blast the target for X shadow damage, and heals the caster
    // for 10% of damage delt.
    size_t shadow_dmg = hero->spell(hero, enemy, SHADOW, 1, 1);

    if (!shadow_dmg) {
        // Spell cast failed for some reason.
        return 0;
    }
    // else, spell succeeded. Heal caster.

    float heal_amnt = 0.1 * ((float) shadow_dmg);

    restore_hp(hero, heal_amnt);

    return shadow_dmg;
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

    char elem_str[64];
    sprintf(msg_buf, "back stab hit %s for %zu %s damage\n",
            enemy->name, hp_reduced, elem_to_str(elem_str, MELEE));
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

    char elem_str[64];
    printf("crushing blow hit %s for %zu %s damage\n",
           enemy->name, hp_reduced, elem_to_str(elem_str, MELEE));

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
        printf("drain touch on cooldown for %zu rounds\n",
               hero->cooldowns[DRAIN_TOUCH].rounds);
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

        char elem_str[64];

        printf("drain touch hit %s for %zu %s hp damage\n",
               enemy->name, hp_reduced, elem_to_str(elem_str, SHADOW));

        total_dmg += hp_reduced;

        hp_reduced /= 2;

        if (hp_reduced == 0) { ++hp_reduced; }

        size_t n = restore_hp(hero, hp_reduced);
        printf("drain touch healed %s for %zu hp\n", hero->name, n);

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
        printf("shield bash on cooldown for %zu rounds\n",
               hero->cooldowns[SHIELD_BASH].rounds);
        return 0;
    }

    if (hero->items[OFF_HAND].slot == NO_ITEM ||
        hero->items[OFF_HAND].armor_type != SHIELD) {
        printf("no shield equipped\n");
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

    printf("shield bash hit %s for %zu hp damage\n",
           enemy->name, hp_reduced);

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
        printf("fireball on cooldown for %zu rounds\n",
               hero->cooldowns[FIREBALL].rounds);
        return 0;
    }

    size_t fire_dmg = hero->spell(hero, enemy, FIRE, 1.5, 1);

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
        printf("holy smite on cooldown for %zu rounds\n",
               hero->cooldowns[HOLY_SMITE].rounds);
        return 0;
    }

    size_t total_spirit = get_total_stat(hero, SPIRIT);
    size_t hp_reduced = attack_barrier(total_spirit, enemy);

    printf("smite hit %s for %zu hp damage\n",
           enemy->name, hp_reduced);

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
        printf("insect swarm on cooldown for %zu rounds\n",
               hero->cooldowns[HOLY_SMITE].rounds);
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
        printf("regen on cooldown for %zu rounds\n",
               hero->cooldowns[REGEN].rounds);
        return 0;
    }

    size_t heal_amnt = hero->heal(hero, 0.5, 0.5);

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
    char         elem_str[64];

    switch (db_status) {
    case DOT:
        // We don't have to consider spell resist or armor
        // here, because mitigation was already taken into
        // account when the debuff was calculated and applied.
        dmg = attack_barrier(amnt, enemy);
        printf("%s did %zu %s damage to %s\n", name, dmg,
               elem_to_str(elem_str, element), enemy->name);
        break;

    case STUN:
        printf("%s stunned %s\n", name, enemy->name);
        break;

    case HOT:
        dmg = restore_hp(enemy, (size_t) floor(amnt));
        printf("%s did %zu healing to %s\n", name, dmg, enemy->name);
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


