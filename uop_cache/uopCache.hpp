#ifndef UOP_CACHE_H
#define UOP_CACHE_H
using namespace std;

class uopCache_t{
    private:
        const uint32_t N_WAYS = 8;
        const uint32_t BUFFER_SIZE = 18;
        const uint32_t LINE_SIZE = 6;
    public:
        const uint32_t N_SETS = 32;
        uint32_t currInstCount;
        uint32_t instToIgnore;
        uint32_t hitCounter;
        uint32_t missCounter;
        uint64_t decodeTimeSaved;

        uopCache_t();
        ~uopCache_t();

        void fillBufferInsert(uop_package_t uop);
        void insert();
        void evict(uint32_t set);
        void statistics();

        uint32_t associativity;
        uint32_t n_sets;
        uopCacheSet_t *sets;

        bool waitBBEnd;
        uint32_t fillBufferIndex;
        uop_package_t lastUop;
        uop_package_t *fillBuffer;
};

#endif // UOP_CACHE_H
