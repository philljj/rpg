#if !defined(SAFER_RAND_H)
#define SAFER_RAND_H

#if !defined(UINT16_MAX)
#define UINT16_MAX (65535)
#endif /* if !defined(UINT16_MAX) */

#define MAX_RAND_NUM (UINT16_MAX)

void   rpg_rand_init(void);
size_t rpg_rand(const size_t min, const size_t max);
#endif /* if !defined(SAFER_RAND_H) */
