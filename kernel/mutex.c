#include "mutex.h"
#include "noyau_prio.h"

typedef struct {
    int8_t ref_count;       // -1 = libre, >=0 = nb d'acquisitions
    uint8_t owner;          // id tâche qui détient le mutex
    uint8_t attente[MAX_TACHES];  // file FIFO d'attente
    uint8_t debut;
    uint8_t fin;
} MUTEX;

static MUTEX mutexes[MAX_MUTEX];

void m_init(void) {
    for (int i = 0; i < MAX_MUTEX; i++) {
        mutexes[i].ref_count = -1;
        mutexes[i].owner = MUTEX_INVALID;
        mutexes[i].debut = 0;
        mutexes[i].fin = 0;
    }
}

uint8_t m_create(void) {
    for (int i = 0; i < MAX_MUTEX; i++) {
        if (mutexes[i].ref_count == -1) {
            mutexes[i].ref_count = 0;
            mutexes[i].owner = MUTEX_INVALID;
            return i;
        }
    }
    return MAX_MUTEX;
}

void m_acquire(uint8_t m) {
    if (m >= MAX_MUTEX || mutexes[m].ref_count == -1) {
        noyau_exit();  // mutex invalide
    }

    MUTEX *mu = &mutexes[m];
    uint8_t tid = noyau_get_tc();

    if (mu->ref_count == 0) {
        mu->ref_count = 1;
        mu->owner = tid;
    } else if (mu->owner == tid) {
        mu->ref_count++;
    } else {
        // Héritage de priorité
        NOYAU_TCB* tcb_tid = noyau_get_p_tcb(tid);
        NOYAU_TCB* tcb_owner = noyau_get_p_tcb(mu->owner);

        if (tcb_tid->priorite < tcb_owner->priorite) {
            tcb_owner->priorite = tcb_tid->priorite;
        }

        mu->attente[mu->fin++] = tid;
        dort();
        schedule();
    }
}

void m_release(uint8_t m) {
    if (m >= MAX_MUTEX || mutexes[m].ref_count <= 0 || mutexes[m].owner != noyau_get_tc()) {
        noyau_exit();  // erreur : tentative de libération invalide
    }

    MUTEX *mu = &mutexes[m];
    uint8_t tid = noyau_get_tc();

    mu->ref_count--;

    if (mu->ref_count == 0) {
        if (mu->debut < mu->fin) {
            uint8_t next_tid = mu->attente[mu->debut++];
            mu->owner = next_tid;
            mu->ref_count = 1;
            reveille(next_tid);
        } else {
            mu->owner = MUTEX_INVALID;
        }

        // Restaurer la priorité de base
        NOYAU_TCB* tcb = noyau_get_p_tcb(tid);
        tcb->priorite = tcb->priorite_base;
    }
}

void m_destroy(uint8_t m) {
    if (m >= MAX_MUTEX || mutexes[m].ref_count != 0) {
        noyau_exit();  // mutex invalide ou encore détenu
    }
    mutexes[m].ref_count = -1;
    mutexes[m].owner = MUTEX_INVALID;
    mutexes[m].debut = mutexes[m].fin = 0;
}
