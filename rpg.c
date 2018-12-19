#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_PROCS (14)
#define MAX_ITEMS (11)
#define MAX_INVENTORY (32)
#define MAX_NAME_LEN  (32)
#define ARMOR_HALF_POINT (100)
#define DODGE_HALF_POINT (100)

#define PORTRAIT_ROW (1)
#define PORTRAIT_COL (4)
#define BATTLE_TXT_ROW (13)
#define BATTLE_TXT_COL (4)
#define MAX_BATTLE_TXT_LINES (32)
#define NEW_ITEM_ROW (1)
#define NEW_ITEM_COL (96)
#define INVENTORY_ROW (4)
#define INVENTORY_COL (96)
#define SELECT_ROW (1)
#define SELECT_COL (64)
#define INV_PROMPT_ROW (16)
#define INV_PROMPT_COL (64)


#define USLEEP_INTERVAL (500000)

#define ITEM_DROP_THRESH (10)

static size_t row_ = 0;
static size_t col_ = 0;

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
    TRINKET   = 10,
    HP_POTION = 20,
    MP_POTION = 21,
    NO_ITEM   = 98,
    RANDOM_S  = 99
} slot_t;

#define MAX_ARMOR_TYPES (7)
typedef enum {
    CLOTH    = 0,
    LEATHER  = 1,
    MAIL     = 2,
    PLATE    = 3,
    SHIELD   = 4,
    WEAPON   = 5,
    MISC     = 6,
    NO_ARMOR = 98,
    RANDOM_A = 99
} armor_t;

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
    size_t non_elemental;
    size_t restoration;
} spell_t;

typedef enum {
    FIRE,
    FROST,
    SHADOW,
    NON_ELEM,
    RESTORATION
} element_t;

typedef enum {
    NO_SMEAR,
    STD_SMEAR
} smear_t;

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

typedef enum {
    COMMON = 0,
    GOOD   = 1,
    RARE   = 2,
    EPIC   = 3
} tier_t;

typedef struct {
    char    name[MAX_NAME_LEN + 1];
    stats_t attr;
    spell_t power;
    spell_t resist;
    proc_t  procs[MAX_PROCS];
    slot_t  slot;
    size_t  is_weapon;
    size_t  armor; // TODO: armor?
    size_t  stack; // 0 for non-stacking, n for amount.
 // size_t  buff_duration; // 0 for infinite, n for rounds? Not sure about this.
    tier_t  tier; // 0 for common, 1 for good, 2 rare, 3 epic, 4 legendary?
} item_t;

// TODO: 1. need way to track temporary buffs/debuffs.
//       2. gold? vendors, can accumulate gold?
//       3. talent points? specializations?
//       4. Boss fights every 5 levels? Must beat boss to unlock
//          next span of levels?
//       5. random gear drops from mobs?

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
    size_t  (*spell)(struct hero_t *, struct hero_t *, const element_t element);
    void    (*defend)(struct hero_t *);
    size_t  (*heal)(struct hero_t *);
    // hero specific fields.
    size_t  xp;
    item_t  items[MAX_ITEMS];       // Equipped items.
    size_t  have_item[MAX_ITEMS];
    item_t  inventory[MAX_INVENTORY];
    size_t  have_inventory[MAX_INVENTORY];
 // item_t  buffs[MAX_BUFFS];       // Placeholder, not sure about this.
};

typedef struct hero_t hero_t;

#define MAX_MOB_TYPES (5)

typedef enum {
    HERO     = 1,
    HUMANOID = 2,
    ANIMAL   = 3,
    UNDEAD   = 4,
    DRAGON   = 5,
    RANDOM_M = 99
} mob_t;

// Basic mob gen functions.
hero_t roll_mob(const char * name, const size_t lvl, mob_t mob);
hero_t roll_hero(const char * name, const size_t lvl);
hero_t roll_dragon(const char * name, const size_t lvl);
item_t create_item(const char * name, const size_t level, const slot_t slot,
                   const size_t is_weapon);
void   spawn_item_drop(hero_t * h);
item_t gen_item(const char * name, const size_t level, const size_t is_weapon,
                armor_t armor_type, slot_t slot);
void   gen_item_name(char * name, const size_t is_weapon,
                     armor_t armor_type, slot_t slot);
size_t gen_item_armor(const size_t level, const size_t is_weapon,
                      armor_t armor_type);
stats_t get_total_stats(const hero_t * h);


