#ifndef UOP_LINE_H
#define UOP_LINE_H

class uopLine_t{
    private:
        const int LINE_SIZE = 6;

    public:
        uint64_t tag;
        uint32_t instCounter;
        uint32_t uopCounter;
        uint32_t validUops;
        uint32_t instructionLength;
        int32_t nextLine; // if spans more then 1 line
        uop_package_t *uops;

        uopLine_t(){
            this->instCounter = 0;
            this->uopCounter = 0;
            this->validUops = 0;
            this->tag = -1;
            this->instructionLength = 0;
            this->nextLine = -1;
            uops = new uop_package_t[LINE_SIZE];
        }
};

#endif // UOP_LINE_H
