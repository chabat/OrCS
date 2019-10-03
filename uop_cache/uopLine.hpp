#ifndef UOP_LINE_H
#define UOP_LINE_H

class uopLine_t{
    private:
        const int LINE_SIZE = 6;

    public:
        uint64_t tag;
        uint32_t validUops;
        uint32_t instructionLength;
        uop_package_t *uops;

        uopLine_t(){
            this->validUops = 0;
            this->tag = 0;
            this->instructionLength = 0;
            uops = new uop_package_t[LINE_SIZE];
        }
}

#endif // UOP_LINE_H
