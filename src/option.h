// option.h
#ifndef option_h
#define option_h

#include <Arduino.h>
#include "subOption.h"

class option {
  private:
    String itemName;
    int itemMemoryAddress;
    void (*itemEmuDataTCallBack)();
    void (*itemValidateCallBack)();
    int itemSubOptionCount = 0;
    bool active = false;
    subOption* itemSubOptions;

  public:
    option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), subOption *subOptions, int subOptionCount);
    int position;
    String getName();
    void getEmuDataT();
    void readMemoryData();
    bool isActive();
    void setActive();
    void setInActive();
    void validate();
    int getPosition();
    // void addSubOption(subOption* subOption);
    bool hasSubOption();
    bool isInRange(int i);
    subOption* getSubOptions();
    int getSubOptionCount();
    void updateMemory(int memoryAddresModifier, bool value);
    void updateMemory(int memoryAddresModifier, int value);
    void updateMemory(int memoryAddresModifier, String value);
    void updateMemory(int memoryAddresModifier, float value);
};

#endif