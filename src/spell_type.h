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

#define MAX_COOLDOWNS (10)
typedef enum {
    // Beginner cooldown abilities.
    BACK_STAB     = 0,  // Triple damage main hand dagger strike. No dodge.
    CRUSHING_BLOW = 1,  // Double damage two hand strike.
    SHIELD_BASH   = 2,  // Bash for X damage and stun for one round.
    HOLY_SMITE    = 3,  // Holy dmg equal to caster's spirit.
    SHAPESHIFT    = 4,  // Unlocks bear and cat forms.
    FIREBALL      = 5,  // Fire spell for %150 spell dmg, plus 50% over 3 rounds.
    DRAIN_TOUCH   = 6,  // Weapon attack and heal for 50% of dmg.
    SHIELD_WALL   = 7,  // Reduces all damage by 90% for 3 rounds.
    ROCK_WEAPON   = 8,  // Weapon attacks also deal 50% non-elem dmg for 4 rounds.
    REGEN         = 9,  // Heals for 100% of health over 3 rounds. No mana cost.
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

#endif /* if !defined (SPELL_TYPE_H) */