void   level_up(hero_t * h);
void   set_hp_mp(hero_t * h);
size_t get_max_hp(hero_t * h);
size_t get_max_mp(hero_t * h);
// Print functions.
void         print_hero(hero_t * h, const size_t verbosity);
void         print_equip(hero_t * h);
size_t       add_to_inventory(hero_t * h, const item_t * new_item);
void         choose_inventory(hero_t * h, item_t * new_item);
void         print_inventory(const hero_t * h, const int selected,
                             const item_t * new_item);
void         print_selection(const hero_t * h, const int selected,
                             const item_t * new_item);
void         print_inventory_prompt(void);
void         sprintf_item_name(char * name, const item_t * item);
void         print_fld(const char * what, const size_t amnt);
const char * slot_to_str(slot_t s);
const char * elem_to_str(const element_t elem);
void         print_portrait(hero_t * h, const size_t i, const size_t j);
void         move_cursor(const size_t i, const size_t j);
void         reset_cursor(void);
void         set_cursor(void);
void         del_line(void);
void         del_eof(void);
void         print_act_prompt(void);
void         clear_act_prompt(void);
void         print_spell_prompt(void);
void         clear_spell_prompt(void);

// Combat functions.
void   decision_loop(hero_t * hero, hero_t * enemy);
size_t choose_spell(hero_t * hero, hero_t * enemy);
void   battle(hero_t * hero, hero_t * enemy);
// Abilities.
void   attack_enemy(hero_t * hero, hero_t * enemy);
void   breath(hero_t * hero, hero_t * enemy);
size_t spell_enemy(hero_t * hero, hero_t * enemy, const element_t element);
size_t cure(hero_t * h);

void   sum_procs(size_t * h_proc_sum, hero_t * h);
float  get_melee_dmg(const hero_t * h, smear_t smear_type);
float  get_spell_dmg(const hero_t * h, const element_t element, smear_t smear_type);
float  get_spell_res(const hero_t * h, const element_t element);
float  get_elem_pow(const spell_t * power, const element_t element);
float  get_elem_res(const spell_t * power, const element_t element);
float  get_mitigation(const hero_t * h);
size_t get_armor(const hero_t * h);
float  get_resist(const hero_t * h);
float  get_dodge(const hero_t * h);
size_t attack_barrier(size_t final_dmg, hero_t * enemy);
void   regen(hero_t * h);
size_t restore_hp(hero_t * h, const size_t amnt);
size_t restore_mp(hero_t * h, const size_t amnt);
size_t spend_hp(hero_t * h, const size_t amnt);
size_t spend_mp(hero_t * h, const size_t amnt);

static const char * action_prompt = "\n"
                                    "  choose action:\n"
                                    "    a: attack\n"
                                    "    s: spell\n"
                                    "    d: defend\n"
                                    "    h: heal\n"
                                    "    q: health potion\n"
                                    "    m: mana potion\n";

static const char * spell_prompt = "\n"
                                   "  choose spell:\n"
                                   "    f: fire\n"
                                   "    i: ice\n"
                                   "    s: shadow\n"
                                   "    u: non-elemental\n";

// TODO: different lists of prefixes and suffixes, maybe germanic,
//       norse, etc? Different combinations for different mobs.

#define MAX_PREFIX (50)
static const char * prefix_list[MAX_PREFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu"
};

#define MAX_SUFFIX (75)
static const char * suffix_list[MAX_SUFFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu",
    "amm", "ath", "ass", "agg", "all",
    "emm", "eth", "ess", "egg", "ell",
    "imm", "ith", "iss", "igg", "ill",
    "omm", "oth", "oss", "ogg", "oll",
    "umm", "uth", "uss", "ugg", "ull",
};

