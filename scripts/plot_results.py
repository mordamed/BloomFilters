#!/usr/bin/env python3

import argparse
import os
import sys
from io import StringIO

import pandas as pd
import matplotlib.pyplot as plt


def read_csv_tolerant(path):
    # Conserve l'en-tête et toute ligne commençant par un chiffre (lignes de données)
    with open(path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    kept = []
    for i, L in enumerate(lines):
        s = L.lstrip()
        # Conserve l'en-tête (première ligne contenant 'm,')
        if i == 0 and 'm,' in s:
            kept.append(L)
        # Conserve les lignes de données (commençant par un chiffre)
        elif s and s[0].isdigit():
            kept.append(L)
    if not kept:
        raise SystemExit(f'No CSV data found in {path}')
    return pd.read_csv(StringIO(''.join(kept)))


def main():
    p = argparse.ArgumentParser(description='Plotter for Bloom results')
    p.add_argument('csv', nargs='?', default='./results/results.csv')
    p.add_argument('--outdir', '-o', default='./plots')
    args = p.parse_args()

    if not os.path.exists(args.csv):
        print(f'CSV file not found: {args.csv}')
        sys.exit(1)

    df = read_csv_tolerant(args.csv)
    df['m'] = df['m'].astype(int)
    df['n'] = df['n'].astype(int)
    df['k'] = df['k'].astype(int)

    # 1. Agrégation des données (moyenne des répétitions)
    agg = df.groupby(['m', 'n', 'k'], as_index=False).agg(
        empirical_mean=('empirical_fp', 'mean'),
        empirical_std=('empirical_fp', 'std'),
        theoretical_mean=('theoretical_fp', 'mean'),
        theoretical_kstar=('theoretical_kstar', 'mean')
    )
    # Remplir le std des points sans répétition par 0
    agg['empirical_std'] = agg['empirical_std'].fillna(0.0)

    os.makedirs(args.outdir, exist_ok=True)
    created = 0
    
    # 2. Boucle de tracé pour chaque paire unique (m, n)
    for (m, n), sub in agg.groupby(['m', 'n']):
        # kstar est la même valeur pour toutes les lignes d'un groupe (m, n)
        kstar = int(round(sub['theoretical_kstar'].iloc[0]))
        
        # --- LOGIQUE SIMPLIFIÉE : AUCUN FILTRE APPLIQUÉ ICI ---
        # Le sous-dataframe 'sub' contient désormais TOUS les points k pour cette paire (m, n)
        
        # Assurez-vous que le dataframe n'est pas vide après l'agrégation
        if sub.empty:
            print(f"Skipping plot for m={m}, n={n} as the aggregated data is empty.")
            continue

        # Préparer les données pour Matplotlib
        sub = sub.sort_values('k')
        ks = sub['k'].to_numpy()
        emp = sub['empirical_mean'].to_numpy()
        emp_std = sub['empirical_std'].to_numpy()
        theo = sub['theoretical_mean'].to_numpy()
        

        # 3. Génération du graphique
        plt.figure(figsize=(7, 4))
        # Points empiriques avec barres d'erreur
        plt.errorbar(ks, emp, yerr=emp_std, fmt='o-', label='empirical')
        # Courbe théorique
        plt.plot(ks, theo, 'r--', label='theoretical')
        # Ligne verticale pour le k* optimal
        plt.axvline(kstar, color='gray', linestyle=':', label=f'k*={kstar}')
        
        plt.xlabel('k')
        plt.ylabel('false positive rate')
        plt.title(f'm={m}, n={n}')
        plt.legend()
        plt.grid(alpha=0.2)
        
        out_png = os.path.join(args.outdir, f'plot_m{m}_n{n}.png')
        plt.tight_layout()
        plt.savefig(out_png)
        plt.close()
        created += 1

    print(f'Wrote {created} plot files to {args.outdir}')