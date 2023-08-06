// subOption.h
#ifndef subOption_h
#define subOption_h

#include <Arduino.h>
#include <info.h>

class subOption
{
private:
  String itemName;

public:
  info itemInfo;
  subOption(String name, struct info structInfo);
  String getName();
  info getInfo();
};

#endif