// Length of 3 for each? To make things simpler?...
// Main hand, offhand, and two hand.
static const char * shields[] = { "buckler", "pavise", "targe" };
static const char * one_hand_swords[] = { "scimitar", "sabre", "shortsword" };
static const char * one_hand_piercing[] = { "dagger", "dirk", "shard" };
static const char * one_hand_blunt[] = { "mace", "hammer", "cudgel" };
static const char * two_hand_swords[] = { "bastard sword", "claymore", "longsword" };
static const char * two_hand_piercing[] = { "lance", "trident", "spear" };
static const char * two_hand_blunt[] = { "staff", "warhammer", "maul" };
// Cloth
static const char * cloth_helm[] = { "hood", "hat", "hat" };
static const char * cloth_shoulders[] = { "amice", "mantle", "shoulders" };
static const char * cloth_chest[] = { "robe", "vest", "shirt" };
static const char * cloth_gloves[] = { "pants", "leggings", "shorts" };
static const char * cloth_pants[] = { "pants", "leggings", "shorts" };
static const char * cloth_boots[] = { "sandals", "slippers", "boots" };
// Leather
static const char * leather_helm[] = { "hood", "hat", "hat" };
static const char * leather_shoulders[] = { "amice", "mantle", "shoulders" };
static const char * leather_chest[] = { "brigandine", "vest", "shirt" };
static const char * leather_gloves[] = { "pants", "leggings", "shorts" };
static const char * leather_pants[] = { "pants", "leggings", "shorts" };
static const char * leather_boots[] = { "sandals", "slippers", "boots" };
// Mail
static const char * mail_helm[] = { "coif", "crown", "hat" };
static const char * mail_shoulders[] = { "amice", "mantle", "shoulders" };
static const char * mail_chest[] = { "hauberk", "vest", "shirt" };
static const char * mail_gloves[] = { "pants", "leggings", "shorts" };
static const char * mail_pants[] = { "pants", "leggings", "shorts" };
static const char * mail_boots[] = { "sandals", "slippers", "boots" };
// Plate
static const char * plate_helm[] = { "helm", "barbute", "hat" };
static const char * plate_shoulders[] = { "pauldrons", "mantle", "shoulders" };
static const char * plate_chest[] = { "breastplate", "vest", "shirt" };
static const char * plate_gloves[] = { "gauntlets", "leggings", "shorts" };
static const char * plate_pants[] = { "pants", "leggings", "shorts" };
static const char * plate_boots[] = { "sabatons", "slippers", "boots" };
// Trinkets and rings
static const char * trinkets[] = { "pendant", "idol", "ankh" };
static const char * rings[] = { "ring", "band", "seal" };

// TODO: titles?
//static const char * title_list[50] = {
//    " the cruel"
//}



int
main(int    argc   __attribute__((unused)),
     char * argv[] __attribute__((unused)))
{
    {
        pid_t  pid = getpid();
        time_t t = time(0);

        srand(pid * t);

        printf("\033[2J");
        printf("\033[1;1H");
    }

    size_t h_ini_lvl = 5;
    size_t e_ini_lvl = 1;

    hero_t hero = roll_hero("Tim the Enchanter", h_ini_lvl);

    print_portrait(&hero, PORTRAIT_ROW, PORTRAIT_COL);

    //size_t xp_req = 10;
    size_t xp_req = 1;

    for (;;) {
        //hero_t enemy = roll_mob("ruby dragon", e_ini_lvl, DRAGON);
        hero_t enemy = roll_mob(0, e_ini_lvl, RANDOM_M);
        print_portrait(&enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        battle(&hero, &enemy);

        if (!hero.hp) {
            break;
        }

        set_hp_mp(&hero);

        hero.xp++;

        spawn_item_drop(&hero);

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
         mob_t        mob)
{
    // Using brute force stack allocation and return by value,
    // because this will only be called once every few minutes.

    if (mob == RANDOM_M) {
        mob = rand() % (MAX_MOB_TYPES + 1);
    }

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
    h.spell = spell_enemy;
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
    else {
        strcat(h.name, prefix_list[rand() % MAX_PREFIX]);
        strcat(h.name, suffix_list[rand() % MAX_SUFFIX]);
    }

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        h.items[i].slot = NO_ITEM;
    }

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        h.inventory[i].slot = NO_ITEM;
    }

    h.items[CHEST] = create_item("plain shirt", h.level, CHEST, 0);
    h.have_item[CHEST] = 1;

    h.items[PANTS] = create_item("worn pants", h.level, PANTS, 0);
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
    else {
        strcat(h.name, prefix_list[rand() % MAX_PREFIX]);
        strcat(h.name, suffix_list[rand() % MAX_SUFFIX]);
    }

    set_hp_mp(&h);

    return h;
}





