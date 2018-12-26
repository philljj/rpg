// TODO: 1. need way to track temporary buffs/debuffs.
//       2. gold? vendors, can accumulate gold?
//       3. talent points? specializations?
//       4. Boss fights every 5 levels? Must beat boss to unlock
//          next span of levels?
//       5. Unlockable special abilities. Druids shadeshift to animal
//          forms, but can't equip plate or weapons to use this ability.
//       6. Shield block mechanics? Shield spike?
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_ITEMS            (11)  // Max item slots on hero.
#define MAX_INVENTORY        (32)  // Max bag space slots on hero.
#define MAX_NAME_LEN         (32)
#define MAX_PROCS            (14)  // Max procs per item.

#define HP_MULT              (4)   // max hp = 4 * stamina
#define MP_MULT              (4)   // max mp = 4 * wisdom

#define SPELL_DMG_MULT       (1.6)
#define TWO_HAND_MULT        (1.6)
#define ONE_HAND_MULT        (0.6)
#define UNARMED_MULT         (0.5)

#define ARMOR_HALF_POINT     (200) // Armor required for 50% melee reduction.
#define DODGE_HALF_POINT     (500) // Agility required for 50% dodge.
#define BASE_STAT            (7)
#define BASE_STAT_VAR        (7)

#define BATTLE_TXT_ROW       (13)
#define BATTLE_TXT_COL       (4)
#define INVENTORY_ROW        (4)
#define INVENTORY_COL        (96)
#define INV_PROMPT_ROW       (16)
#define INV_PROMPT_COL       (64)
#define MAX_BATTLE_TXT_LINES (32)
#define NEW_ITEM_ROW         (1)
#define NEW_ITEM_COL         (96)
#define PORTRAIT_ROW         (1)
#define PORTRAIT_COL         (4)
#define SELECT_ROW           (1)
#define SELECT_COL           (64)

#define USLEEP_INTERVAL (500000)

#define ITEM_DROP_THRESH (10)

static size_t row_ = 0;
static size_t col_ = 0;

// slot_t is where item goes.
typedef enum {
    MAIN_HAND = 0,
    OFF_HAND  = 1,
    TWO_HAND  = 2,
    HEAD      = 3,
    SHOULDERS = 4,
    CHEST     = 5,
    LEGS      = 6,
    HANDS     = 7,
    FEET      = 8,
    RING      = 9,
    TRINKET   = 10,
    HP_POTION = 20,
    MP_POTION = 21,
    NO_ITEM   = 98,
    RANDOM_S  = 99
} slot_t;

// armor_t is what item is.
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

// weapon_t is what weapon type the item is.
typedef enum {
    PIERCING  = 0,
    EDGED     = 1,
    BLUNT     = 2,
    NO_WEAPON = 98,
    RANDOM_W  = 99
} weapon_t;

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
    COMMON      = 0,
    GOOD        = 1,
    RARE        = 2,
    EPIC        = 3,
    RANDOM_TIER = 99
} tier_t;

typedef struct {
    char     name[MAX_NAME_LEN + 1];
    stats_t  attr;
    spell_t  power;
    spell_t  resist;
    proc_t   procs[MAX_PROCS];
    slot_t   slot;
    armor_t  armor_type;
    weapon_t weapon_type;
    size_t   armor; // TODO: armor?
    size_t   stack; // 0 for non-stacking, n for amount.
 // size_t   buff_duration; // 0 for infinite, n for rounds? Not sure about this.
    tier_t   tier; // 0 for common, 1 for good, 2 rare, 3 epic, 4 legendary?
} item_t;

typedef enum {
    HERO      = 0,
    HUMANOID  = 1,
    ANIMAL    = 2,
    UNDEAD    = 3,
    DRAGON    = 5,
    NO_ANIMAL = 98,
    RANDOM_M  = 99
} mob_t;

typedef enum {
    FLYING        = 0,
    DOG           = 1,
    CAT           = 2,
    BOAR          = 3,
    BEAR          = 4,
    RANDOM_ANIMAL = 99
} animal_t;

typedef enum {
    THIEF           = 0, // cloth, leather, one hand
    BARBARIAN       = 1, // cloth, leather, two hand
    SOLDIER         = 2, // mail, shield, one hand
    PRIEST          = 3, // cloth, leather, restoration
    DRUID           = 4, // cloth, leather, mail, shapeshifting
    WIZARD          = 5, // cloth, leather, spell
    NECROMANCER     = 6, // cloth, leather, shadow life drain
    KNIGHT          = 7, // plate, shield, one hand, or two hand
    RANDOM_HUMANOID = 99
} humanoid_t;

