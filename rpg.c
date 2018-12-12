#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_PROCS 14
#define MAX_ITEMS 10
#define MAX_NAME_LEN  32
#define ARMOR_HALF_POINT 100
#define DODGE_HALF_POINT 100

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

typedef struct {
    size_t sta;
    size_t str;
    size_t agi;
    size_t wis;
    size_t spr;
} stats_t;

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
    stats_t attr;
    spell_t power;
    spell_t resist;
    proc_t  procs[MAX_PROCS];
    slot_t  slot;
    size_t  is_weapon;
    //size_t  armor; // TODO: armor?
} item_t;

// TODO: different mob struct types? hero, dragon, undead, generic mob?
//       this might be more complexity than worth it. would have to
//       handle casting structs.

// TODO:
//       attack
//       spell
//       heal or defense

// TODO: need way to track temporary buffs/debuffs.

struct hero_t {
    // Common fields.
    char    name[MAX_NAME_LEN + 1]; // There are Some who call me Tim.
    size_t  level;
    size_t  hp;                     // Health Points.
    size_t  mp;                     // Mana Points.
    size_t  bp;                     // Barrier Points.
    size_t  armor;                  // Mitigates physical damage.
    stats_t base;                   // Base attributes. Increases with level.
    spell_t power;                  // Enhances spell damage given.
    spell_t resist;                 // Resists spell damage received.
    // Need way to handle cooldowns for these...
    void    (*attack)(struct hero_t *, struct hero_t *);
    size_t  (*spell)(struct hero_t *, struct hero_t *);
    void    (*defend)(struct hero_t *);
    size_t  (*heal)(struct hero_t *);
    // hero specific fields.
    size_t  xp;
    item_t  items[MAX_ITEMS];
    size_t  have_item[MAX_ITEMS];
};

typedef struct hero_t hero_t;

typedef enum {
    HERO,
    HUMANOID,
    ANIMAL,
    UNDEAD,
    DRAGON
} mob_t;

// Basic mob gen functions.
hero_t roll_mob(const char * name, const size_t lvl, const mob_t mob);
hero_t roll_hero(const char * name, const size_t lvl);
hero_t roll_dragon(const char * name, const size_t lvl);
item_t create_item(const char * name, const size_t level, const slot_t slot);
void   level_up(hero_t * h);
void   set_hp_mp(hero_t * h);
size_t get_max_hp(hero_t * h);
size_t get_max_mp(hero_t * h);
// Print functions.
void         print_hero(hero_t * h, const size_t concise);
void         print_fld(const char * what, const size_t amnt);
const char * slot_to_str(slot_t s);
// Combat functions.
void   decision_loop(hero_t * hero, hero_t * enemy);
void   battle(hero_t * hero, hero_t * enemy);
// Abilities.
void   attack_enemy(hero_t * hero, hero_t * enemy);
void   breath(hero_t * hero, hero_t * enemy);
size_t cure(hero_t * h);

void   sum_procs(size_t * h_proc_sum, hero_t * h);
float  get_melee_dmg(const hero_t * h);
size_t get_spell_dmg(const hero_t * h);
float  get_mitigation(const hero_t * h);
float  get_resist(const hero_t * h);
size_t get_dodge(const hero_t * h);
size_t attack_barrier(size_t final_dmg, hero_t * enemy);
void   regen(hero_t * h);
size_t restore_hp(hero_t * h, const size_t amnt);
size_t restore_mp(hero_t * h, const size_t amnt);
size_t spend_hp(hero_t * h, const size_t amnt);
size_t spend_mp(hero_t * h, const size_t amnt);



int
main(int    argc   __attribute__((unused)),
     char * argv[] __attribute__((unused)))
{
    {
        pid_t  pid = getpid();
        time_t t = time(0);

        srand(pid * t);
    }

    size_t h_ini_lvl = 50;
    size_t e_ini_lvl = 10;

    hero_t hero = roll_hero("Tim the Enchanter", h_ini_lvl);
    print_hero(&hero, 1);

    size_t xp_req = 10;

    for (;;) {
        hero_t enemy = roll_mob("ruby dragon", e_ini_lvl, DRAGON);
        print_hero(&enemy, 1);

        battle(&hero, &enemy);

        if (!hero.hp) {
            break;
        }

        set_hp_mp(&hero);

        hero.xp++;

        if (hero.xp >= xp_req) {
            level_up(&hero);
            xp_req++;

            e_ini_lvl++;
        }
    }

    return EXIT_SUCCESS;
}



