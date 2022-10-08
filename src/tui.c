#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "combat_stats.h"
#include "tui.h"

// Cursor row and column.
static size_t row_ = 0;
static size_t col_ = 0;

// Prompts.
static const char * spell_prompt = "\n"
                                   "  choose spell:\n"
                                   "    f: fire strike\n"
                                   "    i: ice\n"
                                   "    s: shadow bolt\n"
                                   "    u: non-elemental\n";


void
rpg_tui_print_portrait(hero_t *     h,
                       const size_t i,
                       const size_t j)
{
    // i: row
    // j: column
    stats_t stats = get_total_stats(h);
    printf("\033[%zu;%zuH ",                     i + 0, j);
    printf("\033[%zu;%zuH name:  %s    ",        i + 1, j, h->name);
    printf("\033[%zu;%zuH level: %zu, %s",   i + 2, j, h->level, mob_to_str(h->mob_type));
    printf("\033[%zu;%zuH hp:    %zu / %zu    ", i + 3, j, h->hp, get_max_hp(h));
    printf("\033[%zu;%zuH mp:    %zu / %zu    ", i + 4, j, h->mp, get_max_mp(h));
    printf("\033[%zu;%zuH sta:   %zu    ",       i + 5, j, stats.sta);
    printf("\033[%zu;%zuH str:   %zu    ",       i + 6, j, stats.str);
    printf("\033[%zu;%zuH agi:   %zu    ",       i + 7, j, stats.agi);
    printf("\033[%zu;%zuH wis:   %zu    ",       i + 8, j, stats.wis);
    printf("\033[%zu;%zuH spr:   %zu    ",       i + 9, j, stats.spr);
    printf("\033[%zu;%zuH armor: %zu    ",       i + 10, j, get_armor(h));

    rpg_tui_set_cursor();

    return;
}


void
rpg_tui_move_cursor(const size_t i,
                    const size_t j)
{
    // Move to i'th row, j'th column.
    printf("\033[%zu;%zuH ", i, j);

    return;
}

void
rpg_tui_reset_cursor(void)
{
    // Reset to battle text starting position.
    printf("\033[%d;%dH ", BATTLE_TXT_ROW, BATTLE_TXT_ROW);
    return;
}

void
rpg_tui_set_cursor(void)
{
    // Set cursor to current offset from battle text starting position.
    printf("\033[%zu;%zuH\r", BATTLE_TXT_ROW + row_, BATTLE_TXT_ROW + col_);
    return;
}

void
rpg_tui_del_line(void)
{
    printf("\33[2K");
    return;
}

void
rpg_tui_del_eof(void)
{
    printf("\r\033[J");
    return;
}

void
rpg_tui_clear_screen(void)
{
    rpg_tui_move_cursor(1, 1);
    rpg_tui_del_eof();
    return;
}

char
rpg_tui_safer_fgetc(void)
{
    int n = fgetc(stdin);

    if (n > 0 && n < 255) {
        return (char) n;
    }

    clearerr(stdin);
    return '\0';
}


void
rpg_tui_clear_stdin(void)
{
    size_t done = 0;

    for (;;) {
        char n = rpg_tui_safer_fgetc();

        switch (n) {
        case '\0':
        case '\n':
            done = 1;
            break;

        default:
            break;
        }

        if (done) { break; }
    }

    return;
}

void
rpg_tui_print_act_prompt(const hero_t * h)
{
    printf("\n");
    printf("  choose melee attack:\n");
    printf("    a: attack\n");
    printf("    s: spell\n");
    printf("    d: defend\n");
    printf("    h: heal\n");

    if (h->cooldowns[USE_ITEM].unlocked)
        printf("    u: use item\n");

    return;
}


void
rpg_tui_clear_act_prompt(const hero_t * h)
{
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");

    if (h->cooldowns[USE_ITEM].unlocked)
        printf("\r\033[A");

    // Not sure why this one extra needed.
    printf("\r\033[A");
    rpg_tui_del_eof();

    return;
}


