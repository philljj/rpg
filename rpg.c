#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PROCS 14
#define MAX_ITEMS 10
#define MAX_NAME_LEN  32
#define ARMOR_HALF_POINT 100

void
print_fld(const char * what,
          const size_t amnt)
{
    if (amnt) {
        printf("%s %zu\n", what, amnt);
    }

    return;
}

typedef enum {
    MAIN_HAND = 0,
    OFF_HAND  = 1,
    TWO_HAND  = 2,
    HELM      = 3,
    SHOULDERS = 4,
    CHEST     = 5,
    PANTS     = 6,
    GLOVES    = 7,
    BOOTS     = 8,
    RING      = 9,
    TRINKET   = 10
} slot_t;

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

    case HELM:
        return "helm";

    case SHOULDERS:
        return "shoulders";

    case CHEST:
        return "chest";

    case PANTS:
        return "pants";

    case GLOVES:
        return "gloves";

    case BOOTS:
        return "boots";

    case RING:
        return "ring";

    case TRINKET:
        return "trinket";

    default:
        return "NA";
    }
}

typedef struct {
    size_t sta;
    size_t str;
    size_t agi;
    size_t wis;
    size_t spr;
} attr_t;

typedef struct {
    size_t fire;
    size_t frost;
    size_t shadow;
} spell_t;

typedef enum {
    NONE     = 0,
    DRAIN_HP = 1,
    DRAIN_MP = 2,
    MP_BURN  = 3,
    THORNS   = 4,
    BARRIER  = 5,
} proc_type_t;

typedef struct {
    proc_type_t type;  // proc_type_t
    size_t      rate;  // Chance of triggering. 0 - 100
    size_t      coeff; // Multipier. 0 - 100
} proc_t;

typedef struct {
    char    name[MAX_NAME_LEN + 1];
    attr_t  attr;
    spell_t power;
    spell_t resist;
    proc_t  procs[MAX_PROCS];
    slot_t  slot;
    size_t  is_weapon;
} item_t;

typedef struct {
    char    name[MAX_NAME_LEN + 1]; // There are Some who call me Tim.
    size_t  hp;                     // Health Points.
    size_t  mp;                     // Mana Points.
    size_t  bp;                     // Barrier Points.
    size_t  armor;                  // Mitigates physical damage.
    size_t  level;
    size_t  xp;
    attr_t  base;                   // Base attributes. Increases with level.
    spell_t power;                  // Enhances spell damage given.
    spell_t resist;                 // Resists spell damage received.
    item_t  items[MAX_ITEMS];
    size_t  have_item[MAX_ITEMS];
} hero_t;

void level_up(hero_t * h);
void set_hp_mp(hero_t * h);

item_t create_item(const char * name, const size_t level, const slot_t slot);
size_t equip_item(hero_t * hero, const item_t * item);
size_t attack_damage(const hero_t * h);
size_t spell_damage(hero_t * h);

hero_t roll_hero(void);
void print_hero(hero_t * h);

void sum_procs(size_t * h_proc_sum, hero_t * h);
size_t attack_barrier(size_t final_dmg, hero_t * enemy);



int
main(int    argc,
     char * argv[])
{
    hero_t h = roll_hero();
    print_hero(&h);

    return EXIT_SUCCESS;
}



hero_t
roll_hero(void)
{
    hero_t h;
    pid_t  pid;

    pid = getpid();
    srand(pid);

    memset(&h, 0, sizeof(h));

    h.level = 1;

    h.base.sta = 6 + (rand() % 12);
    h.base.str = 6 + (rand() % 12);
    h.base.agi = 6 + (rand() % 12);
    h.base.wis = 6 + (rand() % 12);
    h.base.spr = 6 + (rand() % 12);

    sprintf(h.name, "Tim the Enchanter");

    h.items[CHEST] = create_item("plain shirt", h.level, CHEST);
    h.have_item[CHEST] = 1;

    h.items[PANTS] = create_item("worn pants", h.level, PANTS);
    h.have_item[PANTS] = 1;

    set_hp_mp(&h);

    return h;
}



