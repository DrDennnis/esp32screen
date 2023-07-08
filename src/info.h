struct info
{
    bool isUpdateMemory;
    int memoryAddressModifier;
    int type; 
    info(bool updateMemory, int addressModifier, int optionType) : isUpdateMemory(updateMemory), memoryAddressModifier(addressModifier), type(optionType) {}
    info(bool updateMemory) : isUpdateMemory(updateMemory) {}
    info(){}
};

