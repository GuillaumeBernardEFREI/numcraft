#include "eadkpp.h"
#include "inventory.h"

void switchItem(unsigned char s1, unsigned s2, unsigned char *inventoryData)
{

  unsigned char tempItem[2] = {*(inventoryData + s1 * 2), *(inventoryData + s1 * 2 + 1)};
  *(inventoryData + s1 * 2) = *(inventoryData + s2 * 2);
  *(inventoryData + s1 * 2 + 1) = *(inventoryData + s2 * 2 + 1);
  *(inventoryData + s2 * 2) = tempItem[0];
  *(inventoryData + s2 * 2 + 1) = tempItem[1];
}

void addItemToInventory(unsigned char slot, unsigned char id, unsigned char count, bool customSlot, unsigned char *inventoryData)
{
  if (count < 1 || count > 64)
  {
    return;
  }
  if (customSlot)
  {
    *(inventoryData + slot * 2) = id;
    *(inventoryData + slot * 2 + 1) = count;
  }
  else
  {
    for (unsigned char i = 0; i < 5; i++)
    {
      if (*(inventoryData + (10 + i) * 2) == id && *(inventoryData + (10 + i) * 2 + 1) < 64)
      {
        if (*(inventoryData + (10 + i) * 2 + 1) + count <= 64)
        {
          *(inventoryData + (10 + i) * 2 + 1) += count;
          return;
        }
        else
        {
          unsigned char temp = 64 - (*(inventoryData + (10 + i) * 2 + 1));
          *(inventoryData + (10 + i) * 2 + 1) = 64;
          addItemToInventory(slot, id, count - temp, customSlot, inventoryData);
          return;
        }
      }
    }
    for (unsigned char i = 0; i < 10; i++)
    {
      if (*(inventoryData + i * 2) == id && *(inventoryData + i * 2 + 1) < 64)
      {
        if (*(inventoryData + i * 2 + 1) + count <= 64)
        {
          *(inventoryData + i * 2 + 1) += count;
          return;
        }
        else
        {
          unsigned char temp = 64 - (*(inventoryData + i * 2 + 1));
          *(inventoryData + i * 2 + 1) = 64;
          addItemToInventory(slot, id, count - temp, customSlot, inventoryData);
          return;
        }
      }
    }
    for (unsigned char i = 0; i < 5; i++)
    {
      if (*(inventoryData + (10 + i) * 2) == 0)
      {
        *(inventoryData + (10 + i) * 2) = id;
        *(inventoryData + (10 + i) * 2 + 1) = count;
        return;
      }
    }
    for (unsigned char i = 0; i < 10; i++)
    {
      if (*(inventoryData + i * 2) == 0)
      {
        *(inventoryData + i * 2) = id;
        *(inventoryData + i * 2 + 1) = count;
        return;
      }
    }
  }
}