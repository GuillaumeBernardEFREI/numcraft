#include "display.h"
#include "eadkpp.h"
#include "vector3.h"
#include "math.h"
#include "texture.h"
#include "world.h"
#include "ui.h"
#include "render.h"
#include "inventory.h"

extern const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "numcraft";
extern const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

extern "C"
{
#include <eadk.h>
}

// struct of the player
struct Player
{
  Vector3 position = {0.1, 3, 1.1};
  Vector3 rotation = {0.1, 0.1, 0};
  unsigned char gamemode = 0;
  unsigned char selectedHotBarSlot = 2;
  unsigned char inventory[15][2] = {
      {1, 62},
      {2, 64},
      {3, 64},
      {4, 64},
      {5, 64},
      {6, 64},
      {7, 64},
      {8, 64},
      {9, 64},
      {10, 64},
      {11, 64},
      {12, 64},
      {0, 64},
      {0, 15},
      {0, 64}};
};

// render options
const unsigned char pixelSize = 4;

float fov = 3.83972435439 / 2;
rayResult cursorRayResult;
double timeFactor = 16;

// player info
bool isInventoryOpen = false;
unsigned char selectedSlotPos[2] = {1, 1};
// return a ray information conserning a ray
// x,y,z,p-> dir
// rayDir = c * xangle + yangle * c + c, yangle * c -c,c* xangle + yangle * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])};

// rayDir = {rCos[1] * xangle + yangle * (rSin[1] * rSin[0]) + (rSin[1] * rCos[0]), (yangle * (rCos[0]) - rSin[0]), -rSin[1] * xangle + yangle * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])};

// Xs,Ys
int get_pixel_pos_from_world_pos(Vector3 *_pos, Player *_player)
{
  unsigned int x = 0;
  unsigned int y = 0;
  Vector3 pos = _pos->sub(_player->position).normalize();

  const double rCos[2] = {cos((*_player).rotation.x), cos((*_player).rotation.y)};
  const double rSin[2] = {sin((*_player).rotation.x), sin((*_player).rotation.y)};

  Vector3 cameraPlaneNormal = {-(rSin[1] * rCos[0]), rSin[0], -(rCos[1] * rCos[0])};
  double t = abs(1 / cameraPlaneNormal.dot(pos));
  pos = pos.mul(t);
  double yangle = (pos.y + rSin[0]) / (rCos[0]);
  double xangle = (pos.x - (yangle * (rSin[1] * rSin[0]) + (rSin[1] * rCos[0]))) / rCos[1];
  // double xangle = (xangle-(yangle * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])))/(-rSin[1]);

  // double z = q_rsqrt(1-yangle*yangle-xangle*xangle);
  // yangle*=z;
  // xangle*=z;
  double xangleReset = -fov / 2;
  double xangleStep = fov / (320 / pixelSize);
  double yangleStep = fov / (320 / pixelSize);

  x = (xangle - xangleReset) / (xangleStep)*pixelSize;
  y = (yangle + xangleReset * 0.75) / (-yangleStep) * pixelSize;

  int xy = (((x / pixelSize) * pixelSize) << 16) | (((y / pixelSize) * pixelSize) & 0x0000FFFF);
  return xy;
}

