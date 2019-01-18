#include <stdio.h>
#include <string.h>

#include "rpg.h"
#include "tui.h"

// Cursor row and column.
static size_t row_ = 0;
static size_t col_ = 0;




void
print_portrait(hero_t *     h,
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

    set_cursor();

    return;
}



void
move_cursor(const size_t i,
            const size_t j)
{
    // Move to i'th row, j'th column.
    printf("\033[%zu;%zuH ", i, j);

    return;
}



void
reset_cursor(void)
{
    // Reset to battle text starting position.
    printf("\033[%d;%dH ", BATTLE_TXT_ROW, BATTLE_TXT_ROW);

    return;
}




void
set_cursor(void)
{
    // Set cursor to current offset from battle text starting position.
    printf("\033[%zu;%zuH\r", BATTLE_TXT_ROW + row_, BATTLE_TXT_ROW + col_);

    return;
}



void
del_line(void)
{
    printf("\33[2K");
    return;
}



void
del_eof(void)
{
    printf("\r\033[J");
    return;
}



void
clear_screen(void)
{
    move_cursor(1, 1);
    del_eof();
    return;
}



void
print_act_prompt(void)
{
    printf("%s", action_prompt);

    return;
}



void
clear_act_prompt(void)
{
    const char * ptr = action_prompt;

    while (*ptr) {
        if (*ptr == '\n') {
            printf("\r\033[A");
        }

        ++ptr;
    }

    // Not sure why this one extra needed.
    printf("\r\033[A");
    del_eof();

    return;
}



void
print_spell_prompt(const hero_t * h)
{
    printf("%s", spell_prompt);

    if (h->cooldowns[FIREBALL].unlocked) {
        printf("    b: fireball\n");
    }

    return;
}



void
print_heal_prompt(const hero_t * h)
{
    printf("\n");
    printf("  choose heal spell:\n");

    if (h->cooldowns[DIVINE_HEAL].unlocked) {
        printf("    d: divine heal\n");
    }

    return;
}



void
clear_heal_prompt(const hero_t * h)
{
    printf("\r\033[A");

    if (h->cooldowns[DIVINE_HEAL].unlocked) {
        printf("\r\033[A");
    }

    printf("\r\033[A");

    return;
}



void
print_attack_prompt(const hero_t * h)
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

    if (h->cooldowns[SHIELD_BASH].unlocked) {
        printf("    s: shield bash\n");
    }

    return;
}



void
clear_spell_prompt(const hero_t * h)
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

    // Not sure why this one extra needed.
    printf("\r\033[A");
    del_eof();

    return;
}



void
clear_attack_prompt(const hero_t * h)
{
    printf("\r\033[A");
    printf("\r\033[A");
    printf("\r\033[A");

    if (h->cooldowns[BACK_STAB].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[CRUSHING_BLOW].unlocked) {
        printf("\r\033[A");
    }

    if (h->cooldowns[SHIELD_BASH].unlocked) {
        printf("\r\033[A");
    }

    // Not sure why this one extra needed.
    printf("\r\033[A");
    del_eof();

    return;
}



void
increment_row(void)
{
    ++row_;
    return;
}



void
check_row(void)
{
    if (row_ >= MAX_BATTLE_TXT_LINES) {
        row_ = 0;
    }

    return;
}



void
reset_row(void)
{
    row_ = 0;
}
