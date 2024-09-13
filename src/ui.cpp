#include "display.h"
#include "eadkpp.h"
#include "ui.h"
#include "texture.h"
#include "vector3.h"
#include "math.h"

const unsigned char font[96][5] = {
{0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011111},
{0b11000110, 0b00110001, 0b10000000, 0b01100011, 0b00011111},
{0b11011110, 0b11110110, 0b00000000, 0b00000000, 0b00011111},
{0b01010111, 0b11111110, 0b10101111, 0b11111101, 0b01011111},
{0b00100111, 0b11110001, 0b11110001, 0b11111100, 0b10011111},
{0b10011101, 0b11001110, 0b11101110, 0b01110111, 0b00111111},
{0b01110010, 0b10011101, 0b11001011, 0b01011111, 0b10111111},
{0b11000110, 0b00110000, 0b00000000, 0b00000000, 0b00011111},
{0b01100011, 0b00110001, 0b10001100, 0b00110001, 0b10011111},
{0b11000110, 0b00011000, 0b11000110, 0b01100011, 0b00011111},
{0b11000110, 0b00000000, 0b00000000, 0b00000000, 0b00011111},
{0b00110001, 0b10111111, 0b11110011, 0b00011000, 0b00011111},
{0b00000000, 0b00000000, 0b00000000, 0b01100011, 0b00011111},
{0b00000000, 0b00111111, 0b11110000, 0b00000000, 0b00011111},
{0b00000000, 0b00000000, 0b00000000, 0b01100011, 0b00011111},
{0b00011001, 0b11001110, 0b11101110, 0b01110011, 0b00011111},
{0b11111111, 0b11110111, 0b10111101, 0b11111111, 0b11111111},
{0b00110111, 0b10111100, 0b01100011, 0b01111111, 0b11111111},
{0b11111111, 0b11000111, 0b11111110, 0b01111111, 0b11111111},
{0b11111111, 0b11000110, 0b11110001, 0b11111111, 0b11111111},
{0b11011110, 0b11110111, 0b11111111, 0b10001100, 0b01111111},
{0b11111111, 0b11110001, 0b11110011, 0b11111111, 0b11111111},
{0b11111111, 0b11110001, 0b11111101, 0b11111111, 0b11111111},
{0b11111111, 0b11000110, 0b00110001, 0b10001100, 0b01111111},
{0b11111111, 0b11110111, 0b11111101, 0b11111111, 0b11111111},
{0b11111111, 0b11110111, 0b11110001, 0b11111111, 0b11111111},
{0b00000110, 0b00110000, 0b00000000, 0b01100011, 0b00011111},
{0b00000110, 0b00110000, 0b00001100, 0b01100011, 0b00011111},
{0b00110011, 0b10111001, 0b10001110, 0b00111000, 0b11011111},
{0b00000000, 0b00111111, 0b11110000, 0b01111111, 0b11111111},
{0b11000111, 0b00011100, 0b01100111, 0b01110011, 0b00011111},
{0b11111111, 0b11000110, 0b01110000, 0b00011000, 0b11011111},
{0b11111111, 0b11100011, 0b01111011, 0b11000011, 0b11111111},
{0b11111111, 0b11110111, 0b11111111, 0b11101111, 0b01111111},
{0b11111111, 0b11110111, 0b11101101, 0b11111111, 0b11111111},
{0b11111111, 0b11110001, 0b10001100, 0b01111111, 0b11111111},
{0b11110111, 0b11110111, 0b10111101, 0b11111111, 0b11011111},
{0b11111111, 0b11110001, 0b11101100, 0b01111111, 0b11111111},
{0b11111111, 0b11110001, 0b11101111, 0b01100011, 0b00011111},
{0b11111111, 0b11110001, 0b10111101, 0b11111111, 0b11111111},
{0b11011110, 0b11111111, 0b11111101, 0b11101111, 0b01111111},
{0b11100111, 0b00010000, 0b10000100, 0b01110011, 0b10011111},
{0b00011000, 0b11000111, 0b10111101, 0b11111111, 0b11111111},
{0b11011110, 0b11111101, 0b11101101, 0b11101111, 0b01111111},
{0b11000110, 0b00110001, 0b10001100, 0b01111111, 0b11111111},
{0b11011111, 0b11101011, 0b01011000, 0b11000110, 0b00111111},
{0b11001111, 0b01111111, 0b11111101, 0b11100111, 0b00111111},
{0b11111111, 0b11110111, 0b10111101, 0b11111111, 0b11111111},
{0b11111111, 0b11110111, 0b11111111, 0b11100011, 0b00011111},
{0b11111111, 0b11110111, 0b10111101, 0b11111011, 0b10111111},
{0b11111111, 0b11110111, 0b11101111, 0b01101111, 0b01111111},
{0b11111111, 0b11111001, 0b11110001, 0b11111111, 0b11111111},
{0b11111111, 0b11011000, 0b11000110, 0b00110001, 0b10011111},
{0b11011110, 0b11110111, 0b10111101, 0b11111111, 0b11111111},
{0b11011110, 0b11110111, 0b10111101, 0b10111001, 0b11011111},
{0b10001100, 0b01100011, 0b01011010, 0b11111111, 0b01111111},
{0b11011110, 0b11001000, 0b01001101, 0b11101111, 0b01111111},
{0b11011110, 0b11111111, 0b11110010, 0b00010000, 0b10011111},
{0b11111111, 0b11001110, 0b11101110, 0b01111111, 0b11111111},
{0b11100111, 0b00110001, 0b10001100, 0b01110011, 0b10011111},
{0b11000111, 0b00111000, 0b11100011, 0b10011100, 0b01111111},
{0b11100111, 0b00011000, 0b11000110, 0b01110011, 0b10011111},
{0b01110111, 0b11110110, 0b00000000, 0b00000000, 0b00011111},
{0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b11111111},
{0b11000110, 0b00000000, 0b00000000, 0b00000000, 0b00011111},
{0b00000000, 0b00111110, 0b00111111, 0b11101111, 0b11111111},
{0b11000110, 0b00111111, 0b11111101, 0b11111111, 0b11111111},
{0b00000000, 0b00111111, 0b11111100, 0b01111111, 0b11111111},
{0b00011000, 0b11111111, 0b11111101, 0b11111111, 0b11111111},
{0b00000000, 0b00111111, 0b10111111, 0b11100011, 0b11111111},
{0b01110011, 0b10011001, 0b11101111, 0b00110001, 0b10011111},
{0b00000000, 0b00111111, 0b11111101, 0b11111100, 0b01111111},
{0b11000110, 0b00111111, 0b11111101, 0b11101111, 0b01111111},
{0b11000110, 0b00000001, 0b10001100, 0b01100011, 0b00011111},
{0b00011000, 0b11000000, 0b00110001, 0b11101111, 0b11111111},
{0b11000110, 0b00110111, 0b11111110, 0b01111111, 0b01111111},
{0b11000110, 0b00110001, 0b10001100, 0b01110011, 0b10011111},
{0b00000000, 0b00111111, 0b11111010, 0b11010110, 0b00111111},
{0b00000000, 0b00111111, 0b11111101, 0b11101111, 0b01111111},
{0b00000000, 0b00111111, 0b11111101, 0b11111111, 0b11111111},
{0b00000000, 0b00111111, 0b11111101, 0b11111111, 0b11111111},
{0b00000000, 0b00111111, 0b11111101, 0b11111111, 0b11111111},
{0b00000000, 0b00111111, 0b11111100, 0b01100011, 0b00011111},
{0b00000000, 0b00111111, 0b11001111, 0b10001111, 0b11111111},
{0b01100111, 0b10111100, 0b11000110, 0b00110001, 0b10011111},
{0b00000000, 0b00110111, 0b10111101, 0b11111111, 0b11111111},
{0b00000000, 0b00110111, 0b10111101, 0b11111101, 0b11011111},
{0b00000000, 0b00100011, 0b01011010, 0b11111111, 0b11111111},
{0b00000000, 0b00110111, 0b10110010, 0b01101111, 0b01111111},
{0b00000000, 0b00110111, 0b10111111, 0b11111100, 0b01111111},
{0b00000000, 0b00111111, 0b11110011, 0b00110011, 0b11111111},
{0b01100011, 0b00110001, 0b10001100, 0b00110001, 0b10011111},
{0b11000110, 0b00110001, 0b10001100, 0b01100011, 0b00011111},
{0b11000110, 0b00011000, 0b11000110, 0b01100011, 0b00011111},
{0b01100111, 0b11100110, 0b00000000, 0b00000000, 0b00011111},
{0b00000000, 0b00011100, 0b11101101, 0b11101111, 0b11111111}
};
void draw_string(signed char *text, EADK::Point pos, EADK::Color textColor, EADK::Color backgroundColor, unsigned char size, bool transparentBackground)
{
    int posx = 0;
    for (unsigned char i=0; i < 256; i++)
    {
        if (*(text + i) == 0)
        {
            return;
        }
        if (*(text + i)>=32 && *(text + i)<=127)
        {

            for (unsigned char j=0; j < 35; j++)
            {
                if (((font[*(text + i) - 32][j / 8]) >> (7 - j % 8)) & 1)
                {
                    EADK::Display::pushRectUniform(EADK::Rect(pos.x() + posx + size * (j % 5), pos.y() + size * (j / 5), size, size), textColor);
                }
                else if (!transparentBackground)
                {
                    EADK::Display::pushRectUniform(EADK::Rect(pos.x() + posx + size * (j % 5), pos.y() + size * (j / 5), size, size), backgroundColor);
                }
            }
            
            posx += size * 6;
            if(*(text + i+1) != 0 && !transparentBackground){
            EADK::Display::pushRectUniform(EADK::Rect(pos.x() + posx-size, pos.y(), size, size*7), backgroundColor);
            }
            
        }
        
        
        
        
    }
}