void
print_hero(hero_t * h)
{
    printf("name: %s\n", h->name);
    printf("\n");
    printf("level: %zu\n", h->level);
    printf("hp:    %zu\n", h->hp);
    printf("mp:    %zu\n", h->mp);
    printf("xp:    %zu\n", h->xp);
    printf("\n");
    printf("base attributes\n");
    printf("  sta: %zu\n", h->base.sta);
    printf("  str: %zu\n", h->base.str);
    printf("  agi: %zu\n", h->base.agi);
    printf("  wis: %zu\n", h->base.wis);
    printf("  spr: %zu\n", h->base.spr);
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
    printf("equipment\n");

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            continue;
        }

        printf("  %s: %s\n", slot_to_str(i), h->items[i].name);
        print_fld("    sta:", h->items[i].attr.sta);
        print_fld("    str:", h->items[i].attr.str);
        print_fld("    agi:", h->items[i].attr.agi);
        print_fld("    wis:", h->items[i].attr.wis);
        print_fld("    spr:", h->items[i].attr.spr);

    }

    printf("\n");


    return;
}



item_t
create_item(const char * name,
            const size_t level,
            const slot_t slot)
{
    // This doesn't set is_weapon.
    item_t item;
    size_t attr_lvl = 0;
    size_t resist_lvl = 5;
    size_t power_lvl = 10;
    size_t procs_lvl = 20;
    size_t shift_lvl = level + 1;

    memset(&item, 0, sizeof(item));

    strcpy(item.name, name);

    item.slot = slot;

    if (level > attr_lvl) {
        item.attr.sta = rand() % (shift_lvl - attr_lvl);
        item.attr.str = rand() % (shift_lvl - attr_lvl);
        item.attr.agi = rand() % (shift_lvl - attr_lvl);
        item.attr.wis = rand() % (shift_lvl - attr_lvl);
        item.attr.spr = rand() % (shift_lvl - attr_lvl);
    }

    if (level > resist_lvl) {
        item.resist.fire = rand() % (shift_lvl - resist_lvl);
        item.resist.frost = rand() % (shift_lvl - resist_lvl);
        item.resist.shadow = rand() % (shift_lvl - resist_lvl);
    }

    if (level > power_lvl) {
        item.power.fire = rand() % (shift_lvl - power_lvl);
        item.power.frost = rand() % (shift_lvl - power_lvl);
        item.power.shadow = rand() % (shift_lvl - power_lvl);
    }

    if (level > procs_lvl) {
        // Do nothing for now...
    }

    return item;
}



size_t
equip_item(hero_t *       hero,
           const item_t * item)
{
    slot_t slot = item->slot;

    if (hero->have_item[slot]) {
        return 0;
    }

    hero->items[slot] = *item;
    hero->have_item[slot] = 1;

    return 1;
}



size_t
attack_damage(const hero_t * h)
{
    // Attack damage is
    //
    //   dmg = (total strength) * (weapon multiplier)
    //
    // where the weapon multiplier is given by
    //
    //   2 handed = 2.0
    //   1 handed = 0.8 (per hand)
    //   unarmed  = 0.5 (per hand)

    size_t dmg = 0;
    float  mult = 0;
    size_t str = 0;

    str += h->base.str;

    size_t mh = h->have_item[MAIN_HAND];
    size_t oh = h->have_item[OFF_HAND];
    size_t th = h->have_item[TWO_HAND];

    if (!th) {
        // Barehanded damage mult.
        if (!mh) { mult += 0.5; }
        if (!oh) { mult += 0.5; }
    }

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to calculate.
            continue;
        }

        str += h->items[i].attr.str;

        if (!h->items[i].is_weapon) {
            // No weapon multiplier. This could be
            // a shield or relic.
            continue;
        }

        switch (h->items[i].slot) {
        case MAIN_HAND:
            mult += 0.8;
        break;

        case OFF_HAND:
            mult += 0.8;
        break;

        case TWO_HAND:
            mult += 2.0;
        break;

        default:
        break;
        }
    }

    dmg = str * mult;

    return dmg;
}



