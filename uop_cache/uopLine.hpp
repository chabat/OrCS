#ifndef UOP_LINE_H
#define UOP_LINE_H

class uopLine_t{

    public:
        uint32_t MAX_UOPS_PER_LINE;  // Maximum of uops per cache line
        uint64_t tag;                // Line tag
        uint32_t validUops;          // Number of valid Uops in that line
        uint32_t instructionsLength; // Length of the x86 instructions corresponding to the cache line
        uop_package_t *uops;         // Uops in this cache line

        // Constructor
        uopLine_t(){
            this->validUops = 0;
            this->tag = -1;
            this->instructionsLength = 0;

            // Get config
            libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

            // Get Uop Cache info
            libconfig::Setting &uc_defs = cfg_root["UOP_CACHE"];
            set_MAX_UOPS_PER_LINE(uc_defs["MAX_UOPS_PER_LINE"]);

            printf("Uop nova no pedaço");
            uops = new uop_package_t[this->MAX_UOPS_PER_LINE];
        }

        // Destructor
        ~uopLine_t(){
            delete[] uops;
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, MAX_UOPS_PER_LINE)

};

#endif // UOP_LINE_H
