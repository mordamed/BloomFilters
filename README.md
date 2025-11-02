# Projet Filtre de Bloom

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

