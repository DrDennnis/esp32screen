// option.cpp

#include "option.h"
#include <EEPROM.h>
#include <Preferences.h>

Preferences preferences;

option::option(String name, int memoryAddress, void (*emuDataTCallBack)(void), void (*validateCallBack)(void), subOption *subOptions, int subOptionCount)
{
  itemName = name;
  itemMemoryAddress = memoryAddress;
  itemEmuDataTCallBack = emuDataTCallBack;
  itemValidateCallBack = validateCallBack;
  itemSubOptionCount = subOptionCount;
  itemSubOptions = subOptions;
  option::readMemoryData();
}

option::option(void)
{
}

// void option::addSubOption(subOption* subOption){
//   itemSubOptions[sizeof(itemSubOptions) +1 ] = subOption;
// }

bool option::hasSubOption()
{
  return itemSubOptionCount > 0;
}

bool option::isInRange(int count)
{
  return itemSubOptionCount >= count;
}

subOption *option::getSubOptions()
{
  return itemSubOptions;
}

int option::getSubOptionCount()
{
  return itemSubOptionCount;
}

void option::updateMemory(String name, bool value)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, false);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  preferences.putBool(char_array, value);

  // EEPROM.writeBool(itemMemoryAddress + memoryAddresModifier, value);
  // EEPROM.commit();
  
  preferences.putBool("Value", value);
  preferences.end();
}

void option::updateMemory(String name, int value)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, false);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  preferences.putInt(char_array, value);
  preferences.end();
  // EEPROM.writeInt(itemMemoryAddress + memoryAddresModifier, value);
  // EEPROM.commit();
}

void option::updateMemory(String name, String value)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, false);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  preferences.putString(char_array, value);
  preferences.end();
  // EEPROM.writeString(itemMemoryAddress + memoryAddresModifier, value);
  // EEPROM.commit();
}

void option::updateMemory(String name, float value)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, false);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  preferences.putFloat(char_array, value);
  preferences.end();
  // EEPROM.writeFloat(itemMemoryAddress + memoryAddresModifier, value);
  // EEPROM.commit();
}

bool option::readMemoryDataBool(String name)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, true);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  return preferences.getBool(char_array);
  preferences.end();
  // return EEPROM.readBool(itemMemoryAddress + memoryAddresModifier);
}

int option::readMemoryDataInt(String name)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, true);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  return preferences.getInt(char_array);
  preferences.end();
  // return preferences.putInt(itemMemoryAddress + memoryAddresModifier);
}

float option::readMemoryDataFloat(String name)
{
  
  int itemNamestr_len = itemName.length() + 1; 
  char itemNameChar_array[itemNamestr_len];
  itemName.toCharArray(itemNameChar_array, itemNamestr_len);
  preferences.begin(itemNameChar_array, true);// option namespace

  int str_len = name.length() + 1; 
  char char_array[str_len];
  name.toCharArray(char_array, str_len);
  return preferences.getFloat(char_array);
  preferences.end();
  // return EEPROM.readFloat(itemMemoryAddress + memoryAddresModifier);
}

void option::readMemoryData()
{
  subOption *subOptions = option::getSubOptions();
  for (int i = 0; i < option::getSubOptionCount(); i++)
  {
    subOption subOption = subOptions[i];
    if (subOption.getName() == "Position")
    {
      position = option::readMemoryDataInt(subOption.getName());
    }
  }
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