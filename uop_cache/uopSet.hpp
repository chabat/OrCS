#ifndef UOP_SET_H
#define UOP_SET_H

class uopSet_t{
    public:
        uint32_t UC_WAYS;
        uopLine_t *lines;

        // Constructor
        uopSet_t(){
            // Get config
            libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

            // Get Uop Cache info
            libconfig::Setting &uc_defs = cfg_root["UOP_CACHE"];
            set_UC_WAYS(uc_defs["UC_WAYS"]);

            this->lines = new uopLine_t[this->UC_WAYS];
        }

        // Destructor
        ~uopSet_t(){
            delete[] lines;
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, UC_WAYS)
};

#endif // UOP_SET_H
