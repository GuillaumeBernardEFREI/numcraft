#include "display.h"
#include "eadkpp.h"
#include "vector3.h"
#include "math.h"
#include "texture.h"
#include "world.h"
#include "ui.h"

extern const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "numcraft";
extern const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;
 
extern "C"
{
#include <eadk.h>
}
double max(double a, double b){
  return (a>b)? a:b;
}
double min(double a, double b){
  return (a<b)? a:b;
}
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

// struct used by ray()
struct rayResult
{
  unsigned char blockId = 0;
  unsigned char face;
  signed char blockPos[3] = {0, -2, 0};
  double distance;
  unsigned short color;
  unsigned char skyFactor = 0;
};

// struct of the player
struct Player
{
  Vector3 position = {0.1, 3, 1.1};
  Vector3 rotation = {0.1, 0.1, 0};
  unsigned char gamemode = 0;
  unsigned char selectedHotBarSlot = 2;
  unsigned char inventory[15][2] = {
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {4, 2},
      {3, 4},
      {2, 16},
      {1, 64},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}}

  ;
};

// render options
const unsigned char pixelSize = 1;
const float fov = 3.1415 / 2;
rayResult cursorRayResult;
const double timeFactor = 1;

// player info
bool isInventoryOpen = false;
unsigned char selectedSlotPos[2] = {1, 1};
// return a ray information conserning a ray
inline rayResult ray(Vector3 *rayPos, Vector3 *rayDir)
{
 Vector3 pos = {rayPos->x, rayPos->y, rayPos->z};
Vector3 dir = {rayDir->x, rayDir->y, rayDir->z};
  signed char blockPos[3] = {(pos.x < 0) ? ((signed char)pos.x) - 1 : (signed char)pos.x,
                             (signed char)pos.y,
                             (pos.z < 0) ? ((signed char)pos.z) - 1 : (signed char)pos.z};

  Vector3 rayFrac = Vector3((pos.x <= 0) + (pos.x - (double)((int)pos.x)),
                            (pos.y <= 0) + (pos.y - (double)((int)pos.y)),
                            (pos.z <= 0) + (pos.z - (double)((int)pos.z)));
  unsigned char uv[2];
  rayResult rayInfo;
  const signed char faces[3][3] = {
      {(dir.x > 0.0) ? -1 : 1, 0, 0},
      {0, (dir.y > 0.0) ? -1 : 1, 0},
      {0, 0, (dir.z > 0.0) ? -1 : 1}};
  double m;
  double t;
  unsigned char iteration = 0;
  double distance = 0;
  unsigned char faceNormal;

  const double inverses[3] = {1 / dir.x, 1 / dir.y, 1 / dir.z};
  //return rayInfo;
  do
  {

    m = ((faces[0][0] == -1) - rayFrac.x) * inverses[0];
    faceNormal = 0;
    t = ((faces[1][1] == -1) - rayFrac.y) * inverses[1];
    if (t < m)
    {
      m = t;
      faceNormal = 1;
    }
    t = ((faces[2][2] == -1) - rayFrac.z) * inverses[2];
    if (t < m)
    {
      m = t;
      faceNormal = 2;
    }

    distance += m + 0.0001;
    if (distance > 8)
    {
      rayInfo.distance = distance;
      return rayInfo;
    }
    pos.x+=dir.x*(m+0.001);
    pos.y+=dir.y*(m+0.001);
    pos.z+=dir.z*(m+0.001);
    if (pos.y > 16 || pos.y < 0 || pos.x < -32 || pos.x > 32 || pos.z < -32 || pos.x > 32 || blockPos[1] >= 16 || blockPos[1] <= 0)
    {
      rayInfo.blockId = 0;
      return rayInfo;
    }
    rayFrac.x = (pos.x <= 0) + (pos.x - (double)((int)pos.x));
    rayFrac.y = (pos.y - (double)((int)pos.y));
    rayFrac.z = (pos.z <= 0) + (pos.z - (double)((int)pos.z));
    signed char blockPos[3] = {(pos.x < 0) ? ((signed char)pos.x) - 1 : (signed char)pos.x,
                               (signed char)pos.y,
                               (pos.z < 0) ? ((signed char)pos.z) - 1 : (signed char)pos.z};
    unsigned char blockId;
    blockId=getBlockFromWorld(blockPos);
    if (blockId != 0)
    {
      rayInfo.blockId = blockId;
      rayInfo.distance = distance;
      unsigned char face = 0;
      uv[0] = (unsigned char)(rayFrac.z * 8);
      uv[1] = (unsigned char)(rayFrac.y * 8);
      if (faces[faceNormal][0] == -1)
      {
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
      rayInfo.blockPos[0] = (signed char)blockPos[0];
      rayInfo.blockPos[1] = (signed char)blockPos[1];
      rayInfo.blockPos[2] = (signed char)blockPos[2];
      rayInfo.color = getPixelFromTexture(blockId, uv[0], uv[1], face);
      double opacity = 
      rayInfo.face = face;
      return rayInfo;
    }
    iteration++;
  } while (distance < 8 && iteration < 13);
  return rayInfo;

    
}

double renderFrame(Player *_player, double dayTime, bool stopWithKey, unsigned char *hotBar,unsigned short left=0,unsigned char top=0,unsigned short width=320,unsigned char height=240)
{
  // render a frame where stopWith determine if a key is pressed if it should stop the render
  double xangleReset = -fov / 2;
  double xangleStep = fov / (320 / pixelSize);
  double yangleStep = fov / (320 / pixelSize);
  double xangle = xangleReset;
  double yangle = -xangle * 0.75;
  double eveningScalar;
  double morningScalar;
  bool alreadyChangeHotBar=false;
  bool isMorning = false;
  bool isEvening = false;
  const Vector3 skyColor[4] = {{40, 40, 100}, {66, 135, 245}, {0, 7, 41}};
  const Vector3 sunColor[3] = {{255, 247, 186}, {248, 248, 230}, {255, 178, 77}};
  const Vector3 sunOutlineColor[3] = {{250, 255, 150}, {238, 238, 200}, {255, 159, 48}};
  const uint64_t startTime = eadk_timing_millis();
  Vector3 rayDir;
  Vector3 upperSkyColor, lowerSkyColor;
  Vector3 sunColor_, sunOutlineColor_;
  const double rCos[2] = {cos((*_player).rotation.x), cos((*_player).rotation.y)};
  const double rSin[2] = {sin((*_player).rotation.x), sin((*_player).rotation.y)};
  const EADK::Keyboard::Key stopKeys[] = {EADK::Keyboard::Key::XNT,
                                          EADK::Keyboard::Key::Down,
                                          EADK::Keyboard::Key::Up,
                                          EADK::Keyboard::Key::Left,
                                          EADK::Keyboard::Key::Right,
                                          EADK::Keyboard::Key::Alpha,
                                          EADK::Keyboard::Key::Ln,
                                          EADK::Keyboard::Key::Backspace,
                                          EADK::Keyboard::Key::Power,
                                          EADK::Keyboard::Key::Toolbox,
                                          EADK::Keyboard::Key::Shift,
                                          EADK::Keyboard::Key::Comma,
                                          EADK::Keyboard::Key::Var,
                                          EADK::Keyboard::Key::Plus};
  Vector3 cursorDir = {rCos[1] * (xangleReset + (xangleStep * 160 / pixelSize)) + (yangle - (yangleStep * 120 / pixelSize)) * (rSin[1] * rSin[0]) + (rSin[1] * rCos[0]), (yangle - (yangleStep * 120 / pixelSize)) * (rCos[0]) - rSin[0], -rSin[1] * (xangleReset + (xangleStep * 160 / pixelSize)) + (yangle - (yangleStep * 120 / pixelSize)) * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])};
  rayResult cursorRayInfo = (ray(&((*_player).position), &cursorDir));
  if (cursorRayInfo.blockId == 0 || cursorRayInfo.distance > 4)
  {
    cursorRayInfo.blockPos[1] = -2;
  }
  cursorRayResult = cursorRayInfo;
  if (dayTime > 2.47492598692 && dayTime < 3.14159265359)
  {
    eveningScalar = (1.5 * (dayTime - 3.14159265359));
    eveningScalar *= eveningScalar;
    upperSkyColor = gradient(skyColor[2], skyColor[1], eveningScalar);
    sunColor_ = gradient(sunColor[2], sunColor[1], eveningScalar);
    sunOutlineColor_ = gradient(sunOutlineColor[2], sunOutlineColor[1], eveningScalar);
    lowerSkyColor = skyColor[0].mul(eveningScalar);
    isEvening = true;
  }
  else if (dayTime > 5.95)
  {
    morningScalar = (3 * (dayTime - 6.28318530718));
    morningScalar *= morningScalar;
    upperSkyColor = gradient(skyColor[1], skyColor[2], morningScalar);
    lowerSkyColor = skyColor[1].mul(1 - morningScalar);
    sunColor_ = gradient(sunColor[1], sunColor[0], morningScalar);
    sunOutlineColor_ = gradient(sunOutlineColor[1], sunOutlineColor[0], morningScalar);
    isMorning = true;
  }
  else if (dayTime < 2.47492598692)
  {
    sunColor_ = sunColor[1];
    sunOutlineColor_ = sunOutlineColor[1];
    upperSkyColor = skyColor[1];
    lowerSkyColor = skyColor[0];
  }
  else
  {
    upperSkyColor = skyColor[2];
    lowerSkyColor = Vector3(0, 0, 0);
    sunColor_ = sunColor[2];
    sunOutlineColor_ = sunOutlineColor[2];
  }

  for (unsigned char y = top; y < top+height; y += pixelSize)
  {
    EADK::Keyboard::State k= EADK::Keyboard::scan();

    if(!alreadyChangeHotBar){if (k.keyDown(EADK::Keyboard::Key::Seven))
    {

      *hotBar = 0;
      alreadyChangeHotBar=true;
      drawHotBar(&(_player->inventory[0][0]),_player->selectedHotBarSlot);
    }
    if (k.keyDown(EADK::Keyboard::Key::Eight))
    {
      *hotBar = 1;
      alreadyChangeHotBar=true;
      drawHotBar(&(_player->inventory[0][0]),_player->selectedHotBarSlot);
    }
    if (k.keyDown(EADK::Keyboard::Key::Nine))
    {
      *hotBar = 2;
      alreadyChangeHotBar=true;
      drawHotBar(&(_player->inventory[0][0]),_player->selectedHotBarSlot);
    }
    if (k.keyDown(EADK::Keyboard::Key::LeftParenthesis))
    {

      *hotBar = 3;
      alreadyChangeHotBar=true;
      drawHotBar(&(_player->inventory[0][0]),_player->selectedHotBarSlot);
    }
    if (k.keyDown(EADK::Keyboard::Key::RightParenthesis))
    {
      *hotBar = 4;
      alreadyChangeHotBar=true;
      drawHotBar(&(_player->inventory[0][0]),_player->selectedHotBarSlot);
    }}
    if (stopWithKey)
    {
      for (int key = 0; key < 14; key++)
      {
        if (k.keyDown(stopKeys[key]))
        {
          uint64_t endTime = eadk_timing_millis();
          double ComputeTime = abs((double)(startTime - endTime) * 0.001);
          return 1;
          return ComputeTime;
        }
      }
    }
    for (unsigned short x = left; x < left+width; x += pixelSize)
    {
      if(!((x>60) && (x<260) && (y>200)+(x+pixelSize-1>60) && (x+pixelSize-1<260) && (y+pixelSize-1>200))){
      Vector3 finalColor;

      if (((160 - x) * (160 - x)) + ((120 - y) * (120 - y)) < 16)
      {
        finalColor = Vector3(255,255,255);
      }
      else
      {

        rayDir = {rCos[1] * xangle + yangle * (rSin[1] * rSin[0]) + (rSin[1] * rCos[0]), (yangle * (rCos[0]) - rSin[0]), -rSin[1] * xangle + yangle * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])};
        rayDir = rayDir.normalize();

        rayResult rayInfo = ray((&(*_player).position), &rayDir);
        if (rayInfo.blockId != 0 && rayInfo.distance < 7.5)
        {
          Vector3 color = {(double)((rayInfo.color >> 11) * 8), (double)(((rayInfo.color >> 5) & 0b0000000000111111) * 4), (double)((rayInfo.color & 0b0000000000011111) * 8)};
          if (rayInfo.blockPos[0] == cursorRayInfo.blockPos[0] && rayInfo.blockPos[1] == cursorRayInfo.blockPos[1] && rayInfo.blockPos[2] == cursorRayInfo.blockPos[2])
          {
            color = color.add(Vector3(255, 255, 255)).mul(0.5);
          }
          if (dayTime > 3.1415)
          {
            color = color.mul(0.8);
          }
          if (isMorning)
          {
            color = color.mul(0.8 + (1 - morningScalar) * 0.2);
          }
          if (isEvening)
          {
            color = color.mul(0.8 + eveningScalar * 0.2);
          }
          finalColor=color;
          //EADK::Display::pushRectUniform(EADK::Rect(x, y, pixelSize, pixelSize), EADK::Color(color.toColor()));
        }
        else
        {
          Vector3 backgroundSkyColor;
          if (rayDir.y > 0)
          {
            backgroundSkyColor = upperSkyColor;
          }
          else if (rayDir.y < -0.2)
          {
            backgroundSkyColor = lowerSkyColor;
          }
          else
          {
            double c = (double)((rayDir.y + 0.2) * 5);
            backgroundSkyColor = gradient(lowerSkyColor, upperSkyColor, c);
          }
          double sunDotProduct = rayDir.dot({0, sin(dayTime), -cos(dayTime)});
          if (sunDotProduct > 0.85)
          {
            if (dayTime > 2.8 && dayTime < 5)
            {
              backgroundSkyColor = gradient(gradient(Vector3(242, 83, 44), backgroundSkyColor, (0.99 - sunDotProduct) * 7.142857), backgroundSkyColor, (1 - (dayTime - 2.8) > 0.5) ? 1 - (dayTime - 2.8) : 0.5);
            }
          }
          if (sunDotProduct > 0.99)
          {
            backgroundSkyColor = sunColor_;

          }
          else if (sunDotProduct > 0.985)
          {
            double sunBorderScalar = 1 - (200 * (0.99 - sunDotProduct));

            backgroundSkyColor = gradient(backgroundSkyColor, sunOutlineColor_, sunBorderScalar);

          }
          double moonDotProduct = rayDir.dot({0, -sin(dayTime), cos(dayTime)});
          if (moonDotProduct > 0.99 && rayDir.dot({0.12, -sin(dayTime), cos(dayTime)}) < 1.005)
          {
            backgroundSkyColor = Vector3(248, 248, 230);
            
          }
          finalColor=backgroundSkyColor;
          if (rayInfo.blockId != 0 && rayInfo.distance < 8)
          {
            Vector3 color = {(double)((rayInfo.color >> 11) * 8), (double)(((rayInfo.color >> 6) & 0b0000000000011111) * 8), (double)((rayInfo.color & 0b0000000000011111) * 4)};
            if (dayTime > 3.1415)
            {
              color = color.mul(0.8);
            }
            if (isMorning)
            {
              color = color.mul(0.8 + (1 - morningScalar) * 0.2);
            }
            if (isEvening)
            {
              color = color.mul(0.8 + eveningScalar * 0.2);
            }
            backgroundSkyColor = gradient(backgroundSkyColor, color, (8 - rayInfo.distance) * 2);
            finalColor=backgroundSkyColor;
          }
            
          //EADK::Display::pushRectUniform(EADK::Rect(x, y, pixelSize, pixelSize), EADK::Color(backgroundSkyColor.toColor()));
        }
      }
      
        EADK::Display::pushRectUniform(EADK::Rect(x, y, pixelSize, pixelSize), EADK::Color(finalColor.toColor()));
      }
      /*else if((x>=60) && (x<=260) && (y<200) && (x+pixelSize>=60) && (x+pixelSize<=260) && (y+pixelSize>=200))
      {
        EADK::Display::pushRectUniform(EADK::Rect(x, y,pixelSize,y+pixelSize-200), EADK::Color(finalColor.toColor()));
      }*/
      
      
      
      xangle += xangleStep;
    }
    xangle = xangleReset;

    yangle -= yangleStep;
  }
  uint64_t endTime = eadk_timing_millis();
  double ComputeTime = abs((double)(startTime - endTime) * 0.001);
  return 1;
  return ComputeTime;
}
int main(int argc, char *argv[])
{
  initWorld();
  double dayTime = 2;
  /*
  midnight :
  moring :
  day :
  afternoon:
  */
  bool discretMode = false;
  Player player;
  player.rotation = {0.1, 0.1, 0};
  player.position = {1.1, 4.2, 1.03};
  renderFrame(&player, 2, false, &player.selectedHotBarSlot);
  drawHotBar(&(player.inventory[0][0]),player.selectedHotBarSlot);

  while (1)
  {
    if (dayTime > 6.28318530718)
    {
      dayTime = 0;
    }
    EADK::Keyboard::State keyboardState = EADK::Keyboard::scan();
    if (!discretMode)
    {
      if (!isInventoryOpen)
      {
        dayTime += 0.03;
        renderFrame(&player, dayTime, true, &player.selectedHotBarSlot);
        // dayTime +=(renderFrame(&player, dayTime, true,&player.selectedHotBarSlot)*6.28318530718)*(1/300)*timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Left))
      {
        player.rotation.y -= 0.2;
        if (player.rotation.y < 0)
        {
          player.rotation.y = 6.27318530718;
        }
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Right))
      {
        player.rotation.y += 0.2;
        if (player.rotation.y > 6.28318530718)
        {
          player.rotation.y = 0.01;
        }

        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Up))
      {
        player.rotation.x -= 0.2;
        if (player.rotation.x < -1.57079632679)
        {
          player.rotation.x = -1.57079632679;
        }

        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Alpha))
      {
        if (cursorRayResult.blockPos[1] >= 0)
        {
          signed char blockPos[3] = {cursorRayResult.blockPos[0], cursorRayResult.blockPos[1], cursorRayResult.blockPos[2]};

          if (cursorRayResult.face == 1)
          {
            blockPos[0] += -1;
          }
          else if (cursorRayResult.face == 0)
          {
            blockPos[0] += 1;
          }
          if (cursorRayResult.face == 3)
          {
            blockPos[1] += -1;
          }
          else if (cursorRayResult.face == 2)
          {
            blockPos[1] += 1;
          }
          if (cursorRayResult.face == 5)
          {
            blockPos[2] += -1;
          }
          else if (cursorRayResult.face == 4)
          {
            blockPos[2] += 1;
          }
          if (player.inventory[10 + player.selectedHotBarSlot][0] != 0 && player.inventory[10 + player.selectedHotBarSlot][1] != 0)
          {

            changeBlockInWorld(blockPos, player.inventory[10 + player.selectedHotBarSlot][0]);
            player.inventory[10 + player.selectedHotBarSlot][1]--;
            if (player.inventory[10 + player.selectedHotBarSlot][1] == 0)
            {
              player.inventory[10 + player.selectedHotBarSlot][0] = 0;
            }
          }
        }
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Ln))
      {

        if (cursorRayResult.blockPos[1] >= 0)
        {

          addItemToInventory(0, getBlockFromWorld(cursorRayResult.blockPos), 1, false, player.inventory[0]);
          changeBlockInWorld(cursorRayResult.blockPos, 0);
        }
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Down))
      {
        player.rotation.x += 0.2;
        if (player.rotation.x > 1.57079632679)
        {
          player.rotation.x = 1.57079632679;
        }

        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Seven))
      {
        switchItem(10, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(0, 2, player.inventory[10][0], player.inventory[10][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Eight))
      {
        switchItem(11, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(1, 2, player.inventory[11][0], player.inventory[11][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Nine))
      {
        switchItem(12, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(2, 2, player.inventory[12][0], player.inventory[12][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::LeftParenthesis))
      {
        switchItem(13, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(3, 2, player.inventory[13][0], player.inventory[13][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::RightParenthesis))
      {
        switchItem(14, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(4, 2, player.inventory[14][0], player.inventory[14][1], true);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        EADK::Timing::msleep(200);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Backspace))
      {
        player.position.z += cos(player.rotation.y);
        player.position.x += sin(player.rotation.y);
        dayTime += 0.03;
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Power))
      {
        player.position.z -= cos(player.rotation.y);
        player.position.x -= sin(player.rotation.y);
        dayTime += 0.1;
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Toolbox))
      {
        player.position.y += 1;
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Comma))
      {
        player.position.y -= 1;
        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Var))
      {
        selectedSlotPos[0] = 1;
        selectedSlotPos[1] = 2;
        isInventoryOpen = !isInventoryOpen;
        if (isInventoryOpen)
        {
          draw_inventory(&(selectedSlotPos[0]), &(player.inventory[0][0]));
          drawHotBar(&(player.inventory[0][0]),player.selectedHotBarSlot);
          EADK::Timing::msleep(200);
        }
        else{
          renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
        }
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Right) && isInventoryOpen)
      {
        if (selectedSlotPos[0] < 4)
        {
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], false);
          selectedSlotPos[0]++;
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
          EADK::Timing::msleep(200);
        }
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Left) && isInventoryOpen)
      {
        if (selectedSlotPos[0] > 0)
        {
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], false);
          selectedSlotPos[0]--;
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
          EADK::Timing::msleep(200);
        }
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Up) && isInventoryOpen)
      {
        if (selectedSlotPos[1] > 0)
        {
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], false);
          selectedSlotPos[1]--;
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
          EADK::Timing::msleep(200);
        }
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Down) && isInventoryOpen)
      {
        if (selectedSlotPos[1] < 2)
        {
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], false);
          selectedSlotPos[1]++;
          drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
          EADK::Timing::msleep(200);
        }
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Shift))
      {

        if (player.rotation.y + 3.14159265359 > 6.28318530718)
        {
          player.rotation.y -= 3.14159265359;
        }
        else
        {
          player.rotation.y += 3.14159265359;
        }

        dayTime += (6.28318530718 * renderFrame(&player, dayTime, false, &player.selectedHotBarSlot)) * (1 / 300) * timeFactor;
      }
    }
    if (keyboardState.keyDown(EADK::Keyboard::Key::Minus))
    {
      break;
    }
    if (keyboardState.keyDown(EADK::Keyboard::Key::Plus))
    {
      discretMode = !discretMode;
      if (!discretMode)
      {
        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      else
      {
        EADK::Display::pushRectUniform(EADK::Rect(0, 0, 320, 18), EADK::Color(0xFFB531));
        EADK::Display::pushRectUniform(EADK::Rect(0, 18, 320, 192), EADK::Color(0xF6FAFF));
        EADK::Display::pushRectUniform(EADK::Rect(0, 205, 320, 35), EADK::Color(0xfffeff));
        EADK::Display::pushRectUniform(EADK::Rect(0, 205, 320, 2), EADK::Color(0xeceaec));
        // EADK::Display::drawString("rad",EADK::Point(5,3),false,EADK::Color(0xFFFFFF), EADK::Color(0xFFB531));
        // EADK::Display::drawString("CALCULS",EADK::Point(137,4),false,EADK::Color(0xFFFFFF), EADK::Color(0xFFB531));
      }

      EADK::Timing::msleep(400);
    }
  }
}
