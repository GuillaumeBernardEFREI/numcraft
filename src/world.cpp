#include "world.h"
#include "eadkpp.h"
#include "math.h"
chunk world[16];

chunk::chunk() : x(0), z(0) {}

chunk::chunk(signed char x, signed char z) : x(x), z(z) {}

void initWorld()
{

    for (signed char x = -2; x < 2; x++)
    {
        for (signed char y = -2; y < 2; y++)
        {
            world[(x + 2) * +3 * (y + 2)] = chunk(x, y);
        }
    }
}

void chunk::generate(int seed)
{
    isGenerated = true;
    for (int y = 0; y < 16; y++)
    {
        for (int z = 0; z < 16; z++)
        {
            for (int x = 0; x < 16; x++)
            {
                if (y < 2)
                {
                    if(y==1){
                        if(max(min((EADK::random() & 0b111) + 1, 8), 0)<3){
                        data[x + z * 16 + y * 256] = 1;
                        }
                        else{
                            data[x + z * 16 + y * 256] = 12;
                        }
                    }
                    else{
                        data[x + z * 16 + y * 256] = 1;
                    }
                }
                else
                {
                    data[x + z * 16 + y * 256] = 0;
                }
            }
        }
    }
    unsigned char treexz[2] = {randint(3, 11), randint(3, 11)};
    for (int i = 0; i < 2; i++)
    {
        for (int x = -2; x < 3; x++)
        {
            for (int z = -2; z < 3; z++)
            {
                data[treexz[0] + x + (treexz[1] + z) * 16 + (5 + i) * 256] = 8;
            }
        }
        for (int x = -1; x < 2; x++)
        {
            for (int z = -1; z < 2; z++)
            {
                data[treexz[0] + x + (treexz[1] + z) * 16 + (7) * 256] = 8;
                if ((abs(x) + abs(z)) < 2)
                    data[treexz[0] + x + (treexz[1] + z) * 16 + (8) * 256] = 8;
            }
        }
    }
    for (int i = 0; i < 6; i++)
    {
        data[treexz[0] + treexz[1] * 16 + (2 + i) * 256] = 3;
    }
}
unsigned char chunk::getBlock(unsigned char *xyz) const
{
    return data[(*xyz) + (*(xyz + 2)) * 16 + (*(xyz + 1)) * 256];
}

//change a block in the chunk, output true if its succesfull else false
bool chunk::changeBlock(unsigned char *xyz, unsigned char blockId)
{

    data[(*xyz) + (*(xyz + 2)) * 16 + (*(xyz + 1)) * 256] = blockId;
    if (blockId != 0)
    {
        subchunk[*(xyz + 1) > 7] = subchunk[*(xyz + 1) > 7] | (((unsigned long)1) << ((*xyz >> 2) + (*(xyz + 2) >> 2) * 4 + (*(xyz + 1) >> 1) * 16));
    }
    else
    {
        unsigned char subchunkpos[3] = {(*xyz >> 2), (*(xyz + 1) >> 1), (*(xyz + 2) >> 2)};

        bool isSubchunkEmpty=true;
        for (int y = 0; y < 2; y++)
        {
            for (int z = 0; z < 4; z++)
            {
                for (int x = 0; x < 4; x++)
                {
                    unsigned char testBlockPos[3]={*(subchunkpos)+x,*(subchunkpos + 1)+y,*(subchunkpos+2)+z};
                    if(getBlock(testBlockPos)){
                        isSubchunkEmpty=false;
                    }
                }
            }
        }
        if(isSubchunkEmpty){
            subchunk[*(xyz + 1) > 7] = subchunk[*(xyz + 1) > 7] & (~(((unsigned long)1) << ((*xyz >> 2) + (*(xyz + 2) >> 2) * 4 + (*(xyz + 1) >> 1) * 16)));
        }
        else{
            subchunk[*(xyz + 1) > 7] = subchunk[*(xyz + 1) > 7] | (((unsigned long)1) << ((*xyz >> 2) + (*(xyz + 2) >> 2) * 4 + (*(xyz + 1) >> 1) * 16));
        }
        
        
    }
    return true;
}
unsigned char getBlockFromWorld(signed char *xyz)
{
    unsigned char chunkx = ((unsigned char)(*(xyz) + 32)) >> 4;
    unsigned char chunkz = ((unsigned char)(*(xyz + 2) + 32)) >> 4;
    unsigned char localBlockPos[3] = {(*xyz + 32) - 16 * (chunkx), *(xyz + 1), (*(xyz + 2) + 32) - 16 * (chunkz)};
    if (world[chunkx + chunkz * 3].isGenerated)
    {
        return (world[chunkx + chunkz * 3].getBlock(localBlockPos));
    }
    else
    {
        world[chunkx + chunkz * 3].generate(0);
        return (world[chunkx + chunkz * 3].getBlock(localBlockPos));
    }
}

bool changeBlockInWorld(signed char *xyz, unsigned char blockId)
{
    unsigned char chunkx = ((unsigned char)(*(xyz) + 32)) >> 4;
    unsigned char chunkz = ((unsigned char)(*(xyz + 2) + 32)) >> 4;
    unsigned char localBlockPos[3] = {(*xyz + 32) - 16 * (chunkx), *(xyz + 1), (*(xyz + 2) + 32) - 16 * (chunkz)};
    if (world[chunkx + chunkz * 3].isGenerated)
    {
        return (world[chunkx + chunkz * 3].changeBlock(localBlockPos, blockId));
    }
    else
    {
        world[chunkx + chunkz * 3].generate(0);
        return (world[chunkx + chunkz * 3].changeBlock(localBlockPos, blockId));
    }
}