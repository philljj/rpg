#if !defined(ABILITY_CB_H)
#define ABILITY_CB_H
void   weapon_attack_cb(void * hero, void * enemy);
void   dragon_breath_cb(void * h, void * e);
size_t spell_attack_cb(void * h, void * e, const element_t element,
                      const float dmg_mult, const float mp_mult);
size_t spell_heal_cb(void * h, const float dmg_mult,
                    const float mp_mult);
#endif
