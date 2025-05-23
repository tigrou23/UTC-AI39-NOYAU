#ifndef _MUTEX_H
#define _MUTEX_H

#include <stdint.h>

#define MAX_MUTEX 8
#define MAX_TACHES        16
#define MUTEX_INVALID 255

void m_init(void);
uint8_t m_create(void);
void m_acquire(uint8_t m);
void m_release(uint8_t m);
void m_destroy(uint8_t m);

#endif