size_t
spell_damage(hero_t * h)
{
    // Spell damage (and spell healing) is
    //
    //   dmg = (total wisdom) * (spell multiplier)
    //
    // where the spell multiplier is 3.

    size_t dmg = 0;
    size_t wis = 0;
    size_t mult = 3;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to calculate.
            continue;
        }

        wis += h->items[i].attr.wis;
    }

    dmg = wis * mult;

    return dmg;
}



void
attack_enemy(hero_t * hero,
             hero_t * enemy)
{
    float  armor = enemy->armor;
    float  base_dmg;
    float  mitigation;
    size_t final_dmg;
    size_t h_proc_sum[MAX_PROCS];
    size_t e_proc_sum[MAX_PROCS];

    memset(h_proc_sum, 0, sizeof(h_proc_sum));
    memset(e_proc_sum, 0, sizeof(e_proc_sum));

    sum_procs(h_proc_sum, hero);
    sum_procs(e_proc_sum, enemy);

    base_dmg = attack_damage(hero);
    mitigation = armor / (armor + ARMOR_HALF_POINT);
    final_dmg = (size_t) floor(base_dmg * mitigation);

    if (!final_dmg) {
        // Always at least 1 hp of damage.
        ++final_dmg;
    }

    // Reduce enemy barrier and health.
    size_t hp_reduced = attack_barrier(final_dmg, enemy);

    if (h_proc_sum[DRAIN_HP] && hp_reduced) {
        // Needs a multiplier...
        hero->hp += hp_reduced; 
    }

    if (h_proc_sum[DRAIN_MP] && hp_reduced) {
        // Needs a multiplier...
        hero->mp += hp_reduced < enemy->mp ? hp_reduced : enemy->mp;
        enemy->mp -= hp_reduced < enemy->mp ? hp_reduced : enemy->mp;
    }

    if (h_proc_sum[MP_BURN] && hp_reduced) {
        // Needs a multiplier...
        enemy->mp -= hp_reduced < enemy->mp ? hp_reduced : enemy->mp;
    }

    if (e_proc_sum[THORNS]) {
        // Reflect damage at hero.
        size_t thorn_dmg = e_proc_sum[THORNS];
        attack_barrier(thorn_dmg, hero);
    }

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

    h->xp = 0;

    return;
}



void
set_hp_mp(hero_t * h)
{
    // hp =  5 * stamina
    // mp = 10 * wisdom
    size_t stamina = h->base.sta;
    size_t wisdom = h->base.wis;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        stamina += h->items[i].attr.sta;
        wisdom += h->items[i].attr.wis;
    }

    h->hp =  5 * stamina;
    h->mp = 10 * wisdom;

    return;
}



void
none(hero_t * h,
     hero_t * enemy)
{
    return;
}



void
sum_procs(size_t * h_proc_sum,
          hero_t * h)
{
    // Sum up all the procs by type, across
    // the full inventory of items equipped.
    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        for (size_t p = 0; p < MAX_PROCS; ++p) {
            proc_type_t p_type = h->items[i].procs[p].type;

            if (!p_type) {
                // Nothing to do.
                continue;
            }

            size_t rate = h->items[i].procs[p].rate;
            size_t trigger = rand() % 100;

            if (rate > trigger) {
                // Proc trigger threshold reached.
                size_t  coeff = h->items[i].procs[p].coeff;
                h_proc_sum[p_type] += coeff;
            }
        }
    }

    h->bp += h_proc_sum[BARRIER];

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
        // Reduce the barrier first.
        size_t dmg = final_dmg < bp ? final_dmg : bp;
        final_dmg -= dmg;
        enemy->bp -= dmg;
    }

    if (final_dmg) {
        // Then the health pool with remaining dmg.
        hp_reduced = final_dmg < hp ? final_dmg : enemy->hp;
        enemy->hp -= hp_reduced;
    }

    return hp_reduced;
}
