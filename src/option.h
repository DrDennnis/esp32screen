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
    bool itemMainScreen = true;
    bool active = false;

  public:
    option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), subOption *subOptions,  bool mainScreen = true);
    int position;
    String getName();
    void getEmuDataT();
    void readMemoryData();
    bool isActive();
    void setActive();
    void setInActive();
    void validate();
    bool isMainScreen();
    int getPosition();
    subOption* itemSubOptions;
    // void addSubOption(subOption* subOption);
    bool hasSubOption();
    subOption getSubOption(int i);
    subOption* getSubOptions();
};

#endif