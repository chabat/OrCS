#include "../simulator.hpp"

// Constructor
uopCache_t::uopCache_t(){
    this->UC_SETS = 0;
    this->MAX_BASIC_BLOCK_SIZE = 0;
    this->OUTPUT_WIDTH = 0;
}

void uopCache_t::allocate(){
    // Get config
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

    // Get Uop Cache info
    libconfig::Setting &uc_defs = cfg_root["UOP_CACHE"];
    set_UC_SETS(uc_defs["UC_SETS"]);
    set_MAX_BASIC_BLOCK_SIZE(uc_defs["MAX_BASIC_BLOCK_SIZE"]);
    set_OUTPUT_WIDTH(uc_defs["OUTPUT_WIDTH"]);

    this->sets = new uopSet_t[this->UC_SETS];


    /*
    printf("UC Configurations:\n");
    printf("SETS: %d\n", UC_SETS);
    printf("WAYS: %d\n", UC_WAYS);
    printf("MAX UOP PER LINE: %d\n", MAX_UOPS_PER_LINE);
    printf("MAX_BASIC_BLOCK_SIZE: %d\n", MAX_BASIC_BLOCK_SIZE);
    printf("OUTPUT_WIDTH: %d\n", OUTPUT_WIDTH);
    */

}