hero_t
roll_mob(const char * name,
         const size_t lvl,
         const mob_t  mob)
{
    switch (mob) {
    case HERO:
        return roll_hero(name, lvl);
    case ANIMAL:
        return roll_hero(name, lvl);
    case DRAGON:
        return roll_dragon(name, lvl);
    default:
        return roll_hero(name, lvl);
    }
}



hero_t
roll_hero(const char * name,
          const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    h.attack = attack_enemy;
    h.heal = cure;

    h.level = lvl ? lvl : 1;

    h.base.sta = 6 + (rand() % 12);
    h.base.str = 6 + (rand() % 12);
    h.base.agi = 6 + (rand() % 12);
    h.base.wis = 6 + (rand() % 12);
    h.base.spr = 6 + (rand() % 12);

    if (name && *name) {
        strcpy(h.name, name);
    }

    h.items[CHEST] = create_item("plain shirt", h.level, CHEST);
    h.have_item[CHEST] = 1;

    h.items[PANTS] = create_item("worn pants", h.level, PANTS);
    h.have_item[PANTS] = 1;

    set_hp_mp(&h);

    return h;
}



hero_t
roll_dragon(const char * name,
            const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    h.attack = breath;

    h.level = lvl ? lvl : 1;

    size_t base = 6 + h.level;
    size_t var = 12 + h.level;

    h.base.sta = base + (rand() % var);
    h.base.str = base + (rand() % var);
    h.base.agi = base + (rand() % var);
    h.base.wis = base + (rand() % var);
    h.base.spr = base + (rand() % var);

    if (name && *name) {
        strcpy(h.name, name);
    }

    set_hp_mp(&h);

    return h;
}




void
print_hero(hero_t *     h,
           const size_t concise)
{
    printf("name: %s\n", h->name);
    printf("\n");
    printf("level: %zu\n", h->level);
    printf("hp:    %zu\n", h->hp);
    printf("mp:    %zu\n", h->mp);
    printf("xp:    %zu\n", h->xp);

    if (concise) {
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
    // TODO: should set armor?
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

    set_hp_mp(h);

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



size_t
get_max_hp(hero_t * h)
{
    // hp =  5 * stamina
    size_t stamina = h->base.sta;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        stamina += h->items[i].attr.sta;
    }

    return (5 * stamina);
}



size_t
get_max_mp(hero_t * h)
{
    // mp = 10 * wisdom
    size_t wisdom = h->base.wis;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        wisdom += h->items[i].attr.wis;
    }

    return (10 * wisdom);
}



float
get_melee_dmg(const hero_t * h)
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

    float  dmg = 0;
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

    // Add a smear to dmg, to give it some randomness.
    float smear = 0.01 * (80 + (rand () % 41));

    return dmg * smear;
}



size_t
get_spell_dmg(const hero_t * h)
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



float
get_mitigation(const hero_t * h)
{
    float  armor = h->armor;
    float  mitigation;

    mitigation = 1 - (armor / (armor + ARMOR_HALF_POINT));

    // TODO: sum over armor bonuses from gear?
    //       How will armor be calculated?

    return mitigation;
}



float
get_resist(const hero_t * h __attribute__((unused)))
{
    float resist;

    // TODO: how will this be calculated?

    resist = 0;

    return resist;
}



size_t
get_dodge(const hero_t * h)
{
    float  agi = 0;
    size_t dodge = 0;

    agi += h->base.agi;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            continue;
        }

        agi += h->items[i].attr.agi;
    }

    dodge = (size_t) floor(100 * agi / (agi + DODGE_HALF_POINT));

    return dodge;
}



