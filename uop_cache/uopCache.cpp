#include "./../simulator.hpp"

// Constructor
uopCache_t::uopCache_t(){
    this->associativity = this->N_WAYS;
    this->n_sets = this->N_SETS;
    this->sets = new uopCacheSet_t[this->n_sets];
    this->fillBufferIndex = 0;
    this->waitBBEnd = false;
    this->fillBuffer = new uop_package_t[BUFFER_SIZE];
    this->currInstCount = 0;
    this->instToIgnore = 0;
    this->hitCounter = 0;
    this->missCounter = 0;
    this->decodeTimeSaved = 0;
}

void uopCache_t::fillBufferInsert(uop_package_t uop){
    //If waiting for a BB to end to restart buffering uops
    if(this->waitBBEnd == true){
        //if this uop belongs to a different instruction then the previous one, and the previous was a branch, the bb is over.
        if(uop.opcode_address != lastUop.opcode_address){
            uop_package_t tmp = lastUop;
            lastUop = uop;
            if(tmp.opcode_operation == INSTRUCTION_OPERATION_BRANCH){
                this->waitBBEnd = false;
                this->fillBufferIndex = 0;
            }
            else return;
        }
        else return;
    }

    //If it's the first uop, set the last Uop to itself
    if(this->fillBufferIndex == 0)
        lastUop = uop;

    //Check if the current uop is from a different instruction then the previous one
    if(uop.opcode_address != lastUop.opcode_address){
        //if the previous instruction was a branch
        if(lastUop.opcode_operation == INSTRUCTION_OPERATION_BRANCH){
            //If the basic block is over, send the buffer to the uop cache
            this->insert();
            this->fillBufferIndex = 0;
            this->currInstCount = -1;
        }
        this->lastUop = uop;
        this->currInstCount++;
    }

    //Check if the fillBuffer is full and the basic block isn't over
    if(this->fillBufferIndex == this->BUFFER_SIZE-1){
        this->waitBBEnd = true;
        this->currInstCount = 0;
        return;
    }

    this->fillBuffer[this->fillBufferIndex++] = uop;
}

void uopCache_t::insert(){
    uint32_t i, j;
    uint32_t destSet = fillBuffer[0].opcode_address & (this->N_SETS-1);
    uint32_t requiredLines = (int) ceil((this->fillBufferIndex / (double)this->LINE_SIZE));

    //printf("Inserindo no %d , %lu... %d uops de %d instruções\n",
    //    destSet, fillBuffer[0].opcode_address,this->fillBufferIndex, this->currInstCount);

    while(this->sets[destSet].freeLines < requiredLines)
        this->evict(destSet);

    int32_t pointerToSet = -1;
    int32_t uopIndex = 0;
    int32_t uopsToGo = this-> fillBufferIndex;
    int64_t tagToSet = fillBuffer[0].opcode_address;
    for(i = 0; i < this->associativity && requiredLines; i++){
        if(this->sets[destSet].lines[i].validUops == 0){
            for(j = 0; j < this->LINE_SIZE && uopIndex < uopsToGo; j++)
                this->sets[destSet].lines[i].uops[j] = this->fillBuffer[uopIndex++];
            this->sets[destSet].lines[i].validUops = j;
            if(pointerToSet != -1){
                this->sets[destSet].lines[pointerToSet].nextLine = i;
                if(uopsToGo > 0) pointerToSet = i;
            }
            else{
                this->sets[destSet].lines[i].tag = tagToSet;
                this->sets[destSet].lines[i].instCounter = this->currInstCount;
                this->sets[destSet].lines[i].uopCounter = this->fillBufferIndex;
            }
            requiredLines--;
            this->sets[destSet].freeLines--;
        }
    }
    //printf("Inseriu\n");
}

void uopCache_t::evict(uint32_t set){
    uint32_t i;
    bool done = false;

    //printf("Evictando...\n");

    for(i = 0; i < this->associativity && this->sets[set].lines[i].validUops == 0; i++);
    while(!done){
        this->sets[set].lines[i].instCounter = 0;
        this->sets[set].lines[i].uopCounter = 0;
        this->sets[set].lines[i].validUops = 0;
        this->sets[set].lines[i].tag = -1;
        if(this->sets[set].lines[i].nextLine != -1)
            i = this->sets[set].lines[i].nextLine;
        else
            done = true;
        this->sets[set].freeLines++;
    }
    //printf("Evictou\n");
    return;
}

void uopCache_t::statistics(){
    printf("\n-----------------ESTATISTICAS DA UOP CACHE:--------------------\n");
    printf("Processing cycles saved: %lu", this->decodeTimeSaved);
    printf("Cache Hit ratio: %.2lf\n", ((double)this->hitCounter/(this->hitCounter + this->missCounter))*100 );
}