typedef enum {
    WHELPLING     = 0,
    FOREST_DRAGON = 1,
    SAND_DRAGON   = 2,
    WATER_DRAGON  = 3,
    FIRE_DRAGON   = 4,
    RANDOM_DRAGON = 99
} dragon_t;

struct hero_t {
    // Common fields.
    char     name[MAX_NAME_LEN + 1]; // There are Some who call me Tim.
    mob_t    mob_type;
    int      sub_type;               // sub_type of mob.
    size_t   level;
    size_t   hp;                     // Health Points.
    size_t   mp;                     // Mana Points.
    size_t   bp;                     // Barrier Points.
    size_t   armor;                  // Mitigates physical damage.
    stats_t  base;                   // Base attributes. Increases with level.
    spell_t  power;                  // Enhances spell damage given.
    spell_t  resist;                 // Resists spell damage received.
    // Need way to handle cooldowns for these...
    void     (*attack)(struct hero_t *, struct hero_t *);
    size_t   (*spell)(struct hero_t *, struct hero_t *, const element_t element);
    void     (*defend)(struct hero_t *);
    size_t   (*heal)(struct hero_t *);
    // hero specific fields.
    size_t   xp;
    item_t   items[MAX_ITEMS];       // Equipped items.
    size_t   have_item[MAX_ITEMS];
    item_t   inventory[MAX_INVENTORY];
 // item_t buffs[MAX_BUFFS];       // Placeholder, not sure about this.
};

typedef struct hero_t hero_t;

// Basic mob gen functions.
hero_t roll_mob(const char * name, const size_t lvl, mob_t mob);
hero_t roll_hero(const char * name, const size_t lvl);
hero_t roll_humanoid(const char * name, const size_t lvl);
hero_t roll_animal(const char * name, const size_t lvl);
hero_t roll_dragon(const char * name, const size_t lvl);
void   gen_base_stats(hero_t * h);
item_t create_item(const char * name, const size_t level, const slot_t slot,
                   const size_t is_weapon);
void   spawn_item_drop(hero_t * h);
tier_t gen_item_tier(void);
item_t gen_item(const char * name, const size_t level, tier_t tier,
                size_t is_weapon, armor_t armor_type, slot_t slot,
                weapon_t weapon_type);
void   gen_item_name(char * name, const armor_t armor_type, const slot_t slot,
                     weapon_t weapon_type);
size_t gen_item_armor(const size_t level, armor_t armor_type);
void   gen_item_set(hero_t * h, const size_t lvl,
                    const tier_t tier, const armor_t armor_type);
stats_t get_total_stats(const hero_t * h);


void   level_up(hero_t * h);
void   set_hp_mp(hero_t * h);
size_t get_max_hp(const hero_t * h);
size_t get_max_mp(const hero_t * h);
// Print functions.
void         print_hero(hero_t * h, const size_t verbosity);
void         print_equip(hero_t * h);
item_t *     add_to_inventory(hero_t * h, item_t * new_item);
void         choose_inventory(hero_t * h, item_t * new_item);
void         equip_from_inventory(hero_t * h, const int selection,
                                  item_t * new_item);
void         use_from_inventory(hero_t * h, const int selection,
                                item_t *  new_item);
void         print_inventory(const hero_t * h, const int selected,
                             const item_t * new_item);
void         print_selection(const hero_t * h, const int selected,
                             const item_t * new_item);
void         print_inventory_prompt(void);
void         sprintf_item_name(char * name, const item_t * item);
void         print_fld(const char * what, const size_t amnt);
const char * slot_to_str(slot_t s);
const char * armor_to_str(armor_t a);
const char * mob_t_to_str(const mob_t m);
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
void   attack_enemy(hero_t * hero, hero_t * enemy, const item_t * weapon);
void   weapon_attack(hero_t * hero, hero_t * enemy);
void   breath(hero_t * hero, hero_t * enemy);
size_t spell_enemy(hero_t * hero, hero_t * enemy, const element_t element);
size_t cure(hero_t * h);

