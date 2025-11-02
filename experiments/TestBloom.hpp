#pragma once
#include <random>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

// Le header suppose que la structure `BloomFilter` est déjà définie
// (par exemple dans [BloomFilter.cpp](BloomFilter.cpp)). Il fournit des
// fonctions de test et d'estimation de la meilleure valeur de k.

inline size_t theoretical_k_opt(size_t m, size_t n) {
    if (n == 0) return 1;
    return static_cast<size_t>(std::max(1.0, std::round((m / static_cast<double>(n)) * std::log(2.0))));
}

// Probabilité théorique de faux-positif pour (m,n,k)
inline double theoretical_fp_prob(size_t m, size_t n, int k) {
    if (n == 0) return 0.0;
    double exp_term = std::exp(- (static_cast<double>(k) * n) / static_cast<double>(m));
    double p = std::pow(1.0 - exp_term, k);
    return p;
}

// Effectue un test empirique pour (m,k) en insérant `n` clés et en effectuant `queries` requêtes non-insérées.
// Les clés sont tirées aléatoirement. Retourne le taux de faux positifs empirique.
inline double empirical_fp_rate(size_t m, int k, size_t n, size_t queries,
                                uint64_t s1 = 12345ULL, uint64_t s2 = 67890ULL,
                                uint64_t rng_seed = 42ULL)
{
    // Nécessite que `BloomFilter` soit visible dans la TU qui inclut ce header.
    BloomFilter bf(m, k, s1, s2);

    std::mt19937_64 rng(rng_seed);
    std::uniform_int_distribution<uint64_t> dist;

    std::unordered_set<uint64_t> inserted;
    inserted.reserve(n * 2 + 10);

    // Log settings
    size_t insert_log_step = std::max<size_t>(1, n / 10);
    size_t query_log_step  = std::max<size_t>(1, queries / 10);

    // Insérer n clés distinctes
    size_t inserted_count = 0;
    while (inserted.size() < n) {
        uint64_t x = dist(rng);
        if (inserted.insert(x).second) {
            bf.add(x);
            ++inserted_count;
            if ((inserted_count % insert_log_step) == 0) {
                //std::cout << "[insertion] k=" << k << " inserted " << inserted_count << "/" << n << "...\n";
            }
        }
    }
    //std::cout << "[insertion] k=" << k << " done, total inserted=" << inserted_count << "\n";

    // Requêtes non-insérées : tirer et ignorer si tirage tombe dans `inserted`
    size_t false_positives = 0;
    size_t performed = 0;
    size_t attempted = 0;
    while (performed < queries) {
        uint64_t q = dist(rng);
        ++attempted;
        if (inserted.count(q)) continue; // s'assurer que la requête n'a pas été insérée
        if (bf.possibly_contains(q)) ++false_positives;
        ++performed;
        if ((performed % query_log_step) == 0) {
            //std::cout << "[query] k=" << k << " performed " << performed << "/" << queries
            //          << " false_pos=" << false_positives << "\n";
        }
    }

    //std::cout << "[query] k=" << k << " done, queries=" << performed
    //          << " false_positives=" << false_positives << "\n";

    return static_cast<double>(false_positives) / static_cast<double>(queries);
}

// Parcourt k dans [k_min, k_max], calcule le taux empirique et renvoie la meilleure valeur.
inline std::pair<int, double> find_best_k(size_t m, size_t n,
                                          int k_min, int k_max,
                                          size_t queries = 100000,
                                          uint64_t s1 = 12345ULL, uint64_t s2 = 67890ULL)
{
    int best_k = k_min;
    double best_rate = std::numeric_limits<double>::infinity();

    for (int k = k_min; k <= k_max; ++k) {
        //std::cout << "=== Testing k=" << k << " (" << (k - k_min + 1) << "/" << (k_max - k_min + 1) << ") ===\n";
        double rate = empirical_fp_rate(m, k, n, queries, s1, s2, static_cast<uint64_t>(k * 1009 + 7));
        if (rate < best_rate) { best_rate = rate; best_k = k; }
        //std::cout << "k=" << k
                //   << " empirical_fp=" << rate
                //   << " theoretical_k*=" << theoretical_k_opt(m, n)
                //   << " theoretical_fp=" << theoretical_fp_prob(m, n, k)
                //   << "\n";
    }

    //std::cout << ">>> Best k empirical = " << best_k << " with fp = " << best_rate << "\n";
    return {best_k, best_rate};
}