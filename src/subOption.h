// subOption.h
#ifndef subOption_h
#define subOption_h

#include <Arduino.h>
#include <info.h>

class subOption {
  private:
    String itemName;
    info itemInfo;

  public:
    subOption(String name, const info structInfo);
    String getName();
    info getInfo();
};


#endif