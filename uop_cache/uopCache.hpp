#ifndef UOP_CACHE_H
#define UOP_CACHE_H

//using namespace std;

class uopCache_t{
    private:
        uint32_t UC_SETS;
        uint32_t MAX_UOPS_PER_LINE;
        uint32_t MAX_BASIC_BLOCK_SIZE;
        uint32_t OUTPUT_WIDTH;

    public:
        uopSet_t *sets;

        uopCache_t();
        ~uopCache_t();
        void allocate();

        INSTANTIATE_GET_SET_ADD(uint32_t, UC_SETS)
        INSTANTIATE_GET_SET_ADD(uint32_t, MAX_BASIC_BLOCK_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, OUTPUT_WIDTH)

};

#endif // UOP_CACHE_H
