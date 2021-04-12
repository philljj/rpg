#if !defined(COMBAT_H)
#define COMBAT_H

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

typedef enum {
    NO_SMEAR,
    STD_SMEAR
} smear_t;

size_t get_spell_cost(const element_t element, const float mp_mult);

float  get_melee_dmg(const hero_t * h, const item_t * weapon,
                     smear_t smear_type);
float  get_spell_dmg(const hero_t * h, const element_t element,
                     smear_t smear_type);
float  get_spell_res(const hero_t * h, const element_t element);
float  get_elem_pow(const spell_t * power, const element_t element);
float  get_elem_res(const spell_t * power, const element_t element);
float  get_mitigation(const hero_t * h);
float  get_mitigation_w_bypass(const hero_t * h, const float bypass);
size_t get_armor(const hero_t * h);
float  get_resist(const hero_t * h);
float  get_dodge(const hero_t * h);
size_t get_total_stat(const hero_t * h, stat_type_t stat_type);

#endif /* if !defined(COMBAT_H) */
