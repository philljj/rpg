#if !defined(RPG_H)
#define RPG_H

#include <stdint.h>

// Geomancer and druid will use these flags?...
// Interact with them differently?

typedef uint32_t weather_flags_t;

#define WF_NONE = 0x00000000u; // clear, neutral temp.
#define WF_WIND = 0x00000001u;
#define WF_MURK = 0x00000002u; // foggy
#define WF_RAIN = 0x00000004u;
#define WF_SNOW = 0x00000008u;
#define WF_WARM = 0x00000010u; // On warm, off cold.
#define WF_EXTR = 0x00000020u; // On extreme weather.

typedef uint32_t geography_flags_t;

#define GEO_NONE     = 0x00000000u; // flat plains, neutral everything.
#define GEO_OCEAN    = 0x00000001u;
#define GEO_RIVER    = 0x00000002u;
#define GEO_LAKE     = 0x00000004u;
#define GEO_FOREST   = 0x00000008u; // On desert, off normal.
#define GEO_DESERT   = 0x00000010u; // On desert, off normal.
#define GEO_ALTITUDE = 0x00000020u; // On high altitude, off low.
#define GEO_ALPINE   = 0x00000040u; // On mountainous.

typedef uint32_t seasons_flags_t;

#define SEASONS_NONE   = 0x00000000u;
#define SEASONS_SPRING = 0x00000001u; // On spring, off fall.
#define SEASONS_SUMMER = 0x00000002u; // On summer, off winter.

// Names and items.
#define MAX_NAME_LEN         (32)
#define ITEM_DROP_THRESH     (10)  // 100 minus this number is drop rate.
#define MAX_ITEMS            (11)  // Max item slots on hero.
#define MAX_INVENTORY        (32)  // Max bag space slots on hero.

// Combat multipliers
#define HP_MULT              (4)   // Max health points = 4 * stamina.
#define MP_MULT              (4)   // Max mana points = 4 * wisdom.
#define SPELL_DMG_MULT       (0.8)
#define TWO_HAND_MULT        (0.8)
#define ONE_HAND_MULT        (0.3)
#define UNARMED_MULT         (0.25)
#define ARMOR_HALF_POINT     (200) // Armor required for 50% melee reduction.
#define DODGE_HALF_POINT     (500) // Agility required for 50% dodge.
#define SPIRIT_REGEN_MULT    (0.5)
#define REGEN_ROUND          (4)
#define BASE_STAT            (7)
#define BASE_STAT_VAR        (7)

// TUI display
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

// Misc
#define USLEEP_INTERVAL      (500000)
#define RAND_BUF_LEN         (6)
#define MAX_RAND_NUM         (65535)

// Quality of item drop and difficulty of mobs.
typedef enum {
    COMMON      = 0,
    GOOD        = 1,
    RARE        = 2,
    EPIC        = 3,
    RANDOM_TIER = 99
} tier_t;

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
    BP_POTION = 22,
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

// Spell schools.
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