void
attack_enemy(hero_t * hero,
             hero_t * enemy)
{
    {
        // Calculate dodge first. If attack misses, nothing left to do.
        size_t dodge = get_dodge(enemy);
        size_t trigger = rand() % 100;

        if (dodge > trigger) {
            printf("%s attacked %s and missed\n", hero->name,
                   enemy->name);

            return;
        }
    }

    float  base_dmg;
    float  mitigation;
    size_t is_crit = 0;
    size_t final_dmg;
    size_t h_proc_sum[MAX_PROCS];
    size_t e_proc_sum[MAX_PROCS];

    memset(h_proc_sum, 0, sizeof(h_proc_sum));
    memset(e_proc_sum, 0, sizeof(e_proc_sum));

    sum_procs(h_proc_sum, hero);
    sum_procs(e_proc_sum, enemy);

    base_dmg = get_melee_dmg(hero);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = rand() % 100;

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
        // TODO: print thorn dmg.
        size_t thorn_dmg = e_proc_sum[THORNS];
        attack_barrier(thorn_dmg, hero);
    }

    // TODO: print overkill?

    if (is_crit) {
        printf("%s crit %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }
    else {
        printf("%s attacked %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }

    return;
}



size_t
cure(hero_t * h)
{
    const char * what = "cure";
    const size_t cost = 16;

    if (!spend_mp(h, cost)) {
        return 0;
    }

    size_t n = restore_hp(h, get_spell_dmg(h));

    printf("%s healed %s for %zu hp\n", what, h->name, n);

    return n;
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
regen(hero_t * h)
{
    // Spirit regen is 0.5 of spirit every 2 rounds.
    float  spr = h->base.spr;
    size_t regen_amnt = 0;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            continue;
        }

        spr += h->items[i].attr.spr;
    }

    if (!spr) {
        // Nothing to do.
        return;
    }

    regen_amnt = (size_t) floor(0.5 * spr);

    size_t hp_gain = restore_hp(h, regen_amnt);
    size_t mp_gain = restore_mp(h, regen_amnt);

    printf("%s regenerated %zu hp, %zu mp\n", h->name, hp_gain,
           mp_gain);

    return;
}



void
breath(hero_t * hero,
       hero_t * enemy)
{
    // Breath cannot be dodged or resisted.
    // Breath penetrates barriers.
    float  base_dmg;
    size_t is_crit = 0;

    base_dmg = 2 * (hero->base.str + hero->base.wis);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = rand() % 100;

        if (crit > trigger) {
            is_crit = 1;
            base_dmg = base_dmg * 1.5;
        }
    }

    size_t hp_reduced = attack_barrier(base_dmg, enemy);

    if (is_crit) {
        printf("%s crit %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }
    else {
        printf("%s attacked %s for %zu hp damage\n", hero->name,
               enemy->name, hp_reduced);
    }

    return;
}




void
battle(hero_t * hero,
       hero_t * enemy)
{
    size_t regen_ctr = 0;

    for (;;) {
        ++regen_ctr;

        decision_loop(hero, enemy);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        sleep(1);

        enemy->attack(enemy, hero);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        printf("\n");

        sleep(1);

        if (regen_ctr == 2) {
            regen(hero);
            regen(enemy);
            printf("\n");

            regen_ctr = 0;

            sleep(1);
        }
    }

    if (!hero->hp && enemy->hp) {
        printf("%s has defeated %s!\n\n", enemy->name, hero->name);
    }
    else if (hero->hp && !enemy->hp) {
        printf("%s has defeated %s!\n\n", hero->name, enemy->name);
    }

    return;
}



void
decision_loop(hero_t * hero,
              hero_t * enemy)
{
    size_t done = 0;

    for (;;) {
        char act_var = (char) fgetc(stdin);

        switch (act_var) {
        case 'a':
            hero->attack(hero, enemy);
            done = 1;
            break;

        case 'h':
            if (hero->heal(hero)) {
                done = 1;
            }

            break;

        default:
            printf("error: invalid input %c\n", act_var);
            break;
        }

        while (fgetc(stdin) != '\n');

        if (done) { break; }
    }

    return;
}



void
none(hero_t * h     __attribute__((unused)),
     hero_t * enemy __attribute__((unused)))
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



void
print_fld(const char * what,
          const size_t amnt)
{
    if (amnt) {
        printf("%s %zu\n", what, amnt);
    }

    return;
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
