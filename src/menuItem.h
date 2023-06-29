// menuItem.h
#ifndef menuItem_h
#define menuItem_h

#include <Arduino.h>

class option {
  private:
    String itemName;
    int itemMemoryAddress;
    void (*itemEmuDataTCallBack)();
    void (*itemValidateCallBack)();
    bool itemMainScreen = true;

  public:
    option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void),  bool mainScreen = true);
    String getName();
    void getEmuDataT();
    bool isActive();
    void setActive();
    void setInActive();
    void validate();
    bool isMainScreen();
};

#endif