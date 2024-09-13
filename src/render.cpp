#include "display.h"
#include "eadkpp.h"
#include "vector3.h"
#include "math.h"
#include "texture.h"
#include "world.h"
#include "ui.h"
#include "render.h"
#include "inventory.h"





// 1/255 (used to convert 255 -> 1)
const double inv255=1.0/255.0;

// render the could if set to true else no
bool renderCloud=false;
// find a idea how to make cloud rendering works.

// dda fast voxel alogrithm here
rayResult ray(Vector3 *rayPos, Vector3 *rayDir,double dayTime)
{
    unsigned char animOffset=(unsigned char)((unsigned char)(dayTime*4)&0b11);

    
    rayResult rayInfo;
    Vector3 color=Vector3();
    rayInfo.distance = 0;
    rayInfo.skyFactor=255;
    unsigned char previousBlockId = 0;
    bool isFirstBlock = true;
    bool hasTouchliquid=false;
    const signed char end[3] = {_round(rayPos->x + rayDir->x * 9),
                                _round(rayPos->y + rayDir->y * 9),
                                _round(rayPos->z + rayDir->z * 9)};

    signed char currentBlock[3] = {_round(rayPos->x), _round(rayPos->y), _round(rayPos->z)};
    signed char _currentBlock[3] = {currentBlock[0], currentBlock[1], currentBlock[2]};

    const unsigned char d[3] = {abs(end[0] - currentBlock[0]), abs(end[1] - currentBlock[1]), abs(end[2] - currentBlock[2])};
    
    unsigned char _axis;
    unsigned char axis = 0;

    signed char step[3] = {
        currentBlock[0] < end[0] ? 1 : -1,
        currentBlock[1] < end[1] ? 1 : -1,
        currentBlock[2] < end[2] ? 1 : -1};

    const double tDelta[3] = {
        abs(d[0]) > 1e-6 ? 1.0 / abs(rayDir->x) : 1e30,
        abs(d[1]) > 1e-6 ? 1.0 / abs(rayDir->y) : 1e30,
        abs(d[2]) > 1e-6 ? 1.0 / abs(rayDir->z) : 1e30};

    double tMax[3] = {
        tDelta[0] < 1e30 ? (currentBlock[0] + (step[0] > 0) - rayPos->x) / rayDir->x : 1e30,
        tDelta[1] < 1e30 ? (currentBlock[1] + (step[1] > 0) - rayPos->y) / rayDir->y : 1e30,
        tDelta[2] < 1e30 ? (currentBlock[2] + (step[2] > 0) - rayPos->z) / rayDir->z : 1e30};

    while (currentBlock[0] != end[0] || currentBlock[1] != end[1] || currentBlock[2] != end[2])
    {
        _currentBlock[0] = currentBlock[0];
        _currentBlock[1] = currentBlock[1];
        _currentBlock[2] = currentBlock[2];
        _axis=axis;
        axis=0;
        if (tMax[1] < tMax[axis])
            axis = 1;
        if (tMax[2] < tMax[axis])
            axis = 2;
        currentBlock[axis] += step[axis];
        tMax[axis] += tDelta[axis];

        if (currentBlock[1] > 15 || currentBlock[1] < 0 ||
            currentBlock[0] > 31 || currentBlock[0] < -32 ||
            currentBlock[2] > 31 || currentBlock[2] < -32)
        {     
            if (isFirstBlock){goto sky;}
            return rayInfo;
        }

        unsigned char blockId = getBlockFromWorld(currentBlock);
        if(blockId==12){
            hasTouchliquid=true;
        }
        if (blockId)
        {

            double m = 0;
            unsigned char face=0;
            unsigned char uv[2];
            if(axis==0){
                m = (_currentBlock[0] + (rayDir->x > 0) - rayPos->x) / rayDir->x;
                face =(rayDir->x > 0);
            }else if(axis==1){
                face = 2 + (rayDir->y > 0);
                m = (_currentBlock[1] + (rayDir->y > 0) - rayPos->y) / rayDir->y;
            }else{
                face = 4 + (rayDir->z > 0);
                m = (_currentBlock[2] + (rayDir->z > 0) - rayPos->z) / rayDir->z;
            }
            //unsigned char face=(_axis<<1)+(step[_axis]==-1);
            
            double pos[3] = {
                rayDir->x * m + rayPos->x,
                rayDir->y * m + rayPos->y,
                rayDir->z * m + rayPos->z};
            
            
            double rayFrac[3] = {(pos[0] <= 0) + (pos[0] - (double)((int)pos[0])),
                                 (pos[1] - (double)((int)pos[1])),
                                 (pos[2] <= 0) + (pos[2] - (double)((int)pos[2]))};

            /*faceX:
                uv[0] = (unsigned char)(rayFrac[2] * 8); // Z axis
                uv[1] = (unsigned char)(rayFrac[1] * 8); // Y axis
            faceY:
                uv[0] = (unsigned char)(rayFrac[0] * 8); // X axis
                uv[1] = (unsigned char)(rayFrac[2] * 8); // Z axis
            faceZ:
                uv[0] = (unsigned char)(rayFrac[0] * 8); // X axis
                uv[1] = (unsigned char)(rayFrac[1] * 8); // Y axis*/
            if (face == 0 || face == 1) // +X or -X face
            {
                uv[0] = (unsigned char)(rayFrac[2] * 8); // Z axis
                uv[1] = (unsigned char)(rayFrac[1] * 8); // Y axis
            }
            else if (face == 2 || face == 3) // +Y or -Y face
            {
                uv[0] = (unsigned char)(rayFrac[0] * 8); // X axis
                uv[1] = (unsigned char)(rayFrac[2] * 8); // Z axis
            }
            else // +Z or -Z face 
            {
                uv[0] = (unsigned char)(rayFrac[0] * 8); // X axis
                uv[1] = (unsigned char)(rayFrac[1] * 8); // Y axis
            }
            unsigned char opacity = getAlphaFromTexture(blockId, uv[0], uv[1], face,(blockId==12)? animOffset:0);
            if (isFirstBlock)
            {
                rayInfo.distance = m;
                
                

                
                rayInfo.blockId = blockId;
                rayInfo.face = face;
                rayInfo.blockPos[0] = currentBlock[0];
                rayInfo.blockPos[1] = currentBlock[1];
                rayInfo.blockPos[2] = currentBlock[2];
                
                if (previousBlockId != blockId){
                    if(blockId==12){
                        
                    }
                if (opacity == 255)
                {

                    //color = Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face));
                    rayInfo.color = color.add(Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face,(blockId==12)? animOffset:0))
                        .mul(rayInfo.skyFactor * inv255))
                        .toShortColor();
                    rayInfo.skyFactor = 0;
                    return rayInfo;
                }else if (opacity!=0)
                {
                    color = color.add(Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face,(blockId==12)? animOffset:0)).mul(opacity * inv255));
                    rayInfo.color=color.toShortColor();
                    rayInfo.skyFactor = (unsigned char)((1-rayInfo.skyFactor*inv255*opacity*inv255)*255);
                        
                }
                }
                
            }
            if (opacity != 0)
            {
                if (previousBlockId != blockId)
                {
                    // color.add(Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face))*);
                    // rayInfo.skyFactor = 0;

                    if (opacity == 255)
                    {

                        rayInfo.color = color.add(Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face,(blockId==12)? animOffset:0))
                        .mul(rayInfo.skyFactor * inv255))
                        .toShortColor();
                        rayInfo.skyFactor = 0;
                        return rayInfo;
                    }
                    else
                    {
                        color = color.add(Vector3(getPixelFromTexture(blockId, uv[0], uv[1], face,(blockId==12)? animOffset:0)).mul(rayInfo.skyFactor*inv255*opacity * inv255));
                        rayInfo.color=color.toShortColor();
                        rayInfo.skyFactor = (unsigned char)((rayInfo.skyFactor*inv255-rayInfo.skyFactor*inv255*opacity*inv255)*255);
                        
                    }
                }
            }
            isFirstBlock = false;
        }
        previousBlockId = blockId;
    }
    if (isFirstBlock){goto sky;}
    return rayInfo;
    
