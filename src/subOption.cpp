// subOption.cpp

#include "subOption.h"
#include <EEPROM.h>

subOption::subOption(String name, int memoryAddressModifier) {
  itemName = name;
  itemMemoryAddressModifier = memoryAddressModifier;
}

String subOption::getName()
{
  return itemName;
}

int subOption::getMemoryAddressModifier()
{
  return itemMemoryAddressModifier;
}