void   sum_procs(size_t * h_proc_sum, hero_t * h);
float  get_melee_dmg(const hero_t * h, const item_t * weapon,
                     smear_t smear_type);
float  get_spell_dmg(const hero_t * h, const element_t element,
                     smear_t smear_type);
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
                                    "    i: inventory\n";

static const char * spell_prompt = "\n"
                                   "  choose spell:\n"
                                   "    f: fire\n"
                                   "    i: ice\n"
                                   "    s: shadow\n"
                                   "    u: non-elemental\n";

// TODO: different lists of prefixes and suffixes, maybe germanic,
//       norse, etc? Different combinations for different mobs.

// Animals.
static const char * flying_list[] = {
    "bat", "gull", "owl", "buzzard"
};

static const char * dog_list[] = {
    "starving wolf", "coyote", "bloodhound", "dire wolf"
};

static const char * cat_list[] = {
    "starving panther", "mountain lion", "shadowcat", "tiger"
};

static const char * boar_list[] = {
    "boar", "tusked boar", "pig", "great boar"
};

static const char * bear_list[] = {
    "brown bear", "black bear", "polar bear", "dire bear"
};

// Dragons.
static const char * whelp_list[] = {
    "whelp", "forest whelp", "whelpling", "searing whelp"
};

static const char * forest_dragon_list[] = {
    "verdant dragon", "forest dragon", "moss drake", "green wyrm"
};

static const char * sand_dragon_list[] = {
    "sand serpent", "desert dragon", "yellow drake", "sand wyrm"
};

static const char * water_dragon_list[] = {
    "water drake", "lake serpent", "blue drake", "deep ocean wyrm"
};

static const char * fire_dragon_list[] = {
    "searing drake", "ember serpent", "inferno dragon", "magma wyrm"
};

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

#define NUM_ITEM_NAMES (3)
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
static const char * cloth_gloves[] = { "mitts", "handwraps", "gloves" };
static const char * cloth_pants[] = { "pants", "breeches", "shorts" };
static const char * cloth_boots[] = { "sandals", "slippers", "boots" };
// Leather
static const char * leather_helm[] = { "hood", "hat", "hat" };
static const char * leather_shoulders[] = { "amice", "mantle", "shoulders" };
static const char * leather_chest[] = { "brigandine", "vest", "shirt" };
static const char * leather_gloves[] = { "gloves", "handguards", "gloveletts" };
static const char * leather_pants[] = { "pants", "leggings", "shorts" };
static const char * leather_boots[] = { "sandals", "slippers", "boots" };
// Mail
static const char * mail_helm[] = { "coif", "crown", "hat" };
static const char * mail_shoulders[] = { "amice", "mantle", "shoulders" };
static const char * mail_chest[] = { "hauberk", "vest", "shirt" };
static const char * mail_gloves[] = { "gauntlets", "handguards", "grips" };
static const char * mail_pants[] = { "chausses", "leggings", "shorts" };
static const char * mail_boots[] = { "sandals", "slippers", "boots" };
// Plate
static const char * plate_helm[] = { "helm", "barbute", "hat" };
static const char * plate_shoulders[] = { "pauldrons", "mantle", "shoulders" };
static const char * plate_chest[] = { "breastplate", "vest", "cuirass" };
static const char * plate_gloves[] = { "gauntlets", "fists", "shorts" };
static const char * plate_pants[] = { "legplates", "leggings", "cuisses" };
static const char * plate_boots[] = { "sabatons", "greaves", "footguards" };
// Trinkets and rings
static const char * trinkets[] = { "pendant", "idol", "ankh" };
static const char * rings[] = { "ring", "band", "seal" };



int
main(int    argc   __attribute__((unused)),
     char * argv[] __attribute__((unused)))
{
    {
        pid_t  pid = getpid();
        time_t t = time(0);

        srand(pid * t);

        move_cursor(1, 1);
        del_eof();
    }

    size_t h_ini_lvl = 2;
    size_t e_ini_lvl = 1;

    hero_t hero = roll_hero("Tim the Enchanter", h_ini_lvl);

    print_portrait(&hero, PORTRAIT_ROW, PORTRAIT_COL);

    //size_t xp_req = 10;
    size_t xp_req = 1;

    for (;;) {
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
    if (mob == RANDOM_M) {
        mob = 1 + rand() % (DRAGON);
    }

    for (;;) {
        switch (mob) {
        case HUMANOID:
            return roll_humanoid(name, lvl);
        case ANIMAL:
            return roll_animal(name, lvl);
        case DRAGON:
            return roll_dragon(name, lvl);
        default:
            return roll_humanoid(name, lvl);
        }
    }
}



hero_t
roll_hero(const char * name,
          const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    h.attack = weapon_attack;
    h.spell = spell_enemy;
    h.heal = cure;

    h.level = lvl ? lvl : 1;

    gen_base_stats(&h);

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

    h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                   PIERCING);
    h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, OFF_HAND,
                                   PIERCING);

    gen_item_set(&h, h.level, COMMON, RANDOM_A);

    set_hp_mp(&h);

    return h;
}



