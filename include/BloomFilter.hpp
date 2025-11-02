#pragma once
#include <cstdint>
#include <vector>
#include "HashFactory.hpp"

// Bloom filter implementation (header-only interface)
class BloomFilter {
public:
    BloomFilter(size_t m_, int k_, uint64_t s1 = 1, uint64_t s2 = 2)
        : m(m_), k(k_), bits((m_ + 63) / 64, 0ULL), hf(m_, s1, s2) {}

    inline void add(uint64_t x) {
        for (int i = 0; i < k; ++i) set_bit(hf.index(x, i));
    }

    inline bool possibly_contains(uint64_t x) const {
        for (int i = 0; i < k; ++i) if (!get_bit(hf.index(x, i))) return false;
        return true;
    }

    // expose basic params
    size_t size_bits() const { return m; }
    int num_hashes() const { return k; }

private:
    size_t m;
    int k;
    std::vector<uint64_t> bits;
    HashFactory hf;

    inline void set_bit(size_t i) { bits[i >> 6] |= (1ULL << (i & 63)); }
    inline bool get_bit(size_t i) const { return (bits[i >> 6] >> (i & 63)) & 1ULL; }
};
