#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <filesystem>
#include <bits/stdc++.h> 
#include "BloomFilter.hpp"
#include "TestBloom.hpp"

using namespace std;

// Déclarations supposées (doivent être définies dans les headers)
size_t theoretical_k_opt(size_t m, size_t n);
double theoretical_fp_prob(size_t m, size_t n, size_t k);
double empirical_fp_rate(size_t m, int k, size_t n, size_t queries, uint64_t hash_seed_1, uint64_t hash_seed_2, uint64_t exp_seed);

int main(int argc, char** argv) {
    
    // Définir les paramètres d'étude fixes
    const size_t M_TEST = 50000;
    const size_t N_TEST = 45000;
    
    const size_t QUERIES = 500000;
    const int REPEATS = 1;

    // Calcul du k* optimal (théorique) : k* ≈ ln(2) * (6000000/10000) ≈ 0.693 * 600 ≈ 415.8
    auto kstar_double = (M_TEST / (double)N_TEST) * log(2.0);
    int kstar = static_cast<int>(std::round(kstar_double));
    
    // --- Calcul des bornes pour une plage d'environ 10 points ---
    
    const int TARGET_POINTS = 10;
    // Définir la demi-fenêtre : 4 à gauche, 5 à droite (pour 10 points autour de kstar)
    const int HALF_WINDOW = TARGET_POINTS / 2; 

    // Début de la plage (minimum 1)
    int k_min_range = std::max(1, kstar - HALF_WINDOW); 
    
    // Fin de la plage (assure que nous avons TARGET_POINTS, si kstar est assez grand)
    int k_max_range = k_min_range + TARGET_POINTS - 1; 

    // --- Initialisation et préparation de la boucle ---

    std::vector<int> ks;
    // Remplir le vecteur ks avec toutes les valeurs entières dans la plage
    for (int k = k_min_range; k <= k_max_range; ++k) {
        ks.push_back(k);
    }
    
    // Assurer que kstar est dans la plage, même si les arrondis l'ont décalé
    if (std::find(ks.begin(), ks.end(), kstar) == ks.end()) {
        ks.push_back(kstar);
        std::sort(ks.begin(), ks.end());
    }
    
    // Vérification finale et nettoyage des doublons
    ks.erase(std::unique(ks.begin(), ks.end()), ks.end());


    std::filesystem::create_directories("results");
    std::ofstream csv_out("results/results.csv"); 
    if (!csv_out) {
        cerr << "Failed to open results/results.csv for writing\n";
        return 1;
    }
    csv_out << "m,n,k,empirical_fp,theoretical_fp,theoretical_kstar\n";

    // --- Boucle d'expérimentation ---
    cout << "Starting experiment for m=" << M_TEST << ", n=" << N_TEST 
         << " | kstar (opt.)=" << kstar 
         << " | Range=[" << ks.front() << ", " << ks.back() << "] (" << ks.size() << " points)\n";

    for (int rep = 0; rep < REPEATS; ++rep) {
        uint64_t seed = static_cast<uint64_t>(rep * 10007 + N_TEST + M_TEST);

        for (int k : ks) {
            double empirical = empirical_fp_rate(M_TEST, k, N_TEST, QUERIES, 12345ULL, 67890ULL, seed);
            double theoretical = theoretical_fp_prob(M_TEST, N_TEST, k);

            csv_out << M_TEST << "," << N_TEST << "," << k << "," 
                    << empirical << "," << theoretical << "," << kstar << "\n";
            
            cout << "k=" << k << " -> Empirical: " << empirical << ", Theoretical: " << theoretical << endl;
        }
    }

    csv_out.close();
    cout << "Data saved to results/results.csv.\n";

    return 0;
}