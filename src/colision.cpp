#include "world.h"
#include "vector3.h"

const unsigned long IsBlockSolid=0b0;

1.8
0.8
0.8

//cameraPos << {0,0.5,0}
Vector3()

const Vector3 playerColisionPoint[12]={};
unsigned char getBlockFromWorldWithVector(Vector3 BlockPos){    
    BlockPos=getBlockPos(BlockPos);
    signed char BlockPos_[3]={(unsigned char)(BlockPos.x),(unsigned char)(BlockPos.y),(unsigned char)(BlockPos.z)};
    return getBlockFromWorld(&BlockPos_[0]);

}

Vector3 getBlockPos(Vector3 pos){
    return {(double)((pos.x<0)? (int)pos.x-1:(int)pos.x),(double)((pos.y<0)? (int)pos.y-1:(int)pos.y),(double)((pos.z<0)? (int)pos.z-1:(int)pos.z)};
}
bool getIsBlockSolid(unsigned char blockId){
    return (IsBlockSolid<<blockId)>>blockId;
}

bool isPlayerIntersectWithTerrain(Vector3 playerPos){
    unsigned char allBlockCOlliding[]={0};
    for (int i = 0; i < 0; i++)
    {
        if (getIsBlockSolid(allBlockCOlliding[i]))
        {
            return true;
        }
    }
    
    return false;
}

Vector3 MovePlayerWithColision(Vector3 playerPos,Vector3 force){
    Vector3 newPlayerPos=playerPos.add(force);
    return Vector3();
}