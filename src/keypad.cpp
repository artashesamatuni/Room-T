#include "esp.h"
#include "keypad.h"
#include "config.h"


int keypad_loop_timer = 0;
uint16_t btn = 0;
uint8_t tbtn;


void key_loop()
{
  if ((millis() - keypad_loop_timer) > 10)
  {
    btn = analogRead(KEYBOARD);
    if (btn < 100)
    {
      keystatus = _KEY_DUMMY;
      tbtn = 0;
    }
    else if (btn < 200)
      if (tbtn == 5)
      {
        keystatus = _KEY_OK;
        tbtn = 0;
      }
      else
        tbtn++;
    else if (btn < 300)
      if (tbtn == 5)
      {
        keystatus = _KEY_LEFT;
        tbtn = 0;
      }
      else
        tbtn++;
    else if (tbtn == 5)
    {
      keystatus = _KEY_RIGHT;
      tbtn = 0;
    }
    else
      tbtn++;
    keypad_loop_timer = millis();
  }
  else
    keystatus = _KEY_DUMMY;
}



