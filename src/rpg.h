#if !defined(RPG_H)
#define RPG_H

// RNG functions.
void   init_rand(void);
size_t safer_rand(const size_t min, const size_t max);



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
#define MAX_ITEMS            (11)  // Max item slots on hero.
#define MAX_INVENTORY        (32)  // Max bag space slots on hero.

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


typedef void   (*rpg_attack_func_t)(void * hero, void * enemy);
typedef size_t (*rpg_spell_func_t)(void * hero, void * enemy,
                                   const element_t elem,
                                   const float dmg_mult,
                                   const float mp_mult);
typedef void   (*rpg_defend_func_t)(void * hero);
typedef size_t (*rpg_heal_func_t)(void * hero, const float dmg_mult,
                                  const float mp_mult);

struct hero_t {
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
};

typedef struct hero_t hero_t;

// Basic mob gen functions.
hero_t roll_hero(const size_t lvl);
hero_t roll_mob(const char * name, const size_t lvl, mob_t mob);
hero_t roll_humanoid(const char * name, const size_t lvl);
hero_t roll_animal(const char * name, const size_t lvl);
hero_t roll_dragon(const char * name, const size_t lvl);
void   gen_base_stats(hero_t * h);


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
size_t attack_barrier(size_t final_dmg, hero_t * enemy);
void   regen(hero_t * h);
size_t restore_hp(hero_t * h, const size_t amnt);
size_t restore_mp(hero_t * h, const size_t amnt);
size_t spend_hp(hero_t * h, const size_t amnt);
size_t spend_mp(hero_t * h, const size_t amnt);


#endif /* if !defined(RPG_H) */
