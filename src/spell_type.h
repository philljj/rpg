#if !defined (SPELL_TYPE_H)
#define SPELL_TYPE_H

// Spell schools.
typedef enum {
    FIRE,
    FROST,
    SHADOW,
    NON_ELEM,
    RESTORATION
} element_t;

// The cooldown_t enum will index into the cooldowns array.
// Anything over MAX_COOLDOWNS is not implemented.
#define MAX_COOLDOWNS (10)
typedef enum {
    // Beginner cooldown abilities.
    BACK_STAB     = 0,  // Triple damage main hand dagger strike. No dodge.
    CRUSHING_BLOW = 1,  // Double damage two hand strike.
    SHIELD_BASH   = 2,  // Bash for X damage and stun for one round.
    HOLY_SMITE    = 3,  // Holy dmg equal to caster's spirit.
    INSECT_SWARM  = 4,  // Nature DoT, reduces enemy chance to hit.
    FIREBALL      = 5,  // Fire spell for %150 spell dmg, plus 50% over 3 rounds.
    DRAIN_TOUCH   = 6,  // Weapon attack and heal for 50% of dmg.
    SHIELD_WALL   = 7,  // Reduces all damage by 90% for 3 rounds.
    ELEMENTAL     = 8,  // Geomancer elemental attack.
    REGEN         = 9,  // Heals for 100% of health over 3 rounds. No mana cost.
    // Novice.
    METEOR        = 25, // Heavy non-elemental damage over 3 rounds.
} cooldown_t;

// Status type of debuff.
// Anything over MAX_DEBUFFS is not implemented.
#define MAX_DEBUFFS (32)
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


#endif /* if !defined (SPELL_TYPE_H) */
