#include "../simulator.hpp"

// Constructor
uopCache_t::uopCache_t(){
    this->associativity = this->N_WAYS;
    this->n_sets = this->N_SETS;
    this->sets = new uopCacheSet_t[this->n_sets];
}
