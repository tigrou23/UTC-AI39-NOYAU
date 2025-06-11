#include "mutex.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"

typedef struct {
    int8_t ref_count;
    uint8_t owner;
    uint8_t attente[MAX_TACHES];
    uint8_t debut;
    uint8_t fin;
} MUTEX;

static MUTEX mutexes[MAX_MUTEX];

void m_init(void) {
    for (int i = 0; i < MAX_MUTEX; i++) {
        mutexes[i].ref_count = -1;
        mutexes[i].owner = MUTEX_INVALID;
        mutexes[i].debut = mutexes[i].fin = 0;
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
        noyau_exit();
    }

    MUTEX *mu = &mutexes[m];
    uint8_t tid = noyau_get_tc();

    if (mu->ref_count == 0) {
        mu->ref_count = 1;
        mu->owner = tid;
    } else if (mu->owner == tid) {
        mu->ref_count++;
    } else {
        NOYAU_TCB* tcb_tid = noyau_get_p_tcb(tid);
        NOYAU_TCB* tcb_owner = noyau_get_p_tcb(mu->owner);

        if (tcb_tid->priorite < tcb_owner->priorite) {
            // Échange d'identité dans les files
            file_swap_ids(tid, mu->owner);

            // Suspendre la tâche propriétaire qui a pris la nouvelle identité
            noyau_get_p_tcb(tid)->status = SUSP;
            file_retire(tid);
            schedule();
        } else {
            // FIFO
            mu->attente[mu->fin++] = tid;
            dort();
        }
    }
}

void m_release(uint8_t m) {
    if (m >= MAX_MUTEX || mutexes[m].ref_count <= 0 || mutexes[m].owner != noyau_get_tc()) {
        noyau_exit();
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

        // Restauration de la priorité
        NOYAU_TCB* tcb = noyau_get_p_tcb(tid);
        tcb->priorite = tcb->priorite_base;
    }
}

void m_destroy(uint8_t m) {
    if (m >= MAX_MUTEX || mutexes[m].ref_count != 0) {
        noyau_exit();
    }
    mutexes[m].ref_count = -1;
    mutexes[m].owner = MUTEX_INVALID;
    mutexes[m].debut = mutexes[m].fin = 0;
}
