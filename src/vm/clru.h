#ifndef CLRU_H
#define CLRU_H

/* Inits policy. */
void clru_init(void);

void clru_enqueue(uint32_t fn);
uint32_t clru_evict(void);

void clru_timer_tick(void);

void set_bits(void);

#endif