item_t
create_item(const char * name,
            const size_t level,
            const slot_t slot,
            const size_t is_weapon)
{
    // TODO: should set armor?
    item_t item;
    size_t attr_lvl = 0;
    size_t resist_lvl = 5;
    size_t power_lvl = 10;
    size_t procs_lvl = 20;
    size_t shift_lvl = level + 1;

    memset(&item, 0, sizeof(item));

    strcpy(item.name, name);

    item.slot = slot;
    item.is_weapon = is_weapon;
    item.armor = rand() % shift_lvl;

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



item_t
gen_item(const char * name,
         const size_t level,
         const size_t is_weapon,
         armor_t      armor_type,
         slot_t       slot)
{
    // Using brute force stack allocation and return by value,
    // because this will only be called once every few minutes.
    item_t item;
    size_t attr_lvl = 0;
    size_t resist_lvl = 5;
    size_t power_lvl = 10;
    size_t procs_lvl = 20;
    size_t shift_lvl = level + 1;
    size_t tier = rand() % 4;

    memset(&item, 0, sizeof(item));

    item.tier = tier;

    // is_weapon has priority over slot_t and armor_t.
    // armor_t has priority over slot_t.
    // This is why is_weapon is const.

    if (is_weapon) {
        armor_type = WEAPON;
    }
    else {
        if (armor_type == RANDOM_S) {
            armor_type = rand() % (MAX_ARMOR_TYPES + 1);
        }
    }

    if (slot == RANDOM_S) {
        switch (armor_type) {
        case WEAPON:
            // Any of 0-2 slot_t.
            slot = rand() % (TWO_HAND + 1);
            break;

        case SHIELD:
            slot = OFF_HAND;
            break;

        case CLOTH:
        case LEATHER:
        case MAIL:
        case PLATE:
            // Any of 3-8 slot_t.
            slot = 3 + (rand() % (5 + 1));
            break;

        case MISC:
        default:
            // Any of 9-10 slot_t.
            slot = 9 + (rand() % (2));
            break;
        }
    }

    if (name && *name) {
        strcpy(item.name, name);
    }
    else {
        gen_item_name(item.name, is_weapon, armor_type, slot);
    }


    item.slot = slot;
    item.is_weapon = is_weapon;
    item.armor = gen_item_armor(level, is_weapon, armor_type);
    //item.armor = rand() % shift_lvl;

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
gen_item_name(char *       name,
              const size_t is_weapon,
              armor_t      armor_type,
              slot_t       slot)
{
    size_t i = rand() % 3;
    size_t j = rand() % 2;

    if (is_weapon) {
        switch (slot) {
        case MAIN_HAND:
        case OFF_HAND:
            switch (i) {
            case 0:
                strcpy(name, one_hand_swords[j]);
                return;
            case 1:
                strcpy(name, one_hand_piercing[j]);
                return;
            case 2:
            default:
                strcpy(name, one_hand_blunt[j]);
                return;
            }
        case TWO_HAND:
        default:
            switch (i) {
            case 0:
                strcpy(name, two_hand_swords[j]);
                return;
            case 1:
                strcpy(name, two_hand_piercing[j]);
                return;
            case 2:
            default:
                strcpy(name, two_hand_blunt[j]);
                return;
            }
        }
    }

    switch (armor_type) {
    case CLOTH:
        switch(slot) {
        case HELM:
            strcpy(name, cloth_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, cloth_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, cloth_chest[j]);
            return;
        case PANTS:
            strcpy(name, cloth_pants[j]);
            return;
        case GLOVES:
            strcpy(name, cloth_gloves[j]);
            return;
        case BOOTS:
        default:
            strcpy(name, cloth_boots[j]);
            return;
        }
    case LEATHER:
        switch(slot) {
        case HELM:
            strcpy(name, leather_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, leather_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, leather_chest[j]);
            return;
        case PANTS:
            strcpy(name, leather_pants[j]);
            return;
        case GLOVES:
            strcpy(name, leather_gloves[j]);
            return;
        case BOOTS:
        default:
            strcpy(name, leather_boots[j]);
            return;
        }
    case MAIL:
        switch(slot) {
        case HELM:
            strcpy(name, mail_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, mail_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, mail_chest[j]);
            return;
        case PANTS:
            strcpy(name, mail_pants[j]);
            return;
        case GLOVES:
            strcpy(name, mail_gloves[j]);
            return;
        case BOOTS:
        default:
            strcpy(name, mail_boots[j]);
            return;
        }
    case PLATE:
        switch(slot) {
        case HELM:
            strcpy(name, plate_helm[j]);
            return;
        case SHOULDERS:
            strcpy(name, plate_shoulders[j]);
            return;
        case CHEST:
            strcpy(name, plate_chest[j]);
            return;
        case PANTS:
            strcpy(name, plate_pants[j]);
            return;
        case GLOVES:
            strcpy(name, plate_gloves[j]);
            return;
        case BOOTS:
        default:
            strcpy(name, plate_boots[j]);
            return;
        }
    case SHIELD:
        strcpy(name, shields[j]);
        return;

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
               const size_t is_weapon,
               armor_t      armor_type)
{
    // TODO: adjust stats based on armor? Cloth has caster stats, e.g.
    //       Plate should have lower stats in general to compensate for armor.
    if (is_weapon) {
        return 0;
    }

    float base_armor = level + (rand() % level);
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
    case MISC:
    default:
        multiplier = 0.5;
        break;
    }

    size_t final_armor = (size_t) floor(base_armor * multiplier);

    return final_armor;
}



void
spawn_item_drop(hero_t * h)
{
    // Anything from mana potions to armor, swords, etc.
    // Tiers of quality? Rare, epic, legendary?
    size_t trigger = rand() % 100;

    if (trigger < ITEM_DROP_THRESH) {
        return;
    }

    // This feels goofy.
    size_t drop_type = rand() % 3;
    item_t new_item;

    switch (drop_type) {
    case 0:
        new_item = gen_item(0, h->level, 0, RANDOM_A, RANDOM_S);

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

    choose_inventory(h, &new_item);

    move_cursor(1, 1);
    del_eof();

    set_hp_mp(h);

    move_cursor(1, 1);
    del_eof();

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

    print_portrait(h, PORTRAIT_ROW, PORTRAIT_COL);

    reset_cursor();
    del_eof();

    {
        // Spend a bonus point.
        printf("%s leveled up!\n", h->name);
        printf("\n");
        printf("Choose a stat for your bonus point:\n");
        printf("   1. stamina:  increases max hp\n");
        printf("   2. strength: increases melee damage\n");
        printf("   3. agility:  increases dodge and crit chance\n");
        printf("   4. wisdom:   increases max mp and spell damage\n");
        printf("   5. spirit:   increases hp and mp regen rates\n");

        size_t done = 0;

        for (;;) {
            char choice = (char) fgetc(stdin);

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
                printf("error: invalid input %c\n", choice);
                break;
            }

            while (fgetc(stdin) != '\n');

            if (done) { break; }
        }
    }

    h->xp = 0;

    set_hp_mp(h);

    print_portrait(h, PORTRAIT_ROW, PORTRAIT_COL);

    reset_cursor();
    del_eof();

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
get_melee_dmg(const hero_t * h,
              smear_t        smear_type)
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
    float smear;

    switch (smear_type) {
    case NO_SMEAR:
        smear = 1;
        break;
    case STD_SMEAR:
        smear = 0.01 * (80 + (rand () % 41));
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
    // where the spell multiplier is 3.
    //
    // TODO: spell mult of 3 is possibly OP. 2 instead?
    //
    float dmg = 0;
    float wis = 0;
    float mult = 3;
    float spell_power = 0;

    // Get unit's innate elemental power and wisdom.
    spell_power += get_elem_pow(&h->power, element);
    wis += h->base.wis;

    // Then get elemental power and wisdom from gear.
    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
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
        smear = 0.01 * (80 + (rand () % 41));
        break;
    }

    return dmg * smear;
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
    float  spell_power = 0;

    switch (element) {
    case FIRE:
        spell_power += resist->fire;
        spell_power += 0.5 * resist->non_elemental;
        break;
    case FROST:
        spell_power += resist->frost;
        spell_power += 0.5 * resist->non_elemental;
        break;
    case SHADOW:
        spell_power += resist->shadow;
        spell_power += 0.5 * resist->non_elemental;
        break;
    case NON_ELEM:
        spell_power += resist->non_elemental;
        spell_power += 0.5 * resist->fire;
        spell_power += 0.5 * resist->frost;
        spell_power += 0.5 * resist->shadow;
        break;
    case RESTORATION:
        break;
    }

    return spell_power;
}



float
get_mitigation(const hero_t * h)
{
    float  armor = (float) get_armor(h);
    float  mitigation;

    mitigation = 1 - (armor / (armor + ARMOR_HALF_POINT));

    // TODO: sum over armor bonuses from gear?
    //       How will armor be calculated?

    return mitigation;
}



size_t
get_armor(const hero_t * h)
{
    size_t armor = h->armor;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        armor += h->items[i].armor;
    }

    return armor;
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
        if (!h->have_item[i]) {
            // Nothing to calculate.
            continue;
        }

        res += get_elem_pow(&h->items[i].resist, element);
    }

    mitigation = 1 - (res / (res + ARMOR_HALF_POINT));

    return mitigation;
}