hero_t
roll_humanoid(const char * name,
              const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    h.mob_type = HUMANOID;
    h.sub_type = rand() % (KNIGHT + 1);

    h.attack = weapon_attack;
    h.spell = spell_enemy;
    h.heal = cure;

    h.level = lvl ? lvl : 1;

    gen_base_stats(&h);

    if (name && *name) {
        strcpy(h.name, name);
    }
    else {
        switch (h.sub_type) {
        case BARBARIAN:
            strcpy(h.name, "barbarian");
            break;
        case SOLDIER:
            strcpy(h.name, "soldier");
            break;
        case PRIEST:
            strcpy(h.name, "priest");
            break;
        case WIZARD:
            strcpy(h.name, "wizard");
            break;
        case KNIGHT:
            strcpy(h.name, "knight");
            break;
        default:
            strcpy(h.name, "thief");
            break;
        }
    }

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        h.items[i].slot = NO_ITEM;
    }

    switch (h.sub_type) {
    case BARBARIAN:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     RANDOM_W);
        h.have_item[TWO_HAND] = 1;

        gen_item_set(&h, h.level, COMMON, LEATHER);

        break;

    case SOLDIER:
        h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                     RANDOM_W);
        h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 0, SHIELD, OFF_HAND,
                                     NO_WEAPON);
        h.have_item[MAIN_HAND] = 1;
        h.have_item[OFF_HAND] = 1;

        gen_item_set(&h, h.level, COMMON, MAIL);

        break;

    case WIZARD:
    case PRIEST:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     BLUNT);
        h.have_item[TWO_HAND] = 1;

        gen_item_set(&h, h.level, COMMON, CLOTH);

        break;

    case KNIGHT:
        h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                      RANDOM_W);
        h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 0, SHIELD, OFF_HAND,
                                     NO_WEAPON);
        h.have_item[MAIN_HAND] = 1;
        h.have_item[OFF_HAND] = 1;

        gen_item_set(&h, h.level, COMMON, PLATE);

        break;

    case THIEF:
    default:
        h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                     RANDOM_W);
        h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, OFF_HAND,
                                     PIERCING);

        h.have_item[MAIN_HAND] = 1;
        h.have_item[OFF_HAND] = 1;

        gen_item_set(&h, h.level, COMMON, LEATHER);

        break;

    }

    set_hp_mp(&h);

    return h;
}




hero_t
roll_animal(const char * name,
            const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    h.mob_type = ANIMAL;
    h.sub_type = rand() % (BEAR + 1);

    if (name && *name) {
        strcpy(h.name, name);
    }
    else {
        const size_t j = rand() % 5;
        switch (h.sub_type) {
        case DOG:
            strcpy(h.name, dog_list[j]);
            break;
        case CAT:
            strcpy(h.name, cat_list[j]);
            break;
        case BOAR:
            strcpy(h.name, boar_list[j]);
            break;
        case BEAR:
            strcpy(h.name, bear_list[j]);
            break;
        default:
        case FLYING:
            strcpy(h.name, flying_list[j]);
            break;
        }
    }

    h.attack = weapon_attack;

    h.level = lvl ? lvl : 1;

    gen_base_stats(&h);

    switch (h.sub_type) {
    case BEAR:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     BLUNT);

        gen_item_set(&h, h.level, COMMON, PLATE);

        break;

    case BOAR:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     EDGED);

        gen_item_set(&h, h.level, COMMON, MAIL);

        break;

    case DOG:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(&h, h.level, COMMON, LEATHER);

        break;

    case CAT:
    case FLYING:
    default:
        h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                       PIERCING);
        h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, OFF_HAND,
                                       PIERCING);

        gen_item_set(&h, h.level, COMMON, CLOTH);

        break;
    }

    set_hp_mp(&h);

    return h;
}