void renderFrame(Player *_player, double dayTime, bool stopWithKey, unsigned char *hotBar, unsigned short left = 0, unsigned char top = 0, unsigned short width = 320, unsigned char height = 240)
{
  left=min(max(left,0),320);
  top=min(max(top,0),240);
  width=min(left+width,320)-left;
  height=min(top+height,240)-top;
  // render a frame where stopWithKey determine if a key is pressed if it should stop the render
  double xangleReset = -fov / 2;
  double xangleStep = fov / (320 / pixelSize);
  double yangleStep = fov / (320 / pixelSize);
  //double xangle = xangleReset+(left/pixelSize)*xangleStep;
  //double yangle = (-xangle * 0.75)-(top/pixelSize)*yangleStep;
  double xangle = xangleReset;
  double yangle = (-xangle * 0.75);
  
  bool alreadyChangeHotBar = false;
  Vector3 rayDir;
  
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

  for (unsigned char y = top; y < top + height; y += pixelSize)
  {
    EADK::Keyboard::State k = EADK::Keyboard::scan();

    if (!alreadyChangeHotBar)
    {
      if (k.keyDown(EADK::Keyboard::Key::Seven))
      {

        *hotBar = 0;
        alreadyChangeHotBar = true;
        drawHotBar(&(_player->inventory[0][0]), _player->selectedHotBarSlot);
      }
      if (k.keyDown(EADK::Keyboard::Key::Eight))
      {
        *hotBar = 1;
        alreadyChangeHotBar = true;
        drawHotBar(&(_player->inventory[0][0]), _player->selectedHotBarSlot);
      }
      if (k.keyDown(EADK::Keyboard::Key::Nine))
      {
        *hotBar = 2;
        alreadyChangeHotBar = true;
        drawHotBar(&(_player->inventory[0][0]), _player->selectedHotBarSlot);
      }
      if (k.keyDown(EADK::Keyboard::Key::LeftParenthesis))
      {

        *hotBar = 3;
        alreadyChangeHotBar = true;
        drawHotBar(&(_player->inventory[0][0]), _player->selectedHotBarSlot);
      }
      if (k.keyDown(EADK::Keyboard::Key::RightParenthesis))
      {
        *hotBar = 4;
        alreadyChangeHotBar = true;
        drawHotBar(&(_player->inventory[0][0]), _player->selectedHotBarSlot);
      }
    }
    if (stopWithKey)
    {
      for (int key = 0; key < 14; key++)
      {
        if (k.keyDown(stopKeys[key]))
        {
          if (stopKeys[key] == EADK::Keyboard::Key::Alpha || stopKeys[key] == EADK::Keyboard::Key::Alpha)
          {
            if (cursorRayInfo.blockId && cursorRayInfo.distance<4){
              return;
            }
          }else{return;}
        }
      }
    }
    for (unsigned short x = left; x < left + width; x += pixelSize)
    {
      if (!((x > 60) && (x < 260) && (y > 200) + (x + pixelSize - 1 > 60) && (x + pixelSize - 1 < 260) && (y + pixelSize - 1 > 200)))
      {
        Vector3 finalColor;

        if (((160 - x) * (160 - x)) + ((120 - y) * (120 - y)) < 16)
        {
          finalColor = Vector3(255, 255, 255);
        }
        else
        {

          rayDir = {rCos[1] * xangle + yangle * (rSin[1] * rSin[0]) + (rSin[1] * rCos[0]), (yangle * (rCos[0]) - rSin[0]), -rSin[1] * xangle + yangle * (rCos[1] * rSin[0]) + (rCos[1] * rCos[0])};
          rayDir = rayDir.normalize();
          rayResult rayInfo = ray((&(*_player).position), &rayDir,dayTime);
          Vector3 color;
          if (rayInfo.blockId != 0)
          {
            color = {(double)((rayInfo.color >> 11) * 8), (double)(((rayInfo.color >> 5) & 0b0000000000111111) * 4), (double)((rayInfo.color & 0b0000000000011111) * 8)};
            if(rayInfo.skyFactor){
              color = gradient(color,getSkyColor(rayDir, dayTime),rayInfo.skyFactor*0.00392156862);  
            }
          }

          if (rayInfo.blockId != 0 && rayInfo.distance < 7)
          {
            if (rayInfo.blockPos[0] == cursorRayInfo.blockPos[0] && rayInfo.blockPos[1] == cursorRayInfo.blockPos[1] && rayInfo.blockPos[2] == cursorRayInfo.blockPos[2])
            {
              if(rayInfo.blockId==12){
                color = color.add(Vector3(64, 64, 64)).mul(0.8);
              }
              else{
              color = color.add(Vector3(255, 255, 255)).mul(0.5);
              }
              
            }
          }
          else
          {
            if (rayInfo.blockId != 0 && rayInfo.distance < 8)
            {
              Vector3 skyColor = getSkyColor(rayDir, dayTime);
              color = gradient(skyColor, color, (8 - rayInfo.distance));
            }
            else
            {
              color = getSkyColor(rayDir, dayTime);
            }
          }
          //color = {(double)((rayInfo.color >> 11) * 8), (double)(((rayInfo.color >> 5) & 0b0000000000111111) * 4), (double)((rayInfo.color & 0b0000000000011111) * 8)};
          EADK::Display::pushRectUniform(EADK::Rect(x, y, pixelSize, pixelSize), EADK::Color(color.toColor()));
        }
      }
      xangle += xangleStep;
    }
    xangle = xangleReset;

    yangle -= yangleStep;
  }
}
int main(int argc, char *argv[])
{
  initWorld();
  double dayTime = 30;
  timeFactor=(double)((320/pixelSize)*(240/pixelSize))/4800.0;
  /*
  midnight : 60-100
  morning : 0-10
  day : 10-50
  afternoon: 50-60
  */
  bool discretMode = false;
  Player player;
  player.rotation = {0.1, 0.1, 0};
  player.position = {1.1, 4.2, 1.03};
  renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
  drawHotBar(&(player.inventory[0][0]), player.selectedHotBarSlot);

  while (1)
  {

    if (dayTime > 100)
    {
      dayTime = 0;
    }
    EADK::Keyboard::State keyboardState = EADK::Keyboard::scan();
    if (!discretMode)
    {
      if (!isInventoryOpen)
      {
        dayTime += 0.5 * timeFactor;
        renderFrame(&player, dayTime, true, &player.selectedHotBarSlot);
        //fov+=1;
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Left))
      {
        player.rotation.y -= 0.2;
        if (player.rotation.y < 0)
        {
          player.rotation.y = 6.27318530718;
        }
        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Right))
      {
        player.rotation.y += 0.2;
        if (player.rotation.y > 6.28318530718)
        {
          player.rotation.y = 0.01;
        }

        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Up))
      {
        player.rotation.x -= 0.2;
        if (player.rotation.x < -1.57079632679)
        {
          player.rotation.x = -1.57079632679;
        }

        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
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
            drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
          }
          renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
        }
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Ln))
      {
        if (cursorRayResult.blockPos[1] >= 0)
        {
          addItemToInventory(0, getBlockFromWorld(cursorRayResult.blockPos), 1, false, player.inventory[0]);
          changeBlockInWorld(cursorRayResult.blockPos, 0);
          renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
          drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        }
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Down))
      {
        player.rotation.x += 0.2;
        if (player.rotation.x > 1.57079632679)
        {
          player.rotation.x = 1.57079632679;
        }

        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Seven))
      {
        switchItem(10, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(0, 2, player.inventory[10][0], player.inventory[10][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Eight))
      {
        switchItem(11, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(1, 2, player.inventory[11][0], player.inventory[11][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Nine))
      {
        switchItem(12, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(2, 2, player.inventory[12][0], player.inventory[12][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::LeftParenthesis))
      {
        switchItem(13, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(3, 2, player.inventory[13][0], player.inventory[13][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        EADK::Timing::msleep(200);
      }
      if (isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::RightParenthesis))
      {
        switchItem(14, selectedSlotPos[0] + selectedSlotPos[1] * 5, &player.inventory[0][0]);
        drawCase(4, 2, player.inventory[14][0], player.inventory[14][1], false);
        drawCase(selectedSlotPos[0], selectedSlotPos[1], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][0], player.inventory[selectedSlotPos[0] + selectedSlotPos[1] * 5][1], true);
        drawHotBar(&player.inventory[0][0], player.selectedHotBarSlot);
        EADK::Timing::msleep(200);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Backspace))
      {
        player.position.z += cos(player.rotation.y);
        player.position.x += sin(player.rotation.y);
        dayTime += 0.03;

        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Power))
      {
        player.position.z -= cos(player.rotation.y);
        player.position.x -= sin(player.rotation.y);
        dayTime += 0.1;
        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Toolbox))
      {
        player.position.y += 1;
        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (!isInventoryOpen && keyboardState.keyDown(EADK::Keyboard::Key::Comma))
      {
        player.position.y -= 1;
        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
      if (keyboardState.keyDown(EADK::Keyboard::Key::Var))
      {
        selectedSlotPos[0] = 1;
        selectedSlotPos[1] = 2;
        isInventoryOpen = !isInventoryOpen;
        if (isInventoryOpen)
        {
          draw_inventory(&(selectedSlotPos[0]), &(player.inventory[0][0]));
          drawHotBar(&(player.inventory[0][0]), player.selectedHotBarSlot);
          EADK::Timing::msleep(200);
        }
        else
        {
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

        renderFrame(&player, dayTime, false, &player.selectedHotBarSlot);
      }
    }
    if (keyboardState.keyDown(EADK::Keyboard::Key::Minus))
    {
      break;
    }
    if (keyboardState.keyDown(EADK::Keyboard::Key::Six))
    {
      fov+=0.05;
      EADK::Timing::msleep(100);
    }
    if (keyboardState.keyDown(EADK::Keyboard::Key::Nine))
    {
      fov-=0.05;
      EADK::Timing::msleep(100);
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
