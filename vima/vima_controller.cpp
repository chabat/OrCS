#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){

}

vima_controller_t::~vima_controller_t(){
    ORCS_PRINTF ("#========================================================================#\n")
    ORCS_PRINTF ("VIMA Cache Hits: %lu\n", get_cache_hits())
    ORCS_PRINTF ("VIMA Cache Misses: %lu\n", get_cache_misses())
    ORCS_PRINTF ("VIMA Cache Accesses: %lu\n", get_cache_accesses())
    ORCS_PRINTF ("VIMA Cache Writebacks: %lu\n", get_cache_writebacks())
    ORCS_PRINTF ("#========================================================================#\n")
    /*free (read1);
    free (read1_unbalanced);
    free (read2);
    free (read2_unbalanced);
    free (write);
    free (write_unbalanced);*/
}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = 0; i < vima_buffer.size(); i++){
        ORCS_PRINTF ("uop %lu %s readyAt %lu status %s\n", vima_buffer[i]->uop_number, get_enum_memory_operation_char (vima_buffer[i]->memory_operation), vima_buffer[i]->readyAt, get_enum_package_state_char (vima_buffer[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    read1->set = false;
    read2->set = false;
    write->set = false;
    if (VIMA_UNBALANCED){
        read1_unbalanced->set = false;
        read2_unbalanced->set = false;
        write_unbalanced->set = false;
    }
    vima_buffer[index]->status = PACKAGE_STATE_WAIT;
    vima_buffer[index]->readyAt = orcs_engine.get_global_cycle();
    if (VIMA_DEBUG) {
        ORCS_PRINTF ("VIMA Controller clock(): instruction %lu, %s ready at cycle %lu.\n", vima_buffer[index]->uop_number, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->readyAt)
    }
    vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[index]), vima_buffer.end());
}

vima_vector_t* vima_controller_t::search_cache (uint64_t address){
    add_cache_accesses();
    uint64_t lru_cycle = UINT64_MAX;
    uint32_t lru_way = 0;
    for (uint32_t i = 0; i < get_lines(); i++){
        if (get_index(cache[i].get_address()) == get_index (address) && get_tag(cache[i].get_address()) == get_tag (address)) {
            add_cache_hits();
            //ORCS_PRINTF ("%s HIT! %lu\n", cache[i].get_label(), address)
            return &cache[i];
        }
        else if (cache[i].lru < lru_cycle) {
            lru_cycle = cache[i].lru;
            lru_way = i;
        }
    }
    add_cache_misses();
    //ORCS_PRINTF ("%s MISS! %lu\n", cache[lru_way].get_label(), address)
    return &cache[lru_way];
}

void vima_controller_t::check_cache(){
    if (vima_buffer[0]->vima_read1 != 0){
        if (!read1->set) {
            if (VIMA_DEBUG) {
                ORCS_PRINTF ("READ1   -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_read1, get_index (vima_buffer[0]->vima_read1), get_tag (vima_buffer[0]->vima_read1))
                if (VIMA_UNBALANCED) ORCS_PRINTF ("READ1UB -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE - 1))
            }
            read1 = search_cache (vima_buffer[0]->vima_read1);
            if (get_index(read1->get_address()) == get_index(vima_buffer[0]->vima_read1) && get_tag(read1->get_address()) == get_tag(vima_buffer[0]->vima_read1)){
                read1->set_label ("READ1");
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ1 HIT!\n", vima_buffer[0]->vima_read1, get_index (vima_buffer[0]->vima_read1), get_tag (vima_buffer[0]->vima_read1))
            }
            else {
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ1 MISS!\n", vima_buffer[0]->vima_read1, get_index (vima_buffer[0]->vima_read1), get_tag (vima_buffer[0]->vima_read1))
                if (read1->status == PACKAGE_STATE_FREE) read1->status = PACKAGE_STATE_WAIT;
                else {
                    add_cache_writebacks();
                    read1->status = PACKAGE_STATE_TRANSMIT;
                }
                read1->set_address (vima_buffer[0]->vima_read1);
                read1->set_tag (get_tag (vima_buffer[0]->vima_read1));    
                read1->set_lru (orcs_engine.get_global_cycle());
            }
            read1->set = true;
        }
        if (VIMA_UNBALANCED){
            if (get_index(vima_buffer[0]->vima_read1) != get_index(vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1)){
                if (!read1_unbalanced->set) {
                    read1_unbalanced = search_cache (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1);
                    if (get_index(read1_unbalanced->get_address()) == get_index(vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1) && get_tag(read1_unbalanced->get_address()) == get_tag(vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1)){
                        read1_unbalanced->set_label ("READ1UB");
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ1UB HIT!\n", vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1, get_index (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1), get_tag (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1))
                    } else {
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ1UB MISS!\n", vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1, get_index (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1), get_tag (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1))
                        if (read1_unbalanced->status == PACKAGE_STATE_FREE) read1_unbalanced->status = PACKAGE_STATE_WAIT;
                        else {
                            add_cache_writebacks();
                            read1_unbalanced->status = PACKAGE_STATE_TRANSMIT;
                        }
                        read1_unbalanced->set_address (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1);
                        read1_unbalanced->set_tag (get_tag (vima_buffer[0]->vima_read1 + VIMA_VECTOR_SIZE -1));    
                        read1_unbalanced->set_lru (orcs_engine.get_global_cycle());
                    }
                    read1_unbalanced->set = true;
                }
            }
        }
    }

    if (vima_buffer[0]->vima_read2 != 0){
        if (!read2->set) {
            if (VIMA_DEBUG) {
                ORCS_PRINTF ("READ2   -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_read2, get_index (vima_buffer[0]->vima_read2), get_tag (vima_buffer[0]->vima_read2))
                if (VIMA_UNBALANCED) ORCS_PRINTF ("READ2UB -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1))
            }
            read2 = search_cache (vima_buffer[0]->vima_read2);
            if (get_index(read2->get_address()) == get_index(vima_buffer[0]->vima_read2) && get_tag(read2->get_address()) == get_tag(vima_buffer[0]->vima_read2)){
                read2->set_label ("READ2");
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ2 HIT!\n", vima_buffer[0]->vima_read2, get_index (vima_buffer[0]->vima_read2), get_tag (vima_buffer[0]->vima_read2))
            }
            else {
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ2 MISS!\n", vima_buffer[0]->vima_read2, get_index (vima_buffer[0]->vima_read2), get_tag (vima_buffer[0]->vima_read2))
                if (read2->status == PACKAGE_STATE_FREE) read2->status = PACKAGE_STATE_WAIT;
                else read2->status = PACKAGE_STATE_TRANSMIT;
                read2->set_address (vima_buffer[0]->vima_read2);
                read2->set_tag (get_tag (vima_buffer[0]->vima_read2));    
                read2->set_lru (orcs_engine.get_global_cycle());
            }
            read2->set = true;
        }
        if (VIMA_UNBALANCED){
            if (get_index(vima_buffer[0]->vima_read2) != get_index(vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1)){
                if (!read2_unbalanced->set) {
                    read2_unbalanced = search_cache (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1);
                    if (get_index(read2_unbalanced->get_address()) == get_index(vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1) && get_tag(read2_unbalanced->get_address()) == get_tag(vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1)){
                        read2_unbalanced->set_label ("READ2UB");
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ2UB HIT!\n", vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1))
                    }
                    else {
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu READ2UB MISS!\n", vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1))
                        if (read2_unbalanced->status == PACKAGE_STATE_FREE) read2_unbalanced->status = PACKAGE_STATE_WAIT;
                        else read2_unbalanced->status = PACKAGE_STATE_TRANSMIT;
                        read2_unbalanced->set_address (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1);
                        read2_unbalanced->set_tag (get_tag (vima_buffer[0]->vima_read2 + VIMA_VECTOR_SIZE - 1));    
                        read2_unbalanced->set_lru (orcs_engine.get_global_cycle());
                    }
                    read2_unbalanced->set = true;
                }   
            }
        }
    }

    if (vima_buffer[0]->vima_read1 != 0){
        if (read1->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read1_unbalanced->set){
            if (read1_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (vima_buffer[0]->vima_read2 != 0){
        if (read2->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read2_unbalanced->set){
            if (read2_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (vima_buffer[0]->vima_write != 0){
        if (!write->set) {
            if (VIMA_DEBUG) {
                ORCS_PRINTF ("WRITE   -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_write, get_index (vima_buffer[0]->vima_write), get_tag (vima_buffer[0]->vima_write))
                if (VIMA_UNBALANCED) ORCS_PRINTF ("WRITEUB -> address: %lu index %lu tag %lu\n", vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1))
            }
            write = search_cache (vima_buffer[0]->vima_write);
            if (get_index(write->get_address()) == get_index(vima_buffer[0]->vima_write) && get_tag(write->get_address()) == get_tag(vima_buffer[0]->vima_write)){
                write->set_label ("WRITE");
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu WRITE HIT!\n", vima_buffer[0]->vima_write, get_index (vima_buffer[0]->vima_write), get_tag (vima_buffer[0]->vima_write))
            }
            else {
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu WRITE MISS!\n", vima_buffer[0]->vima_write, get_index (vima_buffer[0]->vima_write), get_tag (vima_buffer[0]->vima_write))
                if (write->status == PACKAGE_STATE_FREE) write->status = PACKAGE_STATE_WAIT;
                else write->status = PACKAGE_STATE_TRANSMIT;
                write->set_address (vima_buffer[0]->vima_write);
                write->set_tag (get_tag (vima_buffer[0]->vima_write));    
                write->set_lru (orcs_engine.get_global_cycle());
            }
            write->set = true;
        }
        if (VIMA_UNBALANCED){
            if (get_index(vima_buffer[0]->vima_write) != get_index(vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1)){
                if (!write_unbalanced->set) {
                    write_unbalanced = search_cache (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1);
                    if (get_index(write_unbalanced->get_address()) == get_index(vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1) && get_tag(write_unbalanced->get_address()) == get_tag(vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1)){
                        write_unbalanced->set_label ("WRITEUB");
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu WRITEUB HIT!\n", vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1))
                    }
                    else {
                        if (VIMA_DEBUG) ORCS_PRINTF ("%lu index %lu tag %lu WRITEUB MISS!\n", vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1, get_index (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1), get_tag (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1))
                        if (write_unbalanced->status == PACKAGE_STATE_FREE) write_unbalanced->status = PACKAGE_STATE_WAIT;
                        else write_unbalanced->status = PACKAGE_STATE_TRANSMIT;
                        write_unbalanced->set_address (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1);
                        write_unbalanced->set_tag (get_tag (vima_buffer[0]->vima_write + VIMA_VECTOR_SIZE - 1));    
                        write_unbalanced->set_lru (orcs_engine.get_global_cycle());
                    }
                    write_unbalanced->set = true;
                }
            }
        }
    }        

    if (vima_buffer[0]->vima_write != 0) write->dirty = true;
    instruction_ready(0);
    //ORCS_PRINTF ("%lu instrução finalizada, %lu sujo!\n", vima_buffer[0]->uop_number, vima_buffer[0]->vima_write)
}

void vima_controller_t::clock(){
    read1->clock();
    read2->clock();
    write->clock();
    if (VIMA_UNBALANCED){
        read1_unbalanced->clock();
        read2_unbalanced->clock();
        write_unbalanced->clock();
    }

    if (vima_buffer.size() <= 0) return;
    switch (vima_buffer[0]->status){
        case PACKAGE_STATE_WAIT:
            ///ORCS_PRINTF ("%lu %s -> ", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            //vima_buffer[0]->status = PACKAGE_STATE_READY;
            ///ORCS_PRINTF ("%s %lu\n", get_enum_package_state_char (vima_buffer[0]->status), orcs_engine.get_global_cycle())
            instruction_ready (0);
            break;
        case PACKAGE_STATE_VIMA:
            check_cache();
            ///ORCS_PRINTF ("%lu %s -> ", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            break;
        default:
            ///ORCS_PRINTF ("%lu %s -> \n", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[0]), vima_buffer.end());
            break;
    }
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_VIMA_BUFFER (cfg_processor["VIMA_BUFFER"]);
    set_VIMA_DEBUG (cfg_processor["VIMA_DEBUG"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    set_VIMA_CACHE_SIZE (cfg_processor["VIMA_CACHE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_processor["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_processor["VIMA_CACHE_LATENCY"]);
    set_VIMA_UNBALANCED (cfg_processor["VIMA_UNBALANCED"]);

    this->set_lines (this->get_VIMA_CACHE_SIZE()/this->get_VIMA_VECTOR_SIZE());
    //uint64_t sets = lines/this->get_VIMA_CACHE_ASSOCIATIVITY();

    read1 = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    read1_unbalanced = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    read2 = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    read2_unbalanced = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    write = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    write_unbalanced = (vima_vector_t*) malloc (sizeof (vima_vector_t));

    this->cache = (vima_vector_t*) malloc (sizeof (vima_vector_t)*lines);
    std::memset ((void *)this->cache, 0, sizeof (vima_vector_t)*lines);

    for (size_t i = 0; i < lines; i++) this->cache[i].allocate();
    
    this->index_bits_shift = utils_t::get_power_of_two(this->get_VIMA_VECTOR_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(lines);

    uint64_t i;
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(lines); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }

    vima_op_latencies = utils_t::template_allocate_initialize_array<uint32_t>(MEMORY_OPERATION_VIMA_FP_MUL, 0);

    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = cfg_processor["VIMA_LATENCY_INT_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = cfg_processor["VIMA_LATENCY_INT_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = cfg_processor["VIMA_LATENCY_INT_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = cfg_processor["VIMA_LATENCY_FP_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = cfg_processor["VIMA_LATENCY_FP_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = cfg_processor["VIMA_LATENCY_FP_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = cfg_processor["VIMA_LATENCY_INT_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = cfg_processor["VIMA_LATENCY_FP_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);

    set_cache_accesses(0);
    set_cache_hits(0);
    set_cache_misses(0);
}

bool vima_controller_t::addRequest (memory_package_t* request){
    request->sent_to_ram = true;
    if (vima_buffer.size() < this->VIMA_BUFFER) {
        request->status = PACKAGE_STATE_VIMA;
        vima_buffer.push_back (request);
        return true;
    } else if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
    return false;
}
