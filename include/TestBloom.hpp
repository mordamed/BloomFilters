#pragma once
#include <random>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>

// The header provides test helpers and expects that BloomFilter is available

inline size_t theoretical_k_opt(size_t m, size_t n) {
    if (n == 0) return 1;
    return static_cast<size_t>(std::max(1.0, std::round((m / static_cast<double>(n)) * std::log(2.0))));
}

inline double theoretical_fp_prob(size_t m, size_t n, int k) {
    if (n == 0) return 0.0;
    double exp_term = std::exp(- (static_cast<double>(k) * n) / static_cast<double>(m));
    double p = std::pow(1.0 - exp_term, k);
    return p;
}

inline double empirical_fp_rate(size_t m, int k, size_t n, size_t queries,
                                uint64_t s1 = 12345ULL, uint64_t s2 = 67890ULL,
                                uint64_t rng_seed = 42ULL)
{
    // Build Bloom filter and log progress during insertions and queries
    BloomFilter bf(m, k, s1, s2);

    std::mt19937_64 rng(rng_seed);
    std::uniform_int_distribution<uint64_t> dist;

    std::unordered_set<uint64_t> inserted;
    inserted.reserve(n * 2 + 10);

    // Logging cadence: approximately 10 updates during each phase
    size_t insert_log_step = std::max<size_t>(1, n / 10);
    size_t query_log_step  = std::max<size_t>(1, queries / 10);

    size_t inserted_count = 0;
    while (inserted.size() < n) {
        uint64_t x = dist(rng);
        if (inserted.insert(x).second) {
            bf.add(x);
            ++inserted_count;
            if ((inserted_count % insert_log_step) == 0) {
                std::cout << "[insertion] k=" << k << " inserted " << inserted_count << "/" << n << "...\n";
            }
        }
    }
    std::cout << "[insertion] k=" << k << " done, total inserted=" << inserted_count << "\n";

    size_t false_positives = 0;
    size_t performed = 0;
    size_t attempted = 0;
    while (performed < queries) {
        uint64_t q = dist(rng);
        ++attempted;
        if (inserted.count(q)) continue; // ensure queries use non-inserted keys
        if (bf.possibly_contains(q)) ++false_positives;
        ++performed;
        if ((performed % query_log_step) == 0) {
            std::cout << "[query] k=" << k << " performed " << performed << "/" << queries
                      << " false_pos=" << false_positives << "\n";
        }
    }

    std::cout << "[query] k=" << k << " done, queries=" << performed
              << " false_positives=" << false_positives << "\n";

    return static_cast<double>(false_positives) / static_cast<double>(queries);
}

inline std::pair<int, double> find_best_k(size_t m, size_t n,
                                          int k_min, int k_max,
                                          size_t queries = 100000,
                                          uint64_t s1 = 12345ULL, uint64_t s2 = 67890ULL)
{
    if (n == 0) return {1, 0.0};

    // Baseline theoretical optimum
    size_t k_star_sz = theoretical_k_opt(m, n);
    int k_star = static_cast<int>(std::max<size_t>(1, k_star_sz));

    // Initial window: at least 3, scaled with k_star but capped
    int window = std::max(3, std::min(100, static_cast<int>(std::ceil(0.5 * k_star))));

    // Hard cap to avoid unbounded searches
    const int HARD_CAP_K = std::max(500, k_max);

    int best_k = k_min;
    double best_rate = std::numeric_limits<double>::infinity();

    int expand_round = 0;
    int local_min_k = std::max(k_min, k_star - window);
    int local_max_k = std::min(k_max, k_star + window);

    std::cout << "[find_best_k] theoretical_k*=" << k_star << " initial_range=[" << local_min_k << "," << local_max_k << "] window=" << window << "\n";

    while (true) {
        ++expand_round;
        std::cout << "[find_best_k] round=" << expand_round << " scanning k in [" << local_min_k << "," << local_max_k << "]\n";

        int round_best_k = -1;
        double round_best_rate = std::numeric_limits<double>::infinity();

        for (int k = local_min_k; k <= local_max_k; ++k) {
            std::cout << "--- Testing k=" << k << " ---\n";
            double rate = empirical_fp_rate(m, k, n, queries, s1, s2, static_cast<uint64_t>(k * 1009 + 7));
            if (rate < round_best_rate) { round_best_rate = rate; round_best_k = k; }
            std::cout << "k=" << k
                      << " empirical_fp=" << rate
                      << " theoretical_k*=" << k_star
                      << " theoretical_fp=" << theoretical_fp_prob(m, n, k)
                      << "\n";
        }

        // Update global best
        if (round_best_rate < best_rate) {
            best_rate = round_best_rate;
            best_k = round_best_k;
        }

        std::cout << "[find_best_k] round_best_k=" << round_best_k << " rate=" << round_best_rate << " global_best_k=" << best_k << " rate=" << best_rate << "\n";

        // If round-best is strictly inside the range, we likely captured the optimum
        if (round_best_k > local_min_k && round_best_k < local_max_k) {
            std::cout << "[find_best_k] best k inside range; finishing.\n";
            break;
        }

        // If best touches a boundary, expand the search in that direction
        bool expand_left = (round_best_k == local_min_k) && (local_min_k > k_min);
        bool expand_right = (round_best_k == local_max_k) && (local_max_k < k_max);

        if (!expand_left && !expand_right) {
            // no boundary touching (could be equal to k_min/k_max or something else), stop
            std::cout << "[find_best_k] best at boundary but cannot expand further; finishing.\n";
            break;
        }

        if (expand_round > 8 || window > HARD_CAP_K) {
            std::cout << "[find_best_k] reached expansion limit; finishing.\n";
            break;
        }

        // Double window and recompute local_min/max, but clamp to provided bounds
        window = std::min(HARD_CAP_K, window * 2);
        local_min_k = std::max(k_min, k_star - window);
        local_max_k = std::min(k_max, k_star + window);
        std::cout << "[find_best_k] expanding window -> " << window << " new_range=[" << local_min_k << "," << local_max_k << "]\n";
    }

    std::cout << ">>> Best k empirical = " << best_k << " with fp = " << best_rate << "\n";
    return {best_k, best_rate};
}