void
rpg_tui_print_spell_prompt(const hero_t * h)
{
    printf("%s", spell_prompt);

    if (h->cooldowns[FIREBALL].unlocked) {
        printf("    b: fireball\n");
    }

    if (h->cooldowns[HOLY_SMITE].unlocked) {
        printf("    h: holy smite\n");
    }

    if (h->cooldowns[INSECT_SWARM].unlocked) {
        printf("    n: insect swarm\n");
    }

    return;
}


void
rpg_tui_print_heal_prompt(const hero_t * h)
{
    printf("\n");
    printf("  choose heal spell:\n");
    printf("    h: Heal I\n");

    if (h->cooldowns[REGEN].unlocked) {
        printf("    r: regen\n");
    }

    return;
}

void
rpg_tui_clear_heal_prompt(const hero_t * h)
{
    printf("\r\033[A");
    printf("\r\033[A");

    if (h->cooldowns[REGEN].unlocked) {
        printf("\r\033[A");
    }

    printf("\r\033[A");

    return;
}


void
rpg_tui_print_attack_prompt(const hero_t * h)
{
    printf("\n");
    printf("  choose melee attack:\n");
    printf("    a: weapon attack\n");

    if (h->cooldowns[BACK_STAB].unlocked) {
        printf("    b: back stab\n");
    }

    if (h->cooldowns[CRUSHING_BLOW].unlocked) {
        printf("    c: crushing blow\n");
    }

    if (h->cooldowns[DRAIN_TOUCH].unlocked) {
        printf("    d: drain touch\n");
    }

    if (h->cooldowns[SHIELD_BASH].unlocked) {
        printf("    s: shield bash\n");
    }

    return;
}


void
rpg_tui_clear_spell_prompt(const hero_t * h)
{
    const char * ptr = spell_prompt;

    while (*ptr) {
        if (*ptr == '\n') {
            printf("\r\033[A");
        }

        ++ptr;
    }

    if (h->cooldowns[FIREBALL].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[HOLY_SMITE].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[INSECT_SWARM].unlocked) {
        printf("\r\033[A");
    }

    // Not sure why this one extra needed.
    printf("\r\033[A");
    rpg_tui_del_eof();

    return;
}


void
rpg_tui_clear_attack_prompt(const hero_t * h)
{
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");

    // Just iterate through list? Make it smarter?
    if (h->cooldowns[BACK_STAB].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[CRUSHING_BLOW].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[DRAIN_TOUCH].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[SHIELD_BASH].unlocked) {
        printf("\r\033[A");
    }

    // Not sure why this one extra needed.
    printf("\r\033[A");
    rpg_tui_del_eof();

    return;
}


void
rpg_tui_increment_row(void)
{
    ++row_;
    return;
}


void
rpg_tui_check_row(void)
{
    if (row_ >= MAX_BATTLE_TXT_LINES) {
        row_ = 0;
    }

    return;
}


void
rpg_tui_reset_row(void)
{
    row_ = 0;
}

void
rpg_tui_print_inventory_prompt(void)
{
    size_t row = INV_PROMPT_ROW;
    size_t col = INV_PROMPT_COL;

    printf("\033[%zu;%zuH Actions:", row, col);

    printf("\033[%zu;%zuH a: add to inventory", row + 3, col);
    printf("\033[%zu;%zuH d: throw away selected item", row + 4, col);
    printf("\033[%zu;%zuH e: equip selected item", row + 5, col);
    printf("\033[%zu;%zuH u: use selected item", row + 6, col);
    printf("\033[%zu;%zuH q: quit", row + 7, col);

    fflush(stdout);

    return;
}

static void
print_fld(const char * what,
          const size_t amnt)
{
    if (amnt) {
        printf("%s %zu\n", what, amnt);
    }

    return;
}

