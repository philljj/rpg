#if !defined(RPG_H)
#define RPG_H

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
#define ACTION_ROW           (11)
#define ACTION_COL           (4)
#define SELECT_ROW           (1)
#define SELECT_COL           (64)

// Misc
#define USLEEP_INTERVAL      (500000)

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

/* current classes ideas:
 *   Druid: Nature and Shadow damage
 *   Geomancer: Nature and Fire and Frost damage
 *   Necromancer: Shadow and Frost damage
 *   Priest: Holy damage
 *   Wizard: Fire, Frost, and non-elemental damage.
 *   Time Mage: Non-Elemental damage (meteor).
 *
 *
 *
 *
 * todo: a job system like this?
 *
 * t0.             Hunter             Squire           Chemist       Thief
 *                  /   \               /                  |          |  \
 * t1.   (Cleric)  /     |       ---- Knight        --- Cleric ---    |   \   (Knight)
 *            \   /      |     /     /       \    /       |        \  |    \   /
 * t2.        Druid   Geomancr  Berserkr  MysticKnt   WhiteMage    Monk    Ninja
 *
 *
 *
 * squire: 1h-2h, mail. Well rounded. Folk medicine heal ability..
 * knight: 1h-2h, plate. Highest potential attack and defense.
 * barbarian: 2h weapon, leather, debuffs enemy (demo shout, fear, etc)
 * mystic knight: 1h-2h, mail, enchants sword.
 *
 * chemist: 1h, leather. use/throw/mix items.
 * cleric: 1h, leather. party buffer. pray (small aoe heal and buff)
 *
 * thief: 1h, leather. Steal Gold.
 * ninja: 2 swords, leather. Unlocks 2 swords passive.
 *
 *
 *
 *
 *
 * Squire:
 *
 */

typedef enum {
    SQUIRE  =  0,
    CHEMIST =  1,
    THIEF   =  2,
    CLERIC  =  3,
    KNIGHT  =  4,
} job_t;

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


typedef void (*rpg_attack_func_t)(void * hero, void * enemy);
/* These callbacks were ultimately not that useful...
 * Might be more usefult to have attack, primary, secondary callbacks,
 * which are set when you choose your job.
 *
 *   typedef size_t (*rpg_spell_func_t)(void * hero, void * enemy,
 *                                      const element_t elem,
 *                                      const float dmg_mult,
 *                                      const float mp_mult);
 *   typedef void   (*rpg_defend_func_t)(void * hero);
 *   typedef size_t (*rpg_heal_func_t)(void * hero, const float dmg_mult,
 *                                  const float mp_mult);
 */

struct job_skill_t {
    job_t             job;       // CLERIC
    short             color;     // COLOR_CYAN
    char              name[16];  // "pray"
    rpg_attack_func_t skill_cb;  // cleric_pray()
    size_t            job_level; // determines future job/skill unlocks
    size_t            jp;        // job points
    size_t            jp_req;    // job points
    // something to track skill unlocks?
};

typedef struct job_skill_t job_skill_t;

struct hero_t {
    // Common fields.
    char     name[MAX_NAME_LEN + 1]; // There are Some who call me Tim.
    mob_t    mob_type;
    int      sub_type;               // sub_type of mob.
    size_t   level;
    size_t   gold;
    size_t   hp;                     // Health Points.
    size_t   mp;                     // Mana Points.
    size_t   bp;                     // Barrier Points.
    size_t   xp;                     // Experience Points.
    size_t   xp_req;                 // XP required to level up
    size_t   xp_rew;                 // XP rewarded for defeating this mob.
    size_t   armor;                  // Mitigates physical damage.
    stats_t  base;                   // Base attributes. Increases with level.
    spell_t  power;                  // Enhances spell damage given.
    spell_t  resist;                 // Resists spell damage received.
    // Cooldowns, buffs, debuffs.
    cd_t     cooldowns[MAX_COOLDOWNS];
    debuff_t debuffs[MAX_DEBUFFS];
    // Equipment and inventory.
    item_t   items[MAX_ITEMS];       // Equipped items.
    item_t   inventory[MAX_INVENTORY];
    // Callbacks.
    rpg_attack_func_t attack;
    rpg_attack_func_t job_primary;
    rpg_attack_func_t job_secondary;
    job_skill_t       skill_primary;
    job_skill_t       skill_second;
    // reaction callback? counterattack, etc
    // movement skill
    // support skill
    /* See callback note above.
     *   rpg_spell_func_t  spell;
     *   rpg_defend_func_t defend;
     *   rpg_heal_func_t   heal;
     */
};

typedef struct hero_t hero_t;

// Basic mob gen functions.
hero_t * rpg_roll_player(hero_t * h, const size_t lvl);
hero_t * rpg_roll_mob(hero_t * h, const char * name, const size_t lvl, mob_t m);
hero_t * rpg_roll_humanoid(hero_t * hero, const char * name, const size_t lvl);
hero_t * rpg_roll_animal(hero_t * hero, const char * name, const size_t lvl);
hero_t * rpg_roll_dragon(hero_t * hero, const char * name, const size_t lvl);
void     rpg_gen_base_stats(hero_t * h);

stats_t get_total_stats(const hero_t * h);

void   level_up(hero_t * h);
void   set_hp_mp_bp(hero_t * h);
size_t get_max_hp(const hero_t * h);
size_t get_max_mp(const hero_t * h);

// Print functions.
const char * slot_to_str(slot_t s);
const char * armor_to_str(armor_t a);
const char * mob_to_str(const mob_t m);

// Combat functions.
void   decision_loop(hero_t * hero, hero_t * enemy);
void   spirit_regen(hero_t * h);
size_t choose_attack(hero_t * hero, hero_t * enemy);
size_t choose_spell(hero_t * hero, hero_t * enemy);
void   battle(hero_t * hero, hero_t * enemy);

size_t back_stab(hero_t * hero, hero_t * enemy);
size_t crushing_blow(hero_t * hero, hero_t * enemy);
size_t drain_touch(hero_t * hero, hero_t * enemy);
size_t holy_smite(hero_t * hero, hero_t * enemy);
size_t shield_bash(hero_t * hero, hero_t * enemy);
/* stackable druid nature debuff */
size_t insect_swarm(hero_t * hero, hero_t * enemy);

void chemist_item(void * h, void * e);
void squire_skills(void * h, void * e);
void thief_skills(void * h, void * e);
void druid_skills(void * h, void * e);


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
size_t fireball(hero_t * hero, hero_t * enemy);
size_t attack_barrier(size_t final_dmg, hero_t * enemy);
size_t time_mage_regen(hero_t * h);
size_t restore_hp(hero_t * h, const size_t amnt);
size_t restore_mp(hero_t * h, const size_t amnt);
size_t spend_hp(hero_t * h, const size_t amnt);
size_t spend_mp(hero_t * h, const size_t amnt);

#endif /* if !defined(RPG_H) */
