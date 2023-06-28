// menuItem.h
#ifndef menuItem_h
#define menuItem_h

#include <Arduino.h>

class option {
  private:
    String itemName;
    int itemMemoryAddress;
    void (*itemEmuDataTCallBack)();

  public:
    option(String name, int memoryAddress, void (*emuDataTCallBack)(void));
    String getName();
    void getEmuDataT();
    bool isActive();
    void setActive();
    void setInActive();
};

#endif