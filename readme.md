# Mini-Projet MI11 - Noyau Temps Réel avec Priorité Dynamique et Héritage de Priorité

## 🧠 Contexte et objectifs pédagogiques

Ce mini-projet, réalisé dans le cadre du module AI39/MI11 de l’[Université de Technologie de Compiègne (UTC)](https://www.utc.fr), a pour but de mettre en œuvre des mécanismes fondamentaux des systèmes temps réel embarqués :

* L’**ordonnancement à priorité dynamique**,
* La **synchronisation par mutex**,
* La **gestion de l’inversion de priorité** via un mécanisme d’**héritage de priorité**,
* Le tout sur un **noyau temps réel bare-metal** fonctionnant sous **QEMU** (sans OS ni allocation dynamique).

---

## 1. 🧠 Contexte et Objectifs Pédagogiques

Ce mini-projet s'inscrit dans le cadre du module MI11 de l'UTC. L'objectif était de concevoir et d'implémenter un noyau temps réel préemptif embarqué, simulé sur une plateforme ARM, et capable de gérer des tâches concurrentes via :

* Un ordonnanceur **à priorité dynamique**,
* Un système de **synchronisation par mutexs**,
* Un **mécanisme d'héritage de priorité** pour traiter l'inversion de priorité,
* Un test exhaustif simulant un cas classique d'inversion de priorité.

Ce projet vise à comprendre en profondeur le comportement d'un noyau embarqué dans un environnement contraint et sans OS.

---

## 2. 🌀 Architecture Globale

### 2.1 Schéma simplifié du noyau

```
+------------------+     +----------------------+     +--------------------+
|  noyau_prio.c    | <-> | noyau_file_prio.c    | <-> | mutex.c            |
| (ordonnanceur)   |     | (gestion des files)  |     | (mutex + héritage) |
+------------------+     +----------------------+     +--------------------+
```

### 2.2 TCB (Task Control Block)

```c
typedef struct {
  uint16_t  status;			/* etat courant de la tache        */
  uint32_t  sp_ini;    		/* valeur initiale de sp           */
  uint32_t  sp_start;   	/* valeur de base de sp pour la tache */
  uint32_t  sp;        		/* valeur courante de sp           */
  TACHE_ADR task_adr;    	/* Pointeur de la fonction de tâche*/
  uint32_t  delay;			/* valeur courante decomptage pour reveil */
  void   	*arg; 			/* pointeur sur des paramètres supplémentaires pour la tâches */

  uint8_t priorite;
  uint8_t priorite_base;
} NOYAU_TCB;
```

---

## 3. ⚙️ Ordonnancement et Priorité Dynamique

### 3.1 Ordonnanceur

Les tâches sont organisées dans des **files par priorité**. L'ordonnanceur sélectionne toujours la tâche de priorité la plus élevée (valeur de `priorite` la plus faible).

### 3.2 Dynamisme des priorités

Lors d'un héritage, une tâche peut temporairement se voir affecter une priorité plus haute. Cela modifie son insertion dans les files, réordonne le planificateur et change les identifiants de tâche si besoin (voir section suivante).

---

## 4. 🔒 Mutex et Synchronisation

### 4.1 Structure d'un mutex

```c
typedef struct {
    int8_t ref_count;             // -1 = libre, ≥0 = nb d'acquisitions
    uint8_t owner;                // id de la tâche détentrice
    uint8_t attente[MAX_TACHES];  // FIFO d'attente des tâches bloquées
    uint8_t debut, fin;
} MUTEX;
```

### 4.2 Fonctionnement

* Lorsqu'une tâche acquiert un mutex, elle en devient propriétaire.
* Si une autre tâche demande ce mutex :

    * Elle hérite temporairement de l'identité de la propriétaire,
    * La priorité est propagée via `tcb->priorite`,
    * Un échange d'identifiant est réalisé dans les files (`file_swap_ids`).

---

## 5. 📊 Gestion de l'inversion de priorité

### 5.1 Cas testé

```text
TacheMutex1 (priorité 6) acquiert le mutex
TacheAutre  (priorité 4) tourne librement
TacheMutex2 (priorité 2) demande le mutex → bloquée
```

### 5.2 Sans héritage

* TacheMutex2 reste bloquée
* TacheAutre s'exécute avant que TacheMutex1 ne libère le mutex
* **Défaut d'inversion de priorité observé**

### 5.3 Avec héritage

* TacheMutex1 hérite de la priorité de TacheMutex2
* Elle termine son travail plus rapidement
* Le mutex est libéré plus tôt

### 5.4 Chronogramme attendu (ASCII)

```
Sans héritage :
[TM2]---------
             [TA]--------------
                          [TM1]----------------------

Avec héritage :
[TM2]---------
             [TM1 (priorité 2)]-------------
                                     [TA]------
```

---

## 6. 🛋þ Analyse critique

### 6.1 Hypothèses simplificatrices

* Une tâche ne peut posséder qu'un seul mutex à la fois
* L'héritage n'est **pas transitif**
* L'échange d'identité dans les files est manuel (non global)

### 6.2 Limites

* Pas de détection d'abandon de mutex
* Pas de file d'attente prioritaire (FIFO uniquement)
* Pas de vérification du débordement de `attente[]`

### 6.3 Pistes d'amélioration

* Gestion transitive des héritages de priorité
* Ajout d'une file de priorité au lieu d'une FIFO
* Outils de tracing + analyse temps d'exécution

---

## 7. 🔧 Compilation et Debug

### 7.1 Compilation (Mac/Linux)

```bash
make clean
make
```

### 7.2 Lancement via QEMU

```bash
qemu-system-arm -M mps2-an500 -cpu cortex-m7 -nographic -serial mon:stdio -kernel kernel.elf
```

---

## 🔧 Implémentation technique

### ⚙️ Ordonnanceur dynamique

Le noyau supporte :

* Des tâches avec une priorité dynamique (`priorite`) modifiable,
* Une priorité de base (`priorite_base`) conservée pour restauration,
* Un ordonnancement basé sur le niveau de `priorite` le plus bas (plus prioritaire).

### 🔐 Mutex

Chaque mutex est une structure locale :

```c
typedef struct {
    int8_t ref_count;             // -1 = libre, >=0 = nb d'acquisitions
    uint8_t owner;                // id tâche qui détient le mutex
    uint8_t attente[MAX_TACHES];  // FIFO d’attente
    uint8_t debut, fin;           // Indices de la FIFO
} MUTEX;
```

### 🚦 Héritage de priorité

Lorsqu’une tâche de priorité `P2` tente d’accéder à un mutex détenu par une tâche `P1` de priorité plus basse :

* Si `P2.priorite < P1.priorite`, alors `P1.priorite = P2.priorite` (héritage),
* Une fois le mutex libéré, `P1.priorite` est restaurée à `P1.priorite_base`.

Ce mécanisme est volontairement simplifié (pas d’héritage transitif, un mutex par tâche max).

## 🧪 Campagne de tests

Le programme `noyau_test_mutex.c` met en scène 3 tâches :

1. `TacheMutex1` — priorité basse (détient un mutex),
2. `TacheAutre` — priorité moyenne (pas de mutex),
3. `TacheMutex2` — priorité haute (attend le mutex).

### 🔍 Sans héritage de priorité

```text
TacheMutex2 reste bloquée, car TacheAutre prend le CPU avant que TacheMutex1 ne libère le mutex → inversion de priorité.
```

### ✅ Avec héritage activé

```text
TacheMutex1 hérite de la priorité de TacheMutex2 → finit plus vite → débloque la situation.
```

## 🧱 Architecture du noyau

* `noyau_prio.c` : ordonnanceur basé sur des files par priorité
* `mutex.c` : implémentation complète du mutex avec héritage
* `fifo.c` : gestion de files FIFO locales pour synchronisation
* `init.c` : création des tâches avec initialisation de la priorité
* `delay.c` : temporisation et réveil différé

## 💻 Compilation et débogage

* Utilisation de **QEMU** : `mps2-an500`, `cortex-m7`, UART simulé
* Debugging : via `gdb-multiarch` connecté à QEMU
* Affichage : terminal série sur port 1234

## 📑 À inclure dans le rapport PDF

1. Objectifs pédagogiques et techniques
2. Fonctionnement du noyau temps réel
3. Rôle des priorités fixes/dynamiques
4. Schéma de fonctionnement du mutex avec héritage
5. Chronogrammes (avant / après)
6. Analyse critique :

    * Hypothèses simplificatrices (1 mutex/tâche, non-transitivité)
    * Comportement attendu vs observé
    * Limitations et pistes d’amélioration

## 📘 Documentation jointe

Le dépôt contient également :

* [Notice Mac.md](./Notice%20Mac.md) : guide détaillé pour la configuration de l’environnement de développement sous macOS,
* Commandes pour lancer QEMU, se connecter au port série, et effectuer le débogage.

---

## 👨‍💻 Auteur

[Hugo Pereira](https://github.com/tigrou23) & Maher Zizouni

Passionnés par le système Linux, le développement embarqué, et l’optimisation bas-niveau.
Toujours curieux d’aller voir ce qu’il y a "sous le capot".

📫 [hugo.pereira@etu.utc.fr](mailto:hugo.pereira@etu.utc.fr)

📫 [maher.zizouni@etu.utc.fr](mailto:maher.zizouni@etu.utc.fr)

---

> *“Intelligence is the ability to avoid doing work, yet getting the work done.” — Linus Torvalds*