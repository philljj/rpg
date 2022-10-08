#if !defined(TUI_H)
#define TUI_H
void rpg_tui_print_portrait(hero_t * h, const size_t i, const size_t j);
void rpg_tui_move_cursor(const size_t i, const size_t j);
void rpg_tui_reset_cursor(void);
void rpg_tui_set_cursor(void);
void rpg_tui_del_line(void);
void rpg_tui_del_eof(void);
void rpg_tui_clear_screen(void);
void rpg_tui_print_act_prompt(const hero_t * h);
void rpg_tui_clear_act_prompt(const hero_t * h);
void rpg_tui_print_attack_prompt(const hero_t * h);
void rpg_tui_clear_attack_prompt(const hero_t * h);
void rpg_tui_print_spell_prompt(const hero_t * h);
void rpg_tui_clear_spell_prompt(const hero_t * h);
void rpg_tui_print_heal_prompt(const hero_t * h);
void rpg_tui_clear_heal_prompt(const hero_t * h);
void rpg_tui_increment_row(void);
void rpg_tui_check_row(void);
void rpg_tui_reset_row(void);
void rpg_tui_print_inventory_prompt(void);
char rpg_tui_safer_fgetc(void);
void rpg_tui_clear_stdin(void);
void rpg_tui_print_hero(hero_t * h, const size_t verbosity);
void rpg_tui_print_equip(hero_t * h);
#endif /* if !defined(TUI_H) */
