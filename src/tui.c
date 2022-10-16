#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "item_stats.h"
#include "spell_type.h"
#include "rpg.h"
#include "combat_stats.h"
#include "tui.h"

#include <curses.h>

#define COMBAT_TXT_ROW       (16)
#define COMBAT_TXT_COL       (4)

// Cursor row and column.
static size_t row_ = 0;
static size_t col_ = 0;

void
rpg_tui_init(void)
{
    initscr();
    noecho();
    cbreak();
    timeout(-1);
    start_color();
    //rpg_tui_clear_screen(); maybe not needed
    return;
}

void
rpg_tui_print_portrait(hero_t *     h,
                       const size_t i,
                       const size_t j)
{
    // i: row
    // j: column
    stats_t stats = get_total_stats(h);
    mvprintw(i+ 0, j, "");
    mvprintw(i+ 1, j, "name:  %s    ",        h->name);
    mvprintw(i+ 2, j, "level: %zu, %s",       h->level, mob_to_str(h->mob_type));
    mvprintw(i+ 3, j, "hp:    %zu / %zu    ", h->hp, get_max_hp(h));
    mvprintw(i+ 4, j, "mp:    %zu / %zu    ", h->mp, get_max_mp(h));
    mvprintw(i+ 5, j, "sta:   %zu    ",       stats.sta);
    mvprintw(i+ 6, j, "str:   %zu    ",       stats.str);
    mvprintw(i+ 7, j, "agi:   %zu    ",       stats.agi);
    mvprintw(i+ 8, j, "wis:   %zu    ",       stats.wis);
    mvprintw(i+ 9, j, "spr:   %zu    ",       stats.spr);
    mvprintw(i+10, j, "armor: %zu    ",       get_armor(h));

    return;
}


void
rpg_tui_move_cursor(const size_t i,
                    const size_t j)
{
    // Move to i'th row, j'th column.
    move(i, j);

    return;
}

void
rpg_tui_del_eof(void)
{
    erase();
    return;
}

void
rpg_tui_clear_screen(void)
{
    rpg_tui_move_cursor(0, 0);
    rpg_tui_del_eof();
    return;
}

char
rpg_tui_getch(void)
{
    int n = getch();

    if (n > 0 && n < 255)
        return (char) n;
    else
        return '\0';
}

void
rpg_tui_print_act_prompt(const hero_t * h,
                         size_t         i,
                         size_t         j)
{
    mvprintw(i + 0, j, "\n");
    mvprintw(i + 1, j, "  choose action:\n");
    mvprintw(i + 2, j, "    a: attack\n");
    mvprintw(i + 3, j, "    s: job primary\n");
    mvprintw(i + 4, j, "    d: job secondary\n");

    if (h->cooldowns[USE_ITEM].unlocked)
        mvprintw(i + 6, j, "    u: use item\n");

    return;
}


