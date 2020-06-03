#if !defined(ITEM_STATS_H)
#define ITEM_STATS_H

#define MAX_NAME_LEN         (32)

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

#endif /* if !defined(ITEM_STATS_H) */
