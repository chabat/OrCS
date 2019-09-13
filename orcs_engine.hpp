// ============================================================================
class orcs_engine_t {
	private:


    public:
        /// Program input
        std::vector<std::string> arg_trace_file_name;
        /// Control the Global Cycle
        uint64_t global_cycle;
        // =======================================
        // Number of cores
        // =======================================
        uint32_t number_of_cores;
        // =======================================
        // output File name
        // =======================================
        char *output_file_name;
        char *config_file;
        bool simulator_alive;
        /// Components modeled
        trace_reader_t *trace_reader;
        configure_t *configuration;
        //==================
        //Processor
        //==================
        processor_t *processor;
        //==================
        //Branch Predictor
        //==================
        branch_predictor_t *branchPredictor;
        //cache Manager and cache
        cache_manager_t *cacheManager;
        //Memory Controller
        memory_controller_t *memory_controller;

        // ====================================================================
        /// Statistics related
        // ====================================================================
        /// Time statistics
        struct timeval stat_timer_start, stat_timer_end;
        // ====================================================================
		/// Methods
		// ====================================================================
		orcs_engine_t();
		void allocate(uint32_t NUMBER_OF_PROCESSORS);
        bool get_simulation_alive(uint32_t NUMBER_OF_PROCESSORS);
        uint64_t get_global_cycle() {
            return this->global_cycle;
        };
};
