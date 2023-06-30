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
    option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void),  bool mainScreen = true);
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
    subOption* itemSubOptions[20];
    void addSubOption(subOption* subOption);
    subOption getSubOption(int i);
};

#endif