float
get_dodge(const hero_t * h)
{
    // Dodge is four digit float, e.g. 48.38%
    // 0 - 9999.
    float agi = 0;
    float dodge = 0;

    agi += h->base.agi;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            continue;
        }

        agi += h->items[i].attr.agi;
    }

    dodge = floor(10000 * agi / (agi + DODGE_HALF_POINT));

    return dodge;
}



float
get_spell_crit(const hero_t * h)
{
    float  wis = 0;
    size_t crit = 0;

    wis += h->base.wis;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            continue;
        }

        wis += h->items[i].attr.wis;
    }

    crit = (size_t) floor(100 * wis / (wis + DODGE_HALF_POINT));

    return crit;
}



void
attack_enemy(hero_t * hero,
             hero_t * enemy)
{
    {
        // Calculate dodge first. If attack misses, nothing left to do.
        float dodge = get_dodge(enemy);
        float trigger = rand() % 10000;

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

    base_dmg = get_melee_dmg(hero, STD_SMEAR);

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
spell_enemy(hero_t *        hero,
            hero_t *        enemy,
            const element_t element)
{
    size_t cost = 0;

    switch (element) {
        case NON_ELEM:
            cost = 32;
            break;

        default:
            cost = 16;
            break;
    }

    if (!spend_mp(hero, cost)) {
        return 0;
    }

    // Spells ignore armor and all procs.
    // Spell resistance functions as armor mitigation.
    float        base_dmg;
    float        resist;
    size_t       is_crit = 0;
    size_t       final_dmg;
    const char * what = elem_to_str(element);

    base_dmg = get_spell_dmg(hero, element, STD_SMEAR);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = rand() % 100;

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
cure(hero_t * h)
{
    const char * what = "cure";
    const size_t cost = 16;

    if (!spend_mp(h, cost)) {
        return 0;
    }

    size_t n = restore_hp(h, get_spell_dmg(h, RESTORATION, STD_SMEAR));

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
    print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
    print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

    size_t regen_ctr = 0;

    for (;;) {
        ++regen_ctr;

        decision_loop(hero, enemy);
        ++row_;

        print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
        print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        usleep(USLEEP_INTERVAL);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        enemy->attack(enemy, hero);
        ++row_;

        print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
        print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

        usleep(USLEEP_INTERVAL);

        if (!hero->hp || !enemy->hp) {
            break;
        }

        if (regen_ctr == 2) {
            regen(hero);
            regen(enemy);

            ++row_;
            ++row_;
            print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));

            regen_ctr = 0;

            usleep(USLEEP_INTERVAL);
        }

        if (row_ >= MAX_BATTLE_TXT_LINES) {
            row_ = 0;
        }

        set_cursor();
        del_eof();
    }

    row_ = 0;

    if (!hero->hp && enemy->hp) {
        printf("%s has defeated %s!\n\n", enemy->name, hero->name);
    }
    else if (hero->hp && !enemy->hp) {
        printf("%s has defeated %s!\n\n", hero->name, enemy->name);
    }

    reset_cursor();
    del_eof();


    usleep(USLEEP_INTERVAL);

    return;
}



void
decision_loop(hero_t * hero,
              hero_t * enemy)
{
    size_t done = 0;

    for (;;) {
        print_act_prompt();

        char act_var = (char) fgetc(stdin);
        while (fgetc(stdin) != '\n');

        clear_act_prompt();

        switch (act_var) {
        case 'a':
            hero->attack(hero, enemy);
            done = 1;
            break;

        case 's':
            if (choose_spell(hero, enemy)) {
                done = 1;
            }
            else {
                printf("%s is out of mana\n", hero->name);
            }

            break;

        case 'h':
            if (hero->heal(hero)) {
                done = 1;
            }
            else {
                printf("%s is out of mana\n", hero->name);
            }

            break;

        default:
            printf("error: invalid input %c\n", act_var);
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
        print_spell_prompt();

        char act_var = (char) fgetc(stdin);
        while (fgetc(stdin) != '\n');

        clear_spell_prompt();

        switch (act_var) {
        case 'f':
            status = hero->spell(hero, enemy, FIRE);
            done = 1;
            break;

        case 'i':
            status = hero->spell(hero, enemy, FROST);
            done = 1;
            break;

        case 's':
            status = hero->spell(hero, enemy, SHADOW);
            done = 1;
            break;

        case 'u':
            status = hero->spell(hero, enemy, NON_ELEM);
            done = 1;
            break;

        default:
            printf("error: invalid input %c\n", act_var);
            break;
        }

        if (done) { break; }
    }

    return status;
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
print_hero(hero_t *     h,
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
        if (!h->have_item[i]) {
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
print_equip(hero_t * h)
{
    printf("name:  %s\n", h->name);
    printf("level: %zu\n", h->level);
    printf("hp:    %zu\n", get_max_hp(h));
    printf("mp:    %zu\n", get_max_mp(h));
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
    printf("attack dmg: %.2f\n", get_melee_dmg(h, NO_SMEAR));
    printf("spell dmg:\n");
    printf("  fire:     %.2f\n", get_spell_dmg(h, FIRE, NO_SMEAR));
    printf("  frost:    %.2f\n", get_spell_dmg(h, FROST, NO_SMEAR));
    printf("  shadow:   %.2f\n", get_spell_dmg(h, SHADOW, NO_SMEAR));
    printf("  non-elem: %.2f\n", get_spell_dmg(h, NON_ELEM, NO_SMEAR));

    printf("\n");
    printf("equipment\n");

    char pretty_name[MAX_NAME_LEN + 1];

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        sprintf_item_name(pretty_name, &h->items[i]);

        printf("  %s: \e[1;32m%s\e[0m\n", slot_to_str(i), pretty_name);
    }

    fflush(stdout);

    return;
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
elem_to_str(const element_t elem)
{
    switch (elem) {
        case FIRE:
            return "fire";
        case FROST:
            return "frost";
        case SHADOW:
            return "shadow";
        case NON_ELEM:
            return "non-elemental";
        case RESTORATION:
            return "restoration";
    }
}



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
    printf("\033[%zu;%zuH level: %zu    ",       i + 2, j, h->level);
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



size_t
add_to_inventory(hero_t *       h,
                 const item_t * new_item)
{
    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        if (!h->have_inventory[i]) {
            h->inventory[i] = *new_item;
            h->have_inventory[i] = 1;

            return 1;
        }
    }

    // Inventory full.
    return 0;
}



void
choose_inventory(hero_t * h,
                 item_t * new_item)
{
    size_t done = 0;
    int    selection = -1;

    move_cursor(1, 1);
    del_eof();

    print_equip(h);
    print_selection(h, selection, new_item);
    print_inventory(h, selection, new_item);

    for (;;) {
        print_inventory_prompt();

        char act_var = (char) fgetc(stdin);

        if (act_var == '\033') {
            fgetc(stdin);
            act_var = (char) fgetc(stdin);

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
                // TODO: need to make consumables stack.
                done = add_to_inventory(h, new_item);
                break;

            case 'd':
                if (selection < 0) {
                    new_item->slot = NO_ITEM;
                }
                else {
                    h->inventory[selection].slot = NO_ITEM;
                    h->have_inventory[selection] = 0;
                }

                break;

            case 'e':
                // TODO: need to handle 2 hand being exclusive with one
                //       hand and offhand.
                if (selection >= 0) {
                    if (!h->have_inventory[selection]) {
                        // Can't equip something from inventory
                        // that's not there.
                        break;
                    }
                }

                // Get pointer to selected item, and swap equipped item
                // with selected item.
                item_t * s_i;
                slot_t   s_i_slot;

                if (selection < 0) {
                    s_i = new_item;
                    s_i_slot = new_item->slot;
                }
                else {
                    s_i = &h->inventory[selection];
                    s_i_slot = h->inventory[selection].slot;
                }

                if (s_i_slot > MAX_ITEMS - 1) {
                    // This isn't an equippable item (It's a potion
                    // or NO_ITEM).
                    break;
                }

                item_t old_item = h->items[s_i_slot];
                size_t have_old_item = h->have_item[s_i_slot];

                h->items[s_i_slot] = *s_i;
                h->have_item[s_i_slot] = 1;

                if (selection < 0) {
                    *new_item = old_item;

                    if (!have_old_item) {
                        new_item->slot = NO_ITEM;
                    }
                }
                else {
                    h->inventory[selection] = old_item;
                    h->have_inventory[selection] = have_old_item;

                    if (!have_old_item) {
                        h->inventory[selection].slot = NO_ITEM;
                    }
                }

                break;

            case 'q':
                done = 1;
                break;

            default:
                break;
            }
        }

        move_cursor(1, 1);
        del_eof();

        print_equip(h);
        print_selection(h, selection, new_item);
        print_inventory(h, selection, new_item);

        while (fgetc(stdin) != '\n');

        if (done) { break; }
    }

    return;
}



void
print_inventory(const hero_t * h,
                const int      selected,
                const item_t * new_item)
{
    char   pretty_name[MAX_NAME_LEN + 1];

    {
        size_t row = NEW_ITEM_ROW;
        size_t col = NEW_ITEM_COL;
        size_t shift = 1;

        sprintf_item_name(pretty_name, new_item);

        printf("\033[%zu;%zuH New Item", row, col);

        if (-1 == selected) {
            printf("\033[%zu;%zuH * %s: %s", row + shift, col,
                   slot_to_str(new_item->slot), pretty_name);
        }
        else {
            printf("\033[%zu;%zuH   %s: %s", row + shift, col,
                   slot_to_str(new_item->slot), pretty_name);
        }

    }

    size_t row = INVENTORY_ROW;
    size_t col = INVENTORY_COL;
    size_t shift = 1;

    printf("\033[%zu;%zuH Inventory", row, col);

    for (size_t i = 0; i < MAX_INVENTORY; ++i) {
        const item_t * item = &h->inventory[i];

        sprintf_item_name(pretty_name, item);

        if ((int) i == selected) {
            printf("\033[%zu;%zuH * %s: %s", row + shift, col,
                   slot_to_str(item->slot), pretty_name);
        }
        else {
            printf("\033[%zu;%zuH   %s: %s", row + shift, col,
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

    printf("\033[%zu;%zuH Item Select", row, col);

    if (item->slot == NO_ITEM) {
        // Nothing to print.
        return;
    }

    char   pretty_name[MAX_NAME_LEN + 1];

    sprintf_item_name(pretty_name, new_item);

    printf("\033[%zu;%zuH %s", row + 2, col, pretty_name);

    if (item->slot == HP_POTION || item->slot == MP_POTION) {
        // No other stats to print.
        return;
    }

    printf("\033[%zu;%zuH armor: %zu", row + 3, col, item->armor);
    printf("\033[%zu;%zuH sta:   %zu", row + 4, col, item->attr.sta);
    printf("\033[%zu;%zuH str:   %zu", row + 5, col, item->attr.str);
    printf("\033[%zu;%zuH agi:   %zu", row + 6, col, item->attr.agi);
    printf("\033[%zu;%zuH wis:   %zu", row + 7, col, item->attr.wis);
    printf("\033[%zu;%zuH spr:   %zu", row + 8, col, item->attr.spr);

    fflush(stdout);

    return;
}



void
print_inventory_prompt(void)
{
    size_t row = INV_PROMPT_ROW;
    size_t col = INV_PROMPT_COL;

    printf("\033[%zu;%zuH Actions:", row, col);

    printf("\033[%zu;%zuH a: add to inventory", row + 3, col);
    printf("\033[%zu;%zuH d: throw away selected item", row + 4, col);
    printf("\033[%zu;%zuH e: equip selected item", row + 5, col);
    printf("\033[%zu;%zuH q: quit", row + 6, col);

    fflush(stdout);

    return;
}



void
sprintf_item_name(char *         name,
                  const item_t * item)
{

    switch (item->tier) {
    case GOOD:
        sprintf(name, "\e[1;32m%s\e[0m", item->name);
        break;

    case RARE:
        sprintf(name, "\e[1;34m%s\e[0m", item->name);
        break;

    case EPIC:
        sprintf(name, "\e[1;35m%s\e[0m", item->name);
        break;

    case COMMON:
    default:
        sprintf(name, "\e[1;0m%s\e[0m", item->name);
        break;
    }

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
print_spell_prompt(void)
{
    printf("%s", spell_prompt);

    return;
}



void
clear_spell_prompt(void)
{
    const char * ptr = spell_prompt;

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



stats_t
get_total_stats(const hero_t * h)
{
    stats_t stats;

    stats.sta = h->base.sta;
    stats.str = h->base.str;
    stats.agi = h->base.agi;
    stats.wis = h->base.wis;
    stats.spr = h->base.spr;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
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