hero_t
roll_dragon(const char * name,
            const size_t lvl)
{
    hero_t h;

    memset(&h, 0, sizeof(h));

    size_t d_lvl = (lvl / 10) + 1;

    h.mob_type = DRAGON;
    h.sub_type = rand() % d_lvl;

    if (name && *name) {
        strcpy(h.name, name);
    }
    else {
        const size_t j = rand() % 5;
        switch (h.sub_type) {
        case FOREST_DRAGON:
            strcpy(h.name, forest_dragon_list[j]);
            break;
        case SAND_DRAGON:
            strcpy(h.name, sand_dragon_list[j]);
            break;
        case WATER_DRAGON:
            strcpy(h.name, water_dragon_list[j]);
            break;
        case FIRE_DRAGON:
            strcpy(h.name, fire_dragon_list[j]);
            break;
        default:
        case WHELPLING:
            strcpy(h.name, whelp_list[j]);
            break;
        }
    }

    h.attack = weapon_attack;

    h.level = lvl ? lvl : 1;

    gen_base_stats(&h);

    switch (h.sub_type) {
    case FOREST_DRAGON:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                      BLUNT);

        gen_item_set(&h, h.level, COMMON, LEATHER);

        break;

    case SAND_DRAGON:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     EDGED);

        gen_item_set(&h, h.level, COMMON, MAIL);

        break;

    case WATER_DRAGON:
        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(&h, h.level, COMMON, LEATHER);

        break;

    case FIRE_DRAGON:
        // Fire breathing dragon. The real deal.
        h.attack = breath;

        h.items[TWO_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, TWO_HAND,
                                     PIERCING);

        gen_item_set(&h, h.level, COMMON, PLATE);

        break;

    case WHELPLING:
    default:
        h.items[MAIN_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, MAIN_HAND,
                                       PIERCING);
        h.items[OFF_HAND] = gen_item(0, h.level, COMMON, 1, WEAPON, OFF_HAND,
                                       PIERCING);

        gen_item_set(&h, h.level, COMMON, CLOTH);

        break;
    }

    set_hp_mp(&h);

    return h;
}



void
gen_base_stats(hero_t * h)
{
    h->base.sta = h->level + BASE_STAT + (rand() % BASE_STAT_VAR);
    h->base.str = h->level + BASE_STAT + (rand() % BASE_STAT_VAR);
    h->base.agi = h->level + BASE_STAT + (rand() % BASE_STAT_VAR);
    h->base.wis = h->level + BASE_STAT + (rand() % BASE_STAT_VAR);
    h->base.spr = h->level + BASE_STAT + (rand() % BASE_STAT_VAR);

    return;
}



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
            armor_type = rand() % (MAX_ARMOR_TYPES + 1);
        }
    }

    if (slot == RANDOM_S) {
        // Determine slot from armor_type.
        switch (armor_type) {
        case WEAPON:
            // Any of 0-2 slot_t.
            slot = rand() % (TWO_HAND + 1);
            if (weapon_type == RANDOM_W) {
                weapon_type = rand() % (BLUNT + 1);
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
            slot = 3 + (rand() % (5 + 1));
            break;

        case MISC:
        default:
            // Any of 9-10 slot_t.
            slot = 9 + (rand() % (2));
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
        item.attr.sta = floor(sta_mult * tier_mult * (float) (rand() % (attr_lvl)));
        item.attr.str = floor(str_mult * tier_mult * (float) (rand() % (attr_lvl)));
        item.attr.agi = floor(agi_mult * tier_mult * (float) (rand() % (attr_lvl)));
        item.attr.wis = floor(wis_mult * tier_mult * (float) (rand() % (attr_lvl)));
        item.attr.spr = floor(spr_mult * tier_mult * (float) (rand() % (attr_lvl)));
    }

    if (level > 10) {
        item.resist.fire = floor(wis_mult * tier_mult * (float) (rand() % (res_lvl)));
        item.resist.frost = floor(wis_mult * tier_mult * (float) (rand() % (res_lvl)));
        item.resist.shadow = floor(wis_mult * tier_mult * (float) (rand() % (res_lvl)));
    }

    if (level > 15) {
        item.power.fire = floor(wis_mult * tier_mult * (float) (rand() % (pow_lvl)));
        item.power.frost = floor(wis_mult * tier_mult * (float) (rand() % (pow_lvl)));
        item.power.shadow = floor(wis_mult * tier_mult * (float) (rand() % (pow_lvl)));
    }

    if (level > procs_lvl) {
        // Do nothing for now...
    }

    return item;
}



tier_t
gen_item_tier(void)
{
    size_t trigger = rand() % 101;

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
    size_t j = rand() % NUM_ITEM_NAMES;

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
        i_armor_type = rand() % (PLATE + 1);
    }
    else {
        i_armor_type = armor_type;
    }

    h->items[HEAD] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, HEAD, 0);
    h->items[SHOULDERS] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, SHOULDERS, 0);
    h->items[CHEST] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, CHEST, 0);
    h->items[LEGS] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, LEGS, 0);
    h->items[HANDS] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, HANDS, 0);
    h->items[FEET] = gen_item(0, i_lvl, COMMON, 0, i_armor_type, FEET, 0);

    return;
}



