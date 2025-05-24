# Mini-Projet MI11 - Noyau Temps Réel avec Priorité Dynamique et Héritage de Priorité

## 🧠 Contexte et objectifs pédagogiques

Ce mini-projet, réalisé dans le cadre du module AI39/MI11 de l'Université de Technologie de Compiègne (UTC), a pour but de mettre en œuvre des mécanismes fondamentaux des systèmes temps réel embarqués :

* L’**ordonnancement à priorité dynamique**,
* La **synchronisation par mutex**,
* La **gestion de l’inversion de priorité** via un mécanisme d’**héritage de priorité**,
* Le tout sur un **noyau temps réel bare-metal** fonctionnant sous **QEMU** (sans OS ni allocation dynamique).

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

* **`Notice Mac.md`** : guide détaillé pour la configuration de l’environnement de développement sous macOS,
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