// subOption.cpp

#include "subOption.h"

subOption::subOption(String name, info structInfo) {
  itemName = name;
  itemInfo = structInfo;
}

String subOption::getName()
{
  return itemName;
}

info subOption::getInfo()
{
  return itemInfo;
}
