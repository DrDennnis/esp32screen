// menuItem.cpp

#include "menuItem.h"
#include <EEPROM.h>

option::option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), bool mainScreen) {
  itemName = name;
  itemMemoryAddress = memoryAddress;
  itemEmuDataTCallBack = emuDataTCallBack;
  itemValidateCallBack = validateCallBack;
  itemMainScreen = mainScreen;
}


bool option::isActive()
{
  return EEPROM.readBool(itemMemoryAddress);
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