void draw_inventory(unsigned char *selectedSlotPos, unsigned char *inventoryData)
{
    EADK::Display::pushRectUniform(EADK::Rect(54, 34, 212, 132), EADK::Color(0x555555));
    EADK::Display::pushRectUniform(EADK::Rect(56, 36, 208, 128), EADK::Color(0xc6c6c6));
    EADK::Display::pushRectUniform(EADK::Rect(60, 40, 200, 120), EADK::Color(0xFFFFFF));
    for (int y = 2; y >= 0; y--)
    {
        for (int x = 4; x >= 0; x--)
        {
            drawCase(x, y, *((inventoryData + (x + 5 * y) * 2)), *((inventoryData + (x + 5 * y) * 2 + 1)), false);
        }
    }
    //drawHotBar(inventoryData);
}
void drawCase(unsigned char slotx, unsigned char sloty, unsigned char id, unsigned char count, bool selected)
{
    if (!selected)
    {
        EADK::Display::pushRectUniform(EADK::Rect(60 + slotx * 40, 40 + sloty * 40, 40, 40), EADK::Color(0xFFFFFF));
    }
    else
    {
        EADK::Display::pushRectUniform(EADK::Rect(60 + slotx * 40, 40 + sloty * 40, 40, 40), EADK::Color(0x646464));
    }

    EADK::Display::pushRectUniform(EADK::Rect(64 + slotx * 40, 44 + sloty * 40, 32, 32), EADK::Color(0x8b8b8b));

    if (id != 0)
    {
        drawItem(slotx, sloty, id);
        if(count>1 &&count<10){
            signed char countChar[1]={48+count};
            draw_string(countChar,EADK::Point(91+slotx*40,69+sloty*40),EADK::Color(0x0),EADK::Color(0x000000),1,true);
            draw_string(countChar,EADK::Point(89+slotx*40,67+sloty*40),EADK::Color(0x00FFFFFF),EADK::Color(0x000000),1,true);
        }
        else if (count>1 && count<=64)
        {
            signed char countChar[3]={'0'+count/10,'0'+(count%10), 0};
            draw_string(countChar,EADK::Point(85+slotx*40,69+sloty*40),EADK::Color(0x0),EADK::Color(0x000000),1,true);
            draw_string(countChar,EADK::Point(83+slotx*40,67+sloty*40),EADK::Color(0x00FFFFFF),EADK::Color(0x000000),1,true);
        }
        //drawItem(slotx,sloty+2,id);
    }
}
void drawHotBar(unsigned char *inventoryData,unsigned char selectedHotbarSlot){
    EADK::Display::pushRectUniform(EADK::Rect(60, 200,200, 40), EADK::Color(0x555555));
    EADK::Display::pushRectUniform(EADK::Rect(60+40*selectedHotbarSlot, 200,40, 40), EADK::Color(0xF0F0F0));
    for(unsigned char i=0; i<5;i++)
    {
        EADK::Display::pushRectUniform(EADK::Rect(64 + i * 40, 204, 32, 32), EADK::Color(0x8b8b8b));
        drawItem(i,4,*(inventoryData+20+i*2));
        unsigned char count = *(inventoryData+21+i*2);
        if(count>1 &&count<10){
            signed char countChar[2]={48+count,0};
            draw_string(countChar,EADK::Point(91+i*40,229),EADK::Color(0x0),EADK::Color(0x000000),1,true);
            draw_string(countChar,EADK::Point(89+i*40,227),EADK::Color(0x00FFFFFF),EADK::Color(0x000000),1,true);
        }
        else if (count>1 && count<=64)
        {
            signed char countChar[3]={'0'+count/10,'0'+(count%10), 0};
            draw_string(countChar,EADK::Point(85+i*40,229),EADK::Color(0x0),EADK::Color(0x000000),1,true);
            draw_string(countChar,EADK::Point(83+i*40,227),EADK::Color(0x00FFFFFF),EADK::Color(0x000000),1,true);
        }

    }
}
void drawItem(unsigned char slotx, unsigned char sloty, unsigned char id)
{
    if (id != 0)
    {

        for (unsigned char y = 0; y < 8; y++)
        {
            for (unsigned char x = 0; x < 8; x++)
            {
                unsigned short col = getPixelFromTexture(id, x, 7 - y, 4,0);
                Vector3 color = {(double)((col >> 11) * 8), (double)(((col >> 5) & 0b0000000000111111) * 4), (double)((col & 0b0000000000011111) * 8)};
                double alpha= getAlphaFromTexture(id, x, 7 - y, 4,0)*0.00392156862;// /divide by 255
                EADK::Display::pushRectUniform(EADK::Rect(64 + (slotx) * 40 + x * 4, 44 + (sloty) * 40 + y * 4, 4, 4), EADK::Color(gradient(Vector3(139,139,139),color,alpha).toColor()));
            }
        }
    }
}