#if !defined(ITEM_H)
#define ITEM_H

void   spawn_item_drop(hero_t * h);
item_t gen_item(const char * name, const size_t level, tier_t tier,
                size_t is_weapon, armor_t armor_type, slot_t slot,
                weapon_t weapon_type);
void   gen_item_set(hero_t * h, const size_t lvl,
                    const tier_t tier, const armor_t armor_type);
#endif /* if !defined(ITEM_H) */