sky:
    rayInfo.distance = 9;
    rayInfo.blockId = 0;
    rayInfo.skyFactor = 255;
    return rayInfo;
}

/*
rayResult ray(Vector3 *rayPos, Vector3 *rayDir)
{
    rayResult rayInfo;
    Vector3 pos = {rayPos->x, rayPos->y, rayPos->z};
    Vector3 dir = {rayDir->x, rayDir->y, rayDir->z};
    signed char blockPos[3] = {(pos.x < 0) ? ((signed char)pos.x) - 1 : (signed char)pos.x,
                               (signed char)pos.y,
                               (pos.z < 0) ? ((signed char)pos.z) - 1 : (signed char)pos.z};

    Vector3 rayFrac = Vector3((pos.x <= 0) + (pos.x - (double)((int)pos.x)),
                              (pos.y <= 0) + (pos.y - (double)((int)pos.y)),
                              (pos.z <= 0) + (pos.z - (double)((int)pos.z)));

    Vector3 subChunkPos=Vector3((pos.x <= 0)*4+ (pos.x - (double)((int)(pos.x*0.25))*4),
                              (pos.y <= 0)*2 + (pos.y - (double)((int)(pos.y*0.5))*2),
                              (pos.z <= 0)*4 + (pos.z - (double)((int)(pos.z*0.25)))*4);
    const signed char faces[3][3] = {
        {(dir.x > 0.0) ? -1 : 1, 0, 0},
        {0, (dir.y > 0.0) ? -1 : 1, 0},
        {0, 0, (dir.z > 0.0) ? -1 : 1}};
    const double inverses[3] = {1 / dir.x, 1 / dir.y, 1 / dir.z};

    double m;
    double t;
    double distance = 0;
    unsigned char faceNormal;
    unsigned char lastBlockId=0;
    unsigned char blockId = 0;
    unsigned int blockIntersection=0;
    bool isFirstBlock = true;
    //(4-p)/x=t

    do
    {

                    m = ((dir.x > 0.0)- rayFrac.x) * inverses[0];
                    faceNormal = 0;
                    t = ((dir.y > 0.0) - rayFrac.y) * inverses[1];
                    if (t < m)
                    {
                        m = t;
                        faceNormal = 1;
                    }
                    t = ((dir.z > 0.0) - rayFrac.z) * inverses[2];
                    if (t < m)
                    {
                        m = t;
                        faceNormal = 2;
                    }
                    distance += m;
                    if (distance > 8)
                    {
                        rayInfo.distance = distance;
                        rayInfo.color=grayScale(15-blockIntersection,15);
                        return rayInfo;
                    }
                    pos=pos.add(dir.mul(m+0.001));
                    if (pos.y > 16 || pos.y < 0 || pos.x < -32 || pos.x > 32 || pos.z < -32 || pos.x > 32)
                    {
                        rayInfo.blockId = 0;
                        rayInfo.color=grayScale(15-blockIntersection,15);
                        return rayInfo;
                    }
                    rayFrac.x = (pos.x <= 0) + (pos.x - (double)((int)pos.x));
                    rayFrac.y = (pos.y - (double)((int)pos.y));
                    rayFrac.z = (pos.z <= 0) + (pos.z - (double)((int)pos.z));
                    signed char blockPos[3] = {(pos.x < 0) ? ((signed char)pos.x) - 1 : (signed char)pos.x,
                                                (signed char)pos.y,
                                                (pos.z < 0) ? ((signed char)pos.z) - 1 : (signed char)pos.z};
                    blockIntersection+=1;
                    blockId = getBlockFromWorld(blockPos);
                    if (blockId != 0)
                    {
                        unsigned char face = 0;
                        unsigned char uv[2];
                        if (faces[faceNormal][0] == 1)
                        {
                            uv[0] = (unsigned char)(rayFrac.z * 8);
                            uv[1] = (unsigned char)(rayFrac.y * 8);
                        }
                        else if (faces[faceNormal][0] == -1)
                        {
                            uv[0] = (unsigned char)(rayFrac.z * 8);
                            uv[1] = (unsigned char)(rayFrac.y * 8);
                            face = 1;
                        }
                        else if (faces[faceNormal][1] == 1)
                        {
                            uv[0] = (unsigned char)(rayFrac.x * 8);
                            uv[1] = (unsigned char)(rayFrac.z * 8);
                            face = 2;
                        }
                        else if (faces[faceNormal][1] == -1)
                        {
                            uv[0] = (unsigned char)(rayFrac.x * 8);
                            uv[1] = (unsigned char)(rayFrac.z * 8);
                            face = 3;
                        }
                        else if (faces[faceNormal][2] == 1)
                        {
                            uv[0] = (unsigned char)(rayFrac.x * 8);
                            uv[1] = (unsigned char)(rayFrac.y * 8);
                            face = 4;
                        }
                        else if (faces[faceNormal][2] == -1)
                        {
                            uv[0] = (unsigned char)(rayFrac.x * 8);
                            uv[1] = (unsigned char)(rayFrac.y * 8);
                            face = 5;
                        }
                        if (isFirstBlock)
                        {
                            rayInfo.blockId = blockId;
                            rayInfo.distance = distance;
                            rayInfo.blockPos[0] = (signed char)blockPos[0];
                            rayInfo.blockPos[1] = (signed char)blockPos[1];
                            rayInfo.blockPos[2] = (signed char)blockPos[2];
                            rayInfo.face = face;
                            unsigned char opacity = getAlphaFromTexture(blockId, uv[0], uv[1], face);
                            isFirstBlock=false;
                            //if (opacity == 255)
                            if (true)
                            {
                                rayInfo.color = getPixelFromTexture(blockId, uv[0], uv[1], face);
                                rayInfo.skyFactor = 0;
                                //rayInfo.color=grayScale(15-blockIntersection,15);
                                return rayInfo;
                            }

                        }else{
                        unsigned char opacity = getAlphaFromTexture(blockId, uv[0], uv[1], face);
                        if (opacity == 255 && lastBlockId!=blockId)
                        {
                            rayInfo.color = getPixelFromTexture(blockId, uv[0], uv[1], face);
                            rayInfo.skyFactor = 0;
                            //rayInfo.color=grayScale(15-blockIntersection,15);
                            return rayInfo;
                        }

                        }

                    }
                    lastBlockId=blockId;
    }while (distance < 8);
//rayInfo.color=grayScale(15-blockIntersection,15);
return rayInfo;
}*/

