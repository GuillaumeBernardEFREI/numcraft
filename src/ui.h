
#ifndef UI_H
#define UI_H

void draw_inventory(unsigned char * selectedSlotPos,unsigned char * inventoryData);
void drawCase(unsigned char slotx, unsigned char sloty, unsigned char id,unsigned char count,bool selected);
void drawItem(unsigned char slotx,unsigned char sloty,unsigned char id);
void draw_string(signed char *text, EADK::Point pos, EADK::Color textColor, EADK::Color backgroundColor, unsigned char size, bool transparentBackground);
void drawHotBar(unsigned char *inventoryData,unsigned char selectedHotbarSlot);
#endif 