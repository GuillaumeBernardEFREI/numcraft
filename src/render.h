#ifndef RENDER_H
#define RENDER_H
#include "vector3.h"
#include "display.h"
#include "eadkpp.h"
#include "vector3.h"
#include "math.h"
#include "texture.h"
#include "world.h"
#include "ui.h"
#include "render.h"

struct rayResult
{
  unsigned char blockId = 0;
  unsigned char face;
  signed char blockPos[3] = {0, -10, 0};
  double distance=9;
  unsigned short color;
  unsigned char skyFactor = 0;
};
rayResult ray(Vector3 *rayPos, Vector3 *rayDir,double dayTime=0);
Vector3 getSkyColor(Vector3 rayDir, double time);
unsigned short grayScale(double x,double maxi);
#endif