#include <stdint.h>
#include <stdlib.h>

#include "hwsupport/stm_uart.h"
#include "kernel/noyau_prio.h"
#include "kernel/delay.h"
#include "io/serialio.h"
#include "io/TERMINAL.h"
#include "kernel/mutex.h"

uint8_t mutex;

typedef struct {
	uint16_t start_delay;
	int work_to_do;
} TACHE_PARAM;

TACHE tacheMutex(void *arg)
{
	TACHE_PARAM *p = (TACHE_PARAM *) arg;

	while(1) {
		delay(p->start_delay);
		m_acquire(mutex);
		for (volatile int i = 0; i < p->work_to_do; i++) continue;
		m_release(mutex);
	}
}

TACHE tacheAutre(void *arg)
{
	TACHE_PARAM *p = (TACHE_PARAM *) arg;

	while(1) {
		delay(p->start_delay);
		for (volatile int i = 0; i < p->work_to_do; i++) continue;
	}
}

TACHE tachedefond(void *arg)
{
	TACHE_PARAM params[3] = {{24, 200000000}, {28, 400000000}, {20, 800000000}};

	SET_CURSOR_POSITION(3,1);
	puts("------> EXEC tache de fond");

	active(cree(tacheMutex, 2, (void*) &params[0]));
	active(cree(tacheAutre, 4,  (void*) &params[1]));
	active(cree(tacheMutex, 6,  (void*) &params[2]));

	usart_read();
	noyau_exit();
}

int main()
{
	usart_init(115200);
	CLEAR_SCREEN(1);
    puts("Test noyau");
    puts("Noyau preemptif");
    m_init();
    mutex = m_create();
	start(tachedefond);
  return(0);
}
