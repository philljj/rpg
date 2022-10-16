#if !defined(TUI_H)
#define TUI_H
void rpg_tui_init(void);
void rpg_tui_print_portrait(hero_t * h, const size_t i, const size_t j);
void rpg_tui_move_cursor(const size_t i, const size_t j);
void rpg_tui_del_eof(void);
void rpg_tui_clear_screen(void);
void rpg_tui_print_act_prompt(const hero_t * h, size_t i, size_t j);
void rpg_tui_clear_act_prompt(const hero_t * h, size_t i, size_t j);
void rpg_tui_print_combat_txt(const char * msg);
void rpg_tui_print_combat_color_txt(const char * prefix, const element_t elem,
                                    const char * postfix);
void rpg_tui_increment_row(void);
void rpg_tui_check_row(void);
void rpg_tui_reset_row(void);
char rpg_tui_getch(void);
void rpg_tui_print_hero(hero_t * h, const size_t verbosity);
#endif /* if !defined(TUI_H) */