void
rpg_tui_print_hero(hero_t *     h,
                   const size_t verbosity)
{
    printf("name: %s\n", h->name);
    printf("level: %zu\n", h->level);
    printf("hp:    %zu / %zu\n", h->hp, get_max_hp(h));
    printf("mp:    %zu / %zu\n", h->mp, get_max_mp(h));
    printf("xp:    %zu\n", h->xp);

    if (verbosity <= 1) {
        printf("\n");
        return;
    }

    printf("\n");
    printf("base attributes\n");
    printf("  sta: %zu\n", h->base.sta);
    printf("  str: %zu\n", h->base.str);
    printf("  agi: %zu\n", h->base.agi);
    printf("  wis: %zu\n", h->base.wis);
    printf("  spr: %zu\n", h->base.spr);

    if (verbosity == 2) {
        printf("\n");
        return;
    }

    printf("\n");
    printf("spell power\n");
    printf("  fire:   %zu\n", h->power.fire);
    printf("  frost:  %zu\n", h->power.frost);
    printf("  shadow: %zu\n", h->power.shadow);
    printf("\n");
    printf("spell resist\n");
    printf("  fire:   %zu\n", h->resist.fire);
    printf("  frost:  %zu\n", h->resist.frost);
    printf("  shadow: %zu\n", h->resist.shadow);
    printf("\n");

    if (verbosity == 3) {
        printf("\n");
        return;
    }

    printf("equipment\n");

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            continue;
        }

        printf("  %s: %s\n", slot_to_str(i), h->items[i].name);
        print_fld("  armor:", h->items[i].armor);
        print_fld("    sta:", h->items[i].attr.sta);
        print_fld("    str:", h->items[i].attr.str);
        print_fld("    agi:", h->items[i].attr.agi);
        print_fld("    wis:", h->items[i].attr.wis);
        print_fld("    spr:", h->items[i].attr.spr);

        printf("\n");
        printf("spell power\n");
        printf("  fire:   %zu\n", h->items[i].power.fire);
        printf("  frost:  %zu\n", h->items[i].power.frost);
        printf("  shadow: %zu\n", h->items[i].power.shadow);
        printf("\n");
        printf("spell resist\n");
        printf("  fire:   %zu\n", h->items[i].resist.fire);
        printf("  frost:  %zu\n", h->items[i].resist.frost);
        printf("  shadow: %zu\n", h->items[i].resist.shadow);
        printf("\n");
    }

    printf("\n");


    return;
}


void
rpg_tui_print_equip(hero_t * h)
{
    printf("name:  %s\n", h->name);
    printf("level: %zu\n", h->level);
    printf("hp:    %zu / %zu\n", h->hp, get_max_hp(h));
    printf("mp:    %zu / %zu\n", h->mp, get_max_mp(h));
    printf("\n");
    printf("sta:   %zu\n", h->base.sta);
    printf("str:   %zu\n", h->base.str);
    printf("agi:   %zu\n", h->base.agi);
    printf("wis:   %zu\n", h->base.wis);
    printf("spr:   %zu\n", h->base.spr);
    printf("\n");
    printf("armor: %zu (%.2f%% melee dmg reduction)\n", get_armor(h),
           100 - (100 * get_mitigation(h)));
    printf("dodge:      %.2f%%\n", 0.01 * get_dodge(h));
    printf("\n");
    printf("attack dmg:\n");
    printf(" main hand: %.2f\n", get_melee_dmg(h, &h->items[MAIN_HAND], NO_SMEAR));
    printf("  off hand: %.2f\n", get_melee_dmg(h, &h->items[OFF_HAND], NO_SMEAR));
    printf("  two hand: %.2f\n", get_melee_dmg(h, &h->items[TWO_HAND], NO_SMEAR));
    printf("spell dmg:\n");
    printf("  fire:     %.2f\n", get_spell_dmg(h, FIRE, NO_SMEAR));
    printf("  frost:    %.2f\n", get_spell_dmg(h, FROST, NO_SMEAR));
    printf("  shadow:   %.2f\n", get_spell_dmg(h, SHADOW, NO_SMEAR));
    printf("  non-elem: %.2f\n", get_spell_dmg(h, NON_ELEM, NO_SMEAR));

    printf("\n");
    printf("equipment\n");

    char pretty_name[MAX_NAME_LEN + 1];

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            printf("  %s:        \n", slot_to_str(i));

            continue;
        }

        sprintf_item_name(pretty_name, &h->items[i]);
        printf("  %s: \e[1;32m%s\e[0m (%s)\n", slot_to_str(i), pretty_name,
               armor_to_str(h->items[i].armor_type));
    }

    fflush(stdout);

    return;
}


