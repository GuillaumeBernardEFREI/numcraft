#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

struct texture
{
    unsigned short colorPalette[16];
    unsigned char pixelData[32];
    double outlineStrenght;
    unsigned char alphaPalette[4]={255};
    unsigned char alphaData[16]={0};
    unsigned short getPixel(unsigned char u,unsigned char v) const;
    unsigned char getAlpha(unsigned char u,unsigned char v) const;
};

unsigned char getAlphaFromTexture(unsigned char id,unsigned char u,unsigned char v,unsigned char face,unsigned char animOffset);
unsigned short getPixelFromTexture(unsigned char id,unsigned char u,unsigned char v,unsigned char face,unsigned char animOffset);
/*
0 air
1 grass
2 stone
3 dirt
4 log
5 glass
6 sand
*/

#endif