void
rpg_tui_clear_act_prompt(const hero_t * h,
                         size_t         i,
                         size_t         j)
{
    move(i++, j); clrtoeol();
    move(i++, j); clrtoeol();
    move(i++, j); clrtoeol();
    move(i++, j); clrtoeol();
    move(i++, j); clrtoeol();
    move(i++, j); clrtoeol();

    if (h->cooldowns[USE_ITEM].unlocked)
        move(i++, j); clrtoeol();

    // Not sure why this one extra needed.
    move(i++, j); clrtoeol();

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
rpg_tui_print_combat_txt(const char * msg)
{
    mvprintw(COMBAT_TXT_ROW + row_, COMBAT_TXT_COL, "%s", msg);
    rpg_tui_increment_row();
    rpg_tui_check_row();
    return;
}

void
rpg_tui_print_color_txt(const element_t elem)
{
    char name[64];
    switch (elem) {
    case FIRE:
        init_pair(1, COLOR_RED, COLOR_BLACK);
        sprintf(name, "fire");
        break;
    case FROST:
        init_pair(1, COLOR_BLUE, COLOR_BLACK);
        sprintf(name, "frost");
        break;
    case SHADOW:
        init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
        sprintf(name, "shadow");
        break;
    case NON_ELEM:
        init_pair(1, COLOR_MAGENTA, COLOR_BLACK);
        sprintf(name, "fire");
        break;
    case HOLY:
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        sprintf(name, "fire");
        break;
    case NATURE:
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        sprintf(name, "nature");
        break;
    case MELEE:
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        sprintf(name, "fire");
        break;
    case RESTORATION:
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        sprintf(name, "fire");
        break;
    }

    attron(COLOR_PAIR(1));
    mvprintw(COMBAT_TXT_ROW + row_, COMBAT_TXT_COL + col_, "%s", name);
    attroff(COLOR_PAIR(1));
    col_ += strlen(name);
    return;
}

void
rpg_tui_print_combat_color_txt(const char *    prefix,
                               const element_t elem,
                               const char *    postfix)
{
    // <spell> did <zu> <color element> damage to <target>
    // <msg prefix> <color element> <msg postfix>
    col_ = 0;
    mvprintw(COMBAT_TXT_ROW + row_, COMBAT_TXT_COL, "%s", prefix);
    col_ += strlen(prefix);
    rpg_tui_print_color_txt(elem);
    mvprintw(COMBAT_TXT_ROW + row_, COMBAT_TXT_COL + col_, "%s", postfix);
    col_ += strlen(postfix);
    rpg_tui_increment_row();
    rpg_tui_check_row();
    return;
}

static void
print_fld(const char * what,
          const size_t amnt)
{
    if (amnt) {
        printw("%s %zu\n", what, amnt);
    }

    return;
}

void
rpg_tui_print_hero(hero_t *     h,
                   const size_t verbosity)
{
    printw("name: %s\n", h->name);
    printw("level: %zu\n", h->level);
    printw("hp:    %zu / %zu\n", h->hp, get_max_hp(h));
    printw("mp:    %zu / %zu\n", h->mp, get_max_mp(h));
    printw("xp:    %zu\n", h->xp);

    if (verbosity <= 1) {
        printw("\n");
        return;
    }

    printw("\n");
    printw("base attributes\n");
    printw("  sta: %zu\n", h->base.sta);
    printw("  str: %zu\n", h->base.str);
    printw("  agi: %zu\n", h->base.agi);
    printw("  wis: %zu\n", h->base.wis);
    printw("  spr: %zu\n", h->base.spr);

    if (verbosity == 2) {
        printw("\n");
        return;
    }

    printw("\n");
    printw("spell power\n");
    printw("  fire:   %zu\n", h->power.fire);
    printw("  frost:  %zu\n", h->power.frost);
    printw("  shadow: %zu\n", h->power.shadow);
    printw("\n");
    printw("spell resist\n");
    printw("  fire:   %zu\n", h->resist.fire);
    printw("  frost:  %zu\n", h->resist.frost);
    printw("  shadow: %zu\n", h->resist.shadow);
    printw("\n");

    if (verbosity == 3) {
        printw("\n");
        return;
    }

    printw("equipment\n");

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (h->items[i].slot == NO_ITEM) {
            continue;
        }

        printw("  %s: %s\n", slot_to_str(i), h->items[i].name);
        print_fld("  armor:", h->items[i].armor);
        print_fld("    sta:", h->items[i].attr.sta);
        print_fld("    str:", h->items[i].attr.str);
        print_fld("    agi:", h->items[i].attr.agi);
        print_fld("    wis:", h->items[i].attr.wis);
        print_fld("    spr:", h->items[i].attr.spr);

        printw("\n");
        printw("spell power\n");
        printw("  fire:   %zu\n", h->items[i].power.fire);
        printw("  frost:  %zu\n", h->items[i].power.frost);
        printw("  shadow: %zu\n", h->items[i].power.shadow);
        printw("\n");
        printw("spell resist\n");
        printw("  fire:   %zu\n", h->items[i].resist.fire);
        printw("  frost:  %zu\n", h->items[i].resist.frost);
        printw("  shadow: %zu\n", h->items[i].resist.shadow);
        printw("\n");
    }

    printw("\n");


    return;
}
