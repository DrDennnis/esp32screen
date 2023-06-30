// subOption.h
#ifndef subOption_h
#define subOption_h

#include <Arduino.h>

class subOption {
  private:
    String itemName;
    int itemMemoryAddressModifier;

  public:
    subOption(String name, int memoryAddressModifier);
    String getName();
    int getMemoryAddressModifier();
};

#endif