void
spawn_item_drop(hero_t * h)
{
    // Anything from mana potions to armor, swords, etc.
    // Tiers of quality? Rare, epic, legendary?
    // Tier of quality should be influenced by tier of mob?
    size_t trigger = rand() % 100;

    if (trigger < ITEM_DROP_THRESH) {
        return;
    }

    // This feels goofy.
    size_t drop_type = rand() % 3;
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

    h->hp = HP_MULT * stamina;
    h->mp = MP_MULT * wisdom;

    return;
}



size_t
get_max_hp(const hero_t * h)
{
    size_t stamina = h->base.sta;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
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
        if (!h->have_item[i]) {
            // Nothing to do.
            continue;
        }

        wisdom += h->items[i].attr.wis;
    }

    return (MP_MULT * wisdom);
}



float
get_melee_dmg(const hero_t * h,
              const item_t * weapon,
              smear_t        smear_type)
{
    // Attack damage is
    //
    //   dmg = (S * str + A * agi) * (weapon multiplier)
    //
    // where the weapon multiplier is given by
    //
    //   2 handed = TWO_HAND_MULT
    //   1 handed = ONE_HAND_MULT (per hand)
    //   unarmed  = UNARMED_MULT
    //
    // and S, A are weapon dependent coefficients.

    float  mult = 0;
    float  S = 0.5;  // Component of damage from strength.
    float  A = 0.5;  // Component of damage from agility.

    // 2 hand blunt = 0.9 * strength + 0.1 * agility
    // 1 hand piercing = 0.1 * strength + 0.9 * agility

    switch (weapon->slot) {
    case MAIN_HAND:
        S -= 0.2;
        A += 0.2;

        mult = ONE_HAND_MULT;

        break;

    case OFF_HAND:
        S -= 0.2;
        A += 0.2;

        mult = ONE_HAND_MULT;

        break;

    case TWO_HAND:
        if (weapon->armor_type != WEAPON) {
            return 0;
        }

        S += 0.2;
        A -= 0.2;

        mult = TWO_HAND_MULT;

        break;

    default:
        mult = UNARMED_MULT;
    }

    switch (weapon->weapon_type) {
    case PIERCING:
        S -= 0.2;
        A += 0.2;
        break;

    case EDGED:
        // Edged benefits from strength and agility equally.
        break;

    case BLUNT:
        S += 0.2;
        A -= 0.2;
        break;

    default:
        // Unarmed. Do nothing.
        break;
    }

    float  dmg = 0;
    size_t agi = 0;
    size_t str = 0;

    str += h->base.str;
    agi += h->base.agi;

    for (size_t i = 0; i < MAX_ITEMS; ++i) {
        if (!h->have_item[i]) {
            // Nothing to calculate.
            continue;
        }

        str += h->items[i].attr.str;
        agi += h->items[i].attr.agi;
    }

    dmg = mult * ((S * str) + (A * agi));

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
    // where the spell multiplier is SPELL_DMG_MULT.
    //
    float dmg = 0;
    float wis = 0;
    float mult = SPELL_DMG_MULT;
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
weapon_attack(hero_t * hero,
              hero_t * enemy)
{
    // This have_item thing is annoying. Also, need
    // to print name,type of weapon on attack for debugging.
    size_t main_hand = hero->have_item[MAIN_HAND];
    size_t off_hand = hero->have_item[OFF_HAND];
    size_t two_hand = hero->have_item[TWO_HAND];

    {
        const item_t * weapon = &hero->items[MAIN_HAND];

        if (main_hand && weapon->armor_type == WEAPON) {
            attack_enemy(hero, enemy, weapon);
        }
        else if (!main_hand && !two_hand) {
            // Unarmed attack.
            attack_enemy(hero, enemy, weapon);
        }
        else {
            // Something in main hand that's not a weapon. Do nothing.
        }
    }

    {
        const item_t * weapon = &hero->items[OFF_HAND];

        if (off_hand && weapon->armor_type == WEAPON) {
            attack_enemy(hero, enemy, weapon);
        }
        else if (!off_hand && !two_hand) {
            // Unarmed attack.
            attack_enemy(hero, enemy, weapon);
        }
        else {
            // Something in off hand that's not a weapon. Do nothing.
        }
    }

    {
        const item_t * weapon = &hero->items[TWO_HAND];

        if (two_hand && weapon->armor_type == WEAPON) {
            attack_enemy(hero, enemy, weapon);
        }
        else {
            // Something in two hand that's not a weapon. Do nothing.
        }
    }


    return;
}



void
attack_enemy(hero_t *       hero,
             hero_t *       enemy,
             const item_t * weapon)
{
    {
        // Calculate dodge first. If attack misses, nothing left to do.
        float dodge = get_dodge(enemy);
        float trigger = rand() % 10000;

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
    size_t h_proc_sum[MAX_PROCS];
    size_t e_proc_sum[MAX_PROCS];

    memset(h_proc_sum, 0, sizeof(h_proc_sum));
    memset(e_proc_sum, 0, sizeof(e_proc_sum));

    sum_procs(h_proc_sum, hero);
    sum_procs(e_proc_sum, enemy);

    base_dmg = get_melee_dmg(hero, weapon, STD_SMEAR);

    {
        // Calculate crit. Reusing dodge for now.
        size_t crit = get_dodge(hero);
        size_t trigger = rand() % 10000;

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

END_ATTACK:
    ++row_;

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
        size_t trigger = rand() % 10000;

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

    return 1;
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
    // Breath cannot crit, but has a large variance.
    float base_dmg;
    float smear = 0.01 * (70 + (rand () % 61));

    base_dmg = smear * 2 * (hero->base.str + hero->base.wis);

    size_t hp_reduced = attack_barrier(base_dmg, enemy);

    printf("dragon breath burned %s for %zu hp damage\n",
           enemy->name, hp_reduced);

    ++row_;

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

        case 'i':
            choose_inventory(hero, 0);

            move_cursor(1, 1);
            del_eof();
            reset_cursor();

            print_portrait(hero, PORTRAIT_ROW, PORTRAIT_COL);
            print_portrait(enemy, PORTRAIT_ROW, PORTRAIT_COL + (2 * 32));


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
        sprintf_item_name(pretty_name, &h->items[i]);

        printf("  %s: \e[1;32m%s\e[0m (%s)\n", slot_to_str(i), pretty_name,
               armor_to_str(h->items[i].armor_type));
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

        h->have_item[MAIN_HAND] = 0;
        h->have_item[OFF_HAND] = 0;
    }
    else if (s_i_slot == MAIN_HAND || s_i_slot == OFF_HAND) {
        // Equipping one hand means de-equipping two hand.
        item_t * old_th = &h->items[TWO_HAND];

        old_th = add_to_inventory(h, old_th);

        if (old_th) {
            // Not enough space to dequip two hand.
            return;
        }

        h->have_item[TWO_HAND] = 0;
    }

    item_t old_item = h->items[s_i_slot];
    size_t have_old_item = h->have_item[s_i_slot];

    h->items[s_i_slot] = *s_i;
    h->have_item[s_i_slot] = 1;

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

    // Print the rest of the inventory.
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

    if (!item || item->slot == NO_ITEM) {
        // Nothing to print.
        return;
    }

    char   pretty_name[MAX_NAME_LEN + 1];

    sprintf_item_name(pretty_name, item);

    printf("\033[%zu;%zuH %s: %s", row + 2, col,
           slot_to_str(item->slot), pretty_name);

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
    printf("\033[%zu;%zuH u: use selected item", row + 6, col);
    printf("\033[%zu;%zuH q: quit", row + 7, col);

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

