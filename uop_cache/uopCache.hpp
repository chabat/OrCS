#ifndef UOP_CACHE_H
#define UOP_CACHE_H
using namespace std;

class uopCache_t{
    private:
        const uint32_t N_SETS = 32
        const uint32_t N_WAYS = 8

    public:
        uopCache_t();
        ~uopCache_t();

        uint32_t associativity;
        uint32_t n_sets;
        uopCacheSet_t *sets;
};

#endif // UOP_CACHE_H
