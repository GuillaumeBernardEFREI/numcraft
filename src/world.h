#ifndef WORLD_H
#define WORLD_H


struct chunk
{
    //chunk 16x16x16
    //subChunk 4x4x2
    chunk();
    chunk(signed char x,signed char z);
    signed char x,z;
    bool isGenerated=false;
    unsigned char data[4096]=""; 
    void generate(int seed);
    unsigned char getBlock(unsigned char *xyz) const;
    bool changeBlock(unsigned char *xyz,unsigned char blockId) ;
    unsigned long subchunk[2];

};
unsigned char getBlockFromWorld(signed char *xyz);
bool changeBlockInWorld(signed char *xyz,unsigned char blockId);


void initWorld();
#endif
