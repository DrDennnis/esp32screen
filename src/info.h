// // info.h
// #ifndef info_h
// #define info_h

// #include "Arduino.h"

// class info
// {
// private:
//     /* data */
// public:
//     bool isUpdateMemory = false;
//     int memoryAddressModifier = 0;
// 	subOptionType type = Type_Bool;

//     info(bool updateMemory, int addressModifier, subOptionType optionType) {}
//     info(bool updateMemory) : isUpdateMemory(updateMemory) {}
//     info(){}
// };

// #endif

struct info
{
    enum subOptionType
    {
        Type_Bool = 1,
        Type_Int = 2,
        Type_String = 3,
    };
    bool isUpdateMemory;
    int memoryAddressModifier;
    subOptionType type;

    info(bool updateMemory, int addressModifier, const subOptionType &optionType) : isUpdateMemory(updateMemory), memoryAddressModifier(addressModifier), type(optionType) {}
    info(bool updateMemory) : isUpdateMemory(updateMemory) {}
    info() {}
};