Vector3 getCloudColor(Vector3 rayDir, double time){
    if(rayDir.y<=0.3){
        return Vector3();
    }
    
    //EADK::random()
    
}

unsigned short grayScale(double x, double maxi)
{
    x = x / maxi;
    unsigned char gray = (unsigned char)(x * 255);
    return (((unsigned short)gray >> 3) << 11) | (((unsigned short)gray >> 2) << 5) | ((unsigned short)gray >> 3);
}

Vector3 getSkyColor(Vector3 rayDir, double time)
{
    // return Vector3();
    double morningScalar;
    double eveningScalar;
    // day top / day bottom / night top / night bottom (0,0,0);
    const Vector3 skyColors[4] = {{66, 135, 245}, {40, 40, 100}, {0, 7, 41}};

    // morning / day / evening + night
    const Vector3 sunColors[3] = {{255, 247, 186}, {248, 248, 230}, {255, 178, 77}};
    const Vector3 sunOutlineColors[3] = {{250, 255, 150}, {238, 238, 200}, {255, 159, 48}};

    Vector3 upperSkyColor, lowerSkyColor;
    Vector3 skyColor;
    Vector3 sunColor, sunOutlineColor;

    if (time >= 0 && time < 10)
    {
        morningScalar = (0.1 * (time));
        upperSkyColor = gradient(skyColors[2], skyColors[0], morningScalar);
        lowerSkyColor = skyColors[1].mul(morningScalar);
        sunColor = gradient(sunColors[0], sunColors[1], morningScalar);
        sunOutlineColor = gradient(sunOutlineColors[0], sunOutlineColors[1], morningScalar);
    }
    else if (time >= 40 && time < 50)
    {
        eveningScalar = time * 0.1 - 4;
        upperSkyColor = gradient(skyColors[0], skyColors[2], eveningScalar);
        lowerSkyColor = skyColors[1].mul(1 - eveningScalar);
        sunColor = gradient(sunColors[1], sunColors[2], eveningScalar);
        sunOutlineColor = gradient(sunOutlineColors[1], sunOutlineColors[2], eveningScalar);
    }
    else if (time >= 50)
    {
        upperSkyColor = skyColors[2];
        lowerSkyColor = Vector3();

        if (time >= 90)
        {
            sunColor = gradient(sunColors[2], sunColors[0], time * 0.1 - 9);
            sunOutlineColor = gradient(sunOutlineColors[2], sunOutlineColors[0], time * 0.1 - 9);
        }
        else
        {
            sunColor = sunColors[2];
            sunOutlineColor = sunOutlineColors[2];
        }
    }
    else
    {
        upperSkyColor = skyColors[0];
        lowerSkyColor = skyColors[1];
        sunColor = sunColors[1];
        sunOutlineColor = sunOutlineColors[1];
    }
    if (rayDir.y > 0)
    {
        skyColor = upperSkyColor;
    }
    else if (rayDir.y < -0.2)
    {
        skyColor = lowerSkyColor;
    }
    else
    {
        skyColor = gradient(lowerSkyColor, upperSkyColor, (double)((rayDir.y + 0.2) * 5));
    }

    const double rtime = time * 0.062893081761; // 0.006 = pi/50
    double sunDotProduct = rayDir.dot({0, sin(rtime), -cos(rtime)});

    if (sunDotProduct > 0.85 && time > 45 && time < 85)
    {
        double c = (1 - (1 - sunDotProduct) * 6.6666) * ((time < 55) ? (time * 0.1 - 4.5) : 1);
        skyColor = gradient(skyColor, Vector3(242, 83, 44), c * c);
    }
    if (sunDotProduct > 0.99)
    {
        return sunColor;
    }
    else if (sunDotProduct > 0.985)
    {
        return gradient(sunOutlineColor, skyColor, (0.99 - sunDotProduct) * 200);
    }
    return skyColor;
}