
struct info
{
    enum subOptionType
    {
        Type_Bool = 1,
        Type_Int = 2,
        Type_String = 3
    };
    bool isUpdateMemory;
    int memoryAddressModifier;
    subOptionType type; 
    info(bool updateMemory, int addressModifier, subOptionType optionType) : isUpdateMemory(updateMemory), memoryAddressModifier(addressModifier), type(optionType) {}
    info(bool updateMemory) : isUpdateMemory(updateMemory) {}
    info(){}
};