// Fundamental monster types, and their various sub types.
typedef enum {
    HERO      = 0,
    HUMANOID  = 1,
    ANIMAL    = 2,
    UNDEAD    = 3,
    DRAGON    = 5,
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
    NECROMANCER     = 6, // cloth, leather, mail, shadow life drain
    KNIGHT          = 7, // plate, shield, one hand, or two hand
    GEOMANCER       = 8, // mail, weapon imbues
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

#if 0
typedef enum {
    // Beginner passives, unlocked at class selection.
    QUICKNESS      = 0,  // t, +10% agility
    PRIMAL_MIGHT   = 1,  // b, +10% strength
    DEFTNESS       = 2,  // s, +10% one hand weapon dmg.
    DIVINITY       = 3,  // p, +20% to all healing effects
    NATURES_WRATH  = 4,  // d, melee attacks 10% change to trigger 50% nature dmg.
    INSIGHT        = 5,  // w, +10% wisdom
    NECROMANCER    = 6,  // n, +10% shadow dmg
    DEFLECTION     = 7,  // k, block and parry % from strength
    IMBUED_WEAPONS = 8,  // g, weapon attacks deal additional 10% non-elem dmg.
    // Novice.
    FLURRY         = 22, // s, 10% chance to trigger 2nd melee, can double proc.
    FEL_TOUCH      = 26, // n, All attacks do 4% drain life.
} passives_t;
#endif

#define MAX_COOLDOWNS (9)
typedef enum {
    // Beginner cooldown abilities.
    BACK_STAB     = 0,  // Triple damage main hand dagger strike. No dodge.
    CRUSHING_BLOW = 1,  // Double damage two hand strike.
    SHIELD_BASH   = 2,  // Bash for X damage and stun for one round.
    DIVINE_HEAL   = 3,  // Heals for 100% of health over 3 rounds. No mana cost.
    SHAPESHIFT    = 4,  // Unlocks bear and cat forms.
    FIREBALL      = 5,  // Fire spell for %150 spell damage, plus 50% over 3 rounds.
    FEL_STRIKE    = 6,  // Weapon attack for X damage and heal for X.
    SHIELD_WALL   = 7,  // Reduces all damage by 90% for 3 rounds.
    ROCK_WEAPON   = 8,  // Weapon attacks also deal 50% non-elem dmg for 4 rounds.
    // Novice.
    METEOR        = 25, // Heavy non-elemental damage over 3 rounds.
} cooldown_t;

#define MAX_NON_COOLDOWNS (9)
typedef enum {
    // Beginner non-cooldown abilities.
    EVASION      = 0, // Increases dodge chance by 40%, mana cost.
    ENRAGE       = 1, // Increases damage given by 30%, taken by 20%.
    BREAK_ARMOR  = 2, // Damage for X and reduces armor by 20% for 4 rounds.
    BARRIER      = 3, // Barrier that absorbs all damage, mana cost, non stacking.
    STARFIRE     = 4, // Non-elem spell damage. Mana cost.
    BLIZZARD     = 5, // Heavy frost damage over 3 rounds.
    CURSE        = 6, // Spell damage over time, mana cost, non stacking.
    POWER_STRIKE = 7, // Powerful strike for 1.5 weapon attack damage.
    EARTH_SPRING = 8, // Restores X health for 10 rounds. Stacks 3 times.
} non_cooldown_t;

// Status type of debuff.
typedef enum {
    // Debuffs.
    DOT           = 0,  // Damage Over Time. Burn, curse, bleed, etc.
    STUN          = 1,  // Incapacitated completely.
    SILENCE       = 2,  // No spellcasting.
    DISARM        = 3,  // Disarms two hand or main hand.
    SLOW          = 4,  // Reduces agility.
    WEAKEN        = 5,  // Reduces strength.
    SLEEP         = 6,  // Incapacitated, but any damage will awaken.
    // Buffs.
    HOT           = 30, // Heal Over Time.
    HASTE         = 34, // Increases agility.
    MIGHT         = 35, // Increases strength.
    RANDOM_STATUS = 99
} db_status_t;

typedef struct {
    size_t sta;  // HP
    size_t str;  // weapon damage (favors blunt and two hand)
    size_t agi;  // dodge, weapon damage (favors piercing and one hand)
    size_t wis;  // MP, spell damage
    size_t spr;  // HP and MP regen
} stats_t;

typedef struct {
    size_t fire;
    size_t frost;
    size_t shadow;
    size_t non_elemental;
    size_t restoration;
} spell_t;

typedef struct {
    // Generic item struct.
    char     name[MAX_NAME_LEN + 1];
    slot_t   slot;
    armor_t  armor_type;
    weapon_t weapon_type;
    tier_t   tier;
    size_t   armor;
    stats_t  attr;
    spell_t  power;
    spell_t  resist;
} item_t;

typedef struct {
    // Generic debuff type. Examples:
    //   X element damage for Y rounds.
    //   Stun for Y rounds.
    //   Silence spell casting for Y rounds.
    char        name[MAX_NAME_LEN + 1];
    db_status_t status;
    element_t   element;
    float       amnt;
    size_t      rounds; // Active rounds remaining. Debuff ends at 0.
    size_t      wait; // Rounds to wait before debuff becomes active.
} debuff_t;

typedef struct {
    // Manages if special ability is unlocked, and cooldown.
    size_t unlocked;
    size_t rounds;
} cd_t;

typedef struct {
    // Manages if special ability is unlocked.
    size_t unlocked;
} non_cd_t;

#define MAX_DEBUFFS (32)

typedef void   (*rpg_attack_func_t)(void * hero, void * enemy);
typedef size_t (*rpg_spell_func_t)(void * hero, void * enemy,
                                   const element_t elem,
                                   const float dmg_mult,
                                   const float mp_mult);
typedef void   (*rpg_defend_func_t)(void * hero);
typedef size_t (*rpg_heal_func_t)(void * hero, const float dmg_mult,
                                  const float mp_mult);

typedef struct {
    // Common fields.
    char     name[MAX_NAME_LEN + 1]; // There are Some who call me Tim.
    mob_t    mob_type;
    int      sub_type;               // sub_type of mob.
    size_t   level;
    size_t   hp;                     // Health Points.
    size_t   mp;                     // Mana Points.
    size_t   bp;                     // Barrier Points.
    size_t   xp;                     // Experience Points.
    size_t   armor;                  // Mitigates physical damage.
    stats_t  base;                   // Base attributes. Increases with level.
    spell_t  power;                  // Enhances spell damage given.
    spell_t  resist;                 // Resists spell damage received.
    // Cooldowns, buffs, debuffs.
    cd_t     cooldowns[MAX_COOLDOWNS];
    non_cd_t non_cooldowns[MAX_NON_COOLDOWNS];
    debuff_t debuffs[MAX_DEBUFFS];
    // Equipment and inventory.
    item_t   items[MAX_ITEMS];       // Equipped items.
    item_t   inventory[MAX_INVENTORY];
    // Callbacks.
    rpg_attack_func_t attack;
    rpg_spell_func_t  spell;
    rpg_defend_func_t defend;
    rpg_heal_func_t   heal;
} hero_t;

// Basic mob gen functions.
hero_t roll_hero(const size_t lvl);
hero_t roll_mob(const char * name, const size_t lvl, mob_t mob);
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
char         safer_fgetc(void);
void         clear_stdin(void);
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
const char * mob_to_str(const mob_t m);
const char * elem_to_str(const element_t elem);


// Combat functions.
void   decision_loop(hero_t * hero, hero_t * enemy);
size_t choose_attack(hero_t * hero, hero_t * enemy);
size_t choose_spell(hero_t * hero, hero_t * enemy);
size_t choose_heal(hero_t * hero);
void   battle(hero_t * hero, hero_t * enemy);

// Baseline Abilities:
//     melee attack.
void   weapon_attack_cb(void * hero, void * enemy);
void   weapon_attack_i(hero_t * hero, hero_t * enemy, const item_t * weapon);
void   dragon_breath_cb(void * h, void * e);
//     spell attack.
size_t spell_attack_cb(void * h, void * e, const element_t element,
                      const float dmg_mult, const float mp_mult);
//     spell heal.
size_t spell_heal_cb(void * h, const float dmg_mult,
                    const float mp_mult);
size_t divine_heal(hero_t * hero);

size_t back_stab(hero_t * hero, hero_t * enemy);
size_t crushing_blow(hero_t * hero, hero_t * enemy);
size_t shield_bash(hero_t * hero, hero_t * enemy);

// Debuffs and cooldowns.
void   apply_debuff(hero_t * enemy, const char * name,
                    const db_status_t status, const element_t element,
                    const float amnt, const size_t rounds,
                    const size_t delay);
size_t process_debuffs(hero_t * enemy);
void   process_debuffs_i(hero_t * enemy, debuff_t * debuff);
void   clear_debuffs(hero_t * enemy);
void   process_cooldowns(hero_t * h);
void   reset_cooldowns(hero_t * h);

// Spell abilities.
size_t fire_strike(hero_t * hero, hero_t * enemy);
size_t shadow_bolt(hero_t * hero, hero_t * enemy);
size_t fireball(hero_t * hero, hero_t * enemy);

size_t get_spell_cost(const element_t element, const float mp_mult);

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

// Prompts.
static const char * action_prompt = "\n"
                                    "  choose action:\n"
                                    "    a: attack\n"
                                    "    s: spell\n"
                                    "    d: defend\n"
                                    "    h: heal\n"
                                    "    i: inventory\n";

static const char * spell_prompt = "\n"
                                   "  choose spell:\n"
                                   "    f: fire strike\n"
                                   "    i: ice\n"
                                   "    s: shadow bolt\n"
                                   "    u: non-elemental\n";

#define NUM_MOB_SUB_TYPES (4)
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

#define MAX_PREFIX (116)
static const char * prefix_list[MAX_PREFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu",//50
    "ag", "bag", "cag", "dag", "mag",
    "al", "el", "il", "ol", "ul",
    "isil", "ithil", "igil", "iril", "imil",
    "aeon", "feon", "heon", "leon", "theon",
    "ainar", "finar", "minar", "thinar", "sinar",
    "mith", "minith", "minas", "milith", "mae",
    "gala", "galad", "gal", "galat", "galag",
    "bala", "balad", "bal", "balat", "balag",
    "rha", "rhe", "rhi", "rho", "rhith",
    "cele", "celem", "curu", "cara", "cura",//100
    "ind", "im", "idril", "inglor", "irime",
    "tha", "the", "tho", "thi", "thu",
    "tham", "than", "thath", "thon", "thoth",
    "thal"
};

#define MAX_SUFFIX (105)
static const char * suffix_list[MAX_SUFFIX] = {
    "ab", "ae", "ag", "am", "an", "ba", "be", "bi", "bo", "bu",
    "ca", "ce", "ci", "co", "cu", "da", "de", "di", "do", "du",
    "fa", "fe", "fi", "fo", "fu", "ga", "ge", "gi", "go", "gu",
    "ha", "he", "hi", "ho", "hu", "ma", "me", "mi", "mo", "mu",
    "sa", "se", "si", "so", "su", "ta", "te", "ti", "to", "tu",//50
    "amm", "ath", "ass", "agg", "all",
    "emm", "eth", "ess", "egg", "ell",
    "imm", "ith", "iss", "igg", "ill",
    "omm", "oth", "oss", "ogg", "oll",
    "umm", "uth", "uss", "ugg", "ull",
    "dur", "bur", "gur", "thur", "nur",
    "endil", "andil", "indil", "ondil", "undil",
    "thig", "thim", "thin", "thir", "this",
    "ain", "din", "fin", "gin", "lin",
    "aith", "fith", "thith", "nith", "sith",//100
    "aeth", "feth", "theth", "neth", "seth"
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



#endif /* if !defined(RPG_H) */
