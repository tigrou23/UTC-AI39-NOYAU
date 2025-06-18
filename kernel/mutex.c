#include "mutex.h"
#include "noyau_prio.h"
#include "noyau_file_prio.h"

typedef struct {
    int8_t   ref_count;
    uint16_t owner;                         /* id explicite                 */
    uint16_t attente[MAX_TACHES];
    uint8_t  debut, fin;
} MUTEX;

static MUTEX mutexes[MAX_MUTEX];

#define M_INVALID 0xFFFF          /* valeur sûre hors domaine id          */

/*------------------------------- init -------------------------------------*/
void m_init(void)
{
    for (uint8_t i = 0; i < MAX_MUTEX; ++i) {
        mutexes[i].ref_count = -1;
        mutexes[i].owner     = M_INVALID;
        mutexes[i].debut = mutexes[i].fin = 0;
    }
}

/*------------------------------- create -----------------------------------*/
uint8_t m_create(void)
{
    for (uint8_t i = 0; i < MAX_MUTEX; ++i)
        if (mutexes[i].ref_count == -1) {
            mutexes[i].ref_count = 0;
            return i;
        }
    return MAX_MUTEX;
}

/*------------------------------- acquire ----------------------------------*/
void m_acquire(uint8_t m)
{
    if (m >= MAX_MUTEX || mutexes[m].ref_count == -1) noyau_exit();

    MUTEX  *mu  = &mutexes[m];
    uint16_t tc = noyau_get_tc();           /* tâche courante              */

    /* mutex libre ---------------------------------------------------------*/
    if (mu->ref_count == 0) {
        mu->ref_count = 1;
        mu->owner     = tc;
        return;
    }

    /* récursivité ---------------------------------------------------------*/
    if (mu->owner == tc) { ++mu->ref_count; return; }

    /* -------------------------------------------------------------------- */
    NOYAU_TCB *tcb_tc    = noyau_get_p_tcb(tc);
    NOYAU_TCB *tcb_owner = noyau_get_p_tcb(mu->owner);

    /* cas PRIORITY-INHERITANCE -------------------------------------------*/
    if (tcb_tc->priorite < tcb_owner->priorite) {

    	/* 1. échange des identités dans les files -------------------- */
    	file_swap_ids(tc, mu->owner);

		/* 2. on met la tâche courante dans la liste d’attente du mutex */
		mu->attente[mu->fin++] = tc;

		/* 3. on la met en SUSP et on retire la *case* qui la contient  */
        noyau_get_p_tcb(tc)->status = SUSP;
        file_retire(mu->owner);          /* <-- la bonne entrée          */

        schedule();                      /* on laisse tourner la tâche
                                               qui a hérité de la prio       */
    	return;
    }
    else {
        /* priorité ≤ : simple blocage FIFO                                 */
        mu->attente[mu->fin++] = tc;
        dort();             /* dort() endort tc et planifie une autre tâche */
    }
}

/*------------------------------- release ----------------------------------*/
void m_release(uint8_t m)
{
    if (m >= MAX_MUTEX) noyau_exit();
    MUTEX *mu = &mutexes[m];

    if (mu->owner != noyau_get_tc() || mu->ref_count <= 0) noyau_exit();

    /* décrément récursivité ----------------------------------------------*/
    if (--mu->ref_count > 0) return;

    /* libération effective ------------------------------------------------*/
    uint16_t old_owner = mu->owner;

    /* quelqu’un attend ? --------------------------------------------------*/
    if (mu->debut < mu->fin) {
        uint16_t next = mu->attente[mu->debut++];
        mu->owner     = next;
        mu->ref_count = 1;

        /* dés-héritage : on remet l’ancienne propriétaire à sa priorité base */
        NOYAU_TCB *tcb_old = noyau_get_p_tcb(old_owner);
        if (tcb_old->priorite != tcb_old->priorite_base) {
            file_retire(old_owner);                 /* la sortir de l’anneau “hérité” */
            tcb_old->priorite = tcb_old->priorite_base;
            file_ajoute(old_owner);                 /* la ré-insérer au bon niveau   */
        }

        reveille(next);                             /* réveil de la tâche 16          */
    }
    else {
        /* file d’attente vide                                              */
        mu->owner = M_INVALID;

        NOYAU_TCB *tcb_old = noyau_get_p_tcb(old_owner);
        tcb_old->priorite  = tcb_old->priorite_base;

        mu->debut = mu->fin = 0;          /* remise à zéro propre           */
    }
}

/*------------------------------- destroy ----------------------------------*/
void m_destroy(uint8_t m)
{
    if (m >= MAX_MUTEX || mutexes[m].ref_count != 0) noyau_exit();

    mutexes[m].ref_count = -1;
    mutexes[m].owner     = M_INVALID;
    mutexes[m].debut = mutexes[m].fin = 0;
}
