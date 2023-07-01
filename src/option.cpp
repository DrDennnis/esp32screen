// option.cpp

#include "option.h"
#include <EEPROM.h>

option::option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), subOption *subOptions, bool mainScreen) {
  itemName = name;
  itemMemoryAddress = memoryAddress;
  itemEmuDataTCallBack = emuDataTCallBack;
  itemValidateCallBack = validateCallBack;
  itemMainScreen = mainScreen;
  itemSubOptions = subOptions;
  option::readMemoryData();
}


// void option::addSubOption(subOption* subOption){
//   itemSubOptions[sizeof(itemSubOptions) +1 ] = subOption;
// }

bool option::hasSubOption() {
  return sizeof(itemSubOptions) / sizeof(int) > 0;
}
subOption option::getSubOption(int i) {
  return itemSubOptions[i];
}

subOption* option::getSubOptions() {
  return itemSubOptions;
}

void option::readMemoryData()
{
  active = EEPROM.readBool(itemMemoryAddress);
  position = EEPROM.readInt(itemMemoryAddress+1);
}

bool option::isActive()
{
  return active;
}

int option::getPosition()
{
  return position;
}

void option::setActive()
{
  EEPROM.writeBool(itemMemoryAddress, true);
  EEPROM.commit();
}

void option::setInActive()
{
  EEPROM.writeBool(itemMemoryAddress, false);
  EEPROM.commit();
}

bool option::isMainScreen()
{
  return itemMainScreen;
}

String option::getName()
{
  return itemName;
}

void option::getEmuDataT()
{
  itemEmuDataTCallBack();
}

void option::validate()
{
  itemValidateCallBack();
}