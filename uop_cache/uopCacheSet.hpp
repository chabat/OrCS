#ifndef UOP_CACHESET_H
#define UOP_CACHESET_H

class uopCacheSet_t{
private:
    const uint32_t N_WAYS = 8;

public:
    uint32_t nlines;
    uint32_t freeLines;
    uopLine_t *lines;

    // Constructor
    uopCacheSet_t(){
        this->nlines = N_WAYS;
        this->freeLines = nlines;
        this->lines = new uopLine_t[nlines];
    }

    // Desctructor
    ~uopCacheSet_t(){
        delete[] lines;
    }

};

#endif // UOP_CACHESET_H
