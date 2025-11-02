#pragma once            //directive de préprocesseur, "once" empeche l'inclusion multiple d'un même header
#include <cstdint>
#include <vector>
#include <functional>

// SplitMix64
struct SplitMix64 {
    uint64_t seed;

    explicit SplitMix64(uint64_t s =0) : seed(s) {}

    uint64_t operator()(uint64_t x) const {
        x += 0x9e3779b97f4a7c15ULL + seed;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
};

// XorShiftMix64
struct XortShiftMix64{
    uint16_t seed;

    explicit XortShiftMix64(uint16_t s=0) : seed(s){}

    uint64_t operator()(uint64_t x) const {
        x ^= seed;
        x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;        
    }

};

//On utilise nos deux fct de hash pour hacher dans [0, m-1]
class HashFactory {
public :
    using index_t = size_t; //pour le compilateur, on spécifie qu'on utilise 'index_t' a la place de 'size_t'
    
    explicit HashFactory(   size_t m,
                            uint64_t seed1 = 0x1234567890abcdefULL,
                            uint64_t seed2 = 0xfedcba0987654321ULL)
        :   m_(m), H_(seed1), G_(seed2) {}

    index_t index(uint64_t x, int i) const {
        uint64_t h1 = H_(x);
        uint64_t h2 = G_(x);
        h2 |= 1ULL ; //OR bit à bit
        if ((m_ & (m_ - 1)) == 0){
            return static_cast<index_t>((h1 + (uint64_t)i * h2) & (m_ - 1));
        } else {
            return static_cast<index_t>((h1 + (uint64_t)i * h2) % m_);
        }
    }

    size_t modulut() const { return m_; }

private :
    size_t m_;      // Par convention on met le '_' pour spécifier que c'est une var privée
    SplitMix64 H_;
    XortShiftMix64 G_;
};
