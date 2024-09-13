#ifndef INVENTORY_H
#define INVENTORY_H

#include "eadkpp.h"
void switchItem(unsigned char s1, unsigned s2, unsigned char *inventoryData);
void addItemToInventory(unsigned char slot, unsigned char id, unsigned char count, bool customSlot, unsigned char *inventoryData);

#endif