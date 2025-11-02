# Projet Filtre de Bloom

Ce projet implémente et évalue la performance d'un filtre de Bloom.

## Structure du projet

-   `include/` : Contient les fichiers d'en-tête.
    -   `BloomFilter.hpp`: Implémentation de la classe du filtre de Bloom.
    -   `HashFactory.hpp`: Fonctions de hachage.
    -   `TestBloom.hpp`: Fonctions utilitaires pour les tests.
-   `src/` : Fichiers sources.
    -   `BloomFilter.cpp`: Implémentation des méthodes de la classe `BloomFilter`.
-   `experiments/` : Code pour les expérimentations.
    -   `TestBloom.cpp`: Programme principal pour lancer les tests et générer les données.
-   `scripts/` : Scripts utilitaires.
    -   `plot_results.py`: Script Python pour générer des graphiques à partir des données CSV.
-   `results/` : Contient les données brutes générées par les expériences.
-   `plots/` : Contient les graphiques générés par le script `plot_results.py`.
-   `makefile` : Automatise la compilation, l'exécution et le nettoyage du projet.

## Comment utiliser

### Prérequis

-   Un compilateur C++ (supportant C++17), comme `g++`.
-   Python 3 avec les bibliothèques `pandas` et `matplotlib`.

### Compilation

Pour compiler le projet, exécutez la commande suivante :

```bash
make build
```

### Exécution des tests

Pour lancer une série de tests, utilisez :

```bash
make run
```

Cette commande va :
1.  Compiler le code source.
2.  Exécuter le programme de test `test_bloom.exe`, qui va générer un fichier `results/results.csv`.
3.  Lancer le script `plot_results.py` pour créer les graphiques à partir des données.

### Nettoyage

Pour supprimer les fichiers générés (exécutables et résultats) :

```bash
make clean
```

Pour une suppression complète incluant les graphiques :

```bash
make dist-clean
```

