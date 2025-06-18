/*----------------------------------------------------------------------------*
 * fichier : noyau_file_prio.c                                                *
 * gestion de la file d'attente des taches pretes et actives                  *
 * la file est rangee dans un tableau. ce fichier definit toutes              *
 * les primitives de base                                                     *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "noyau_file_prio.h"
// recuperation du bon fichier selon l'architecture pour la fonction printf
#include "../io/serialio.h"

static uint16_t _file[MAX_PRIO][MAX_TACHES_FILE];     /* anneaux (next)    */
static uint16_t _queue[MAX_PRIO];                     /* last-in par prio  */

/*  Identité explicite associée à chaque case de l’anneau          */
static uint16_t _id_explicit[MAX_PRIO][MAX_TACHES_FILE];


/*----------------------------------------------------------------------------*
 * variables communes a toutes les procedures                                 *
 *----------------------------------------------------------------------------*/

/*
 * tableau qui stocke les taches
 * indice = numero de tache
 * valeur = tache suivante
 */
static uint16_t _file[MAX_PRIO][MAX_TACHES_FILE];

/*
 * index de queue
 * valeur de l'index de la tache en cours d'execution
 * pointe sur la prochaine tache a activer
 */
static uint16_t _queue[MAX_TACHES_FILE];

void file_init(void)
{
    for (uint16_t p = 0; p < MAX_PRIO; ++p) {
        _queue[p] = F_VIDE;
        for (uint16_t i = 0; i < MAX_TACHES_FILE; ++i) {
            _file[p][i]        = i;        /* anneau auto-bouclé             */
            _id_explicit[p][i] = MAX_TACHES_NOYAU;  /* invalide                */
        }
    }
}

void file_ajoute(uint16_t id)
{
    uint16_t p = id >> 3;      /* priorité “rang”        */
    uint16_t n = id & 7;       /* index dans l’anneau    */

    _id_explicit[p][n] = id;   /* mémorise l’identité    */

    if (_queue[p] == F_VIDE) {          /* première entrée                 */
        _file[p][n] = n;
    } else {
        _file[p][n]       = _file[p][_queue[p]];
        _file[p][_queue[p]] = n;
    }
    _queue[p] = n;
}

void file_retire(uint16_t id)
{
    uint16_t p = id >> 3, n = id & 7;
    uint16_t *q = &_queue[p], *ring = _file[p];

    if (*q == F_VIDE) return;      /* rien à faire                        */

    if (*q == ring[*q]) {          /* anneau mono-élément                */
        *q = F_VIDE;
    } else {
        /* recherche du prédécesseur de n                                 */
        uint16_t pred = *q;
        while (ring[pred] != n) pred = ring[pred];
        ring[pred] = ring[n];      /* by-pass n                           */
        if (*q == n) *q = pred;    /* maj tail si besoin                  */
    }
    _id_explicit[p][n] = MAX_TACHES_NOYAU;   /* case redevenue libre       */
}

uint16_t file_suivant(void)
{
    for (uint16_t p = 0; p < MAX_PRIO; ++p) {
        if (_queue[p] != F_VIDE) {
            uint16_t head = _file[p][_queue[p]]; /* premier de l’anneau    */
            _queue[p]     = head;                /* round-robin            */
            return _id_explicit[p][head];        /* vraie identité         */
        }
    }
    return MAX_TACHES_NOYAU;   /* plus rien d’éligible                      */
}


/*
 * affiche la queue, donc la derniere tache
 * entre  : sans
 * sortie : sans
 * description : affiche la valeur de queue
 */
void file_affiche_queue() {
	uint16_t i;
	for (i=0; i < MAX_TACHES_FILE; i++){
		 printf("_queue[%d] = %d\n", i, _queue[i]);
	}
}

/*
 * affiche la file
 * entre  : sans
 * sortie : sans
 * description : affiche les valeurs de la file
 */
void file_affiche() {
	uint16_t i,j;

    for (j=0; j < MAX_TACHES_FILE; j++){
		printf("Tache   | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", i);
		}

		printf("\nSuivant | ");
		for (i = 0; i < MAX_TACHES_FILE; i++) {
			printf("%03d | ", _file[j][i]);
		}
		printf("\n");
    }
}

void file_swap_ids(uint16_t id1, uint16_t id2)
{
    if (id1 == id2) return;

    uint16_t p1 = id1 >> 3, n1 = id1 & 7;
    uint16_t p2 = id2 >> 3, n2 = id2 & 7;

    /* échange pur et simple des identités explicites                        */
    uint16_t tmp                     = _id_explicit[p1][n1];
    _id_explicit[p1][n1]             = _id_explicit[p2][n2];
    _id_explicit[p2][n2]             = tmp;
}
