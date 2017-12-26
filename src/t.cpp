#include "esp.h"
#include "t.h"
#include "lcd.h"
#include "config.h"



byte addr[8];
byte data[12];
uint8_t sensor_step = 0;
unsigned long sensor_loop_timer = 0;

float t[24];




OneWire  ds(ds1820);

bool sensor_init()
{
  if ( ds.search(addr))
  {
    console("SENSOR","OK");
    tftConsole("SENSOR","OK");
    return true;
  }
  else
  {
    console("SENSOR","ERROR");
    tftConsole("SENSOR","ERROR");
    return false;
  }
}

void sensor_loop()
{
  switch (sensor_step)
  {
    case 0:
      if ((millis() - sensor_loop_timer) > 250)
      {
        ds.search(addr);
        ds.reset();
        sensor_step++;
      }
      break;
    case 1:
      if ((millis() - sensor_loop_timer) > 500)
      {
        ds.select(addr);
        ds.write(0x44, 1);
        sensor_step++;
      }
      break;
    case 2:
      if ((millis() - sensor_loop_timer) > 1500)
      {
        ds.reset();
        ds.select(addr);
        ds.write(0xBE);
        for (int i = 0; i < 9; i++)
          data[i] = ds.read();
        sensor_step = 0;
        sensor_loop_timer = millis();
      }
      break;
    default:
      break;
  }
  ct = (float)(((data[1] << 8) | data[0]) / 16.0);
}


void sensor_shift()
{
  for (uint8_t i = 0; i <= 22; i++)
    t[23 - i] = t[22 - i];
  if (ct > 40)
    t[0] = 40;
  else if (ct < 0)
    t[0] = 0;
  else
    t[0] = ct;
}


