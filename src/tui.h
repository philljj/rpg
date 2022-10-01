#if !defined(TUI_H)
#define TUI_H
void print_portrait(hero_t * h, const size_t i, const size_t j);
void move_cursor(const size_t i, const size_t j);
void reset_cursor(void);
void set_cursor(void);
void del_line(void);
void del_eof(void);
void clear_screen(void);
void print_act_prompt(const hero_t * h);
void clear_act_prompt(const hero_t * h);
void print_attack_prompt(const hero_t * h);
void clear_attack_prompt(const hero_t * h);
void print_spell_prompt(const hero_t * h);
void clear_spell_prompt(const hero_t * h);
void print_heal_prompt(const hero_t * h);
void clear_heal_prompt(const hero_t * h);
void increment_row(void);
void check_row(void);
void reset_row(void);
#endif /* if !defined(TUI_H) */
