// option.cpp

#include "option.h"
#include <EEPROM.h>

option::option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), subOption *subOptions, int subOptionCount) {
  itemName = name;
  itemMemoryAddress = memoryAddress;
  itemEmuDataTCallBack = emuDataTCallBack;
  itemValidateCallBack = validateCallBack;
  itemSubOptionCount = subOptionCount;
  itemSubOptions = subOptions;
  option::readMemoryData();
}


// void option::addSubOption(subOption* subOption){
//   itemSubOptions[sizeof(itemSubOptions) +1 ] = subOption;
// }

bool option::hasSubOption() {
  return itemSubOptionCount > 0;
}

bool option::isInRange(int count) {
  return itemSubOptionCount >= count;
}


subOption* option::getSubOptions() {
  return itemSubOptions;
}

int option::getSubOptionCount() {
  return itemSubOptionCount;
}


void option::updateMemory(int memoryAddresModifier, bool value)
{
  EEPROM.writeBool(itemMemoryAddress + memoryAddresModifier, value);
  EEPROM.commit();
}

void option::updateMemory(int memoryAddresModifier, int value)
{
  EEPROM.writeInt(itemMemoryAddress + memoryAddresModifier, value);
  EEPROM.commit();
}

void option::updateMemory(int memoryAddresModifier, String value)
{
  EEPROM.writeString(itemMemoryAddress + memoryAddresModifier, value);
  EEPROM.commit();
}

void option::updateMemory(int memoryAddresModifier, float value)
{
  EEPROM.writeFloat(itemMemoryAddress + memoryAddresModifier, value);
  EEPROM.commit();
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