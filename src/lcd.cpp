#include "lcd.h"
#include "esp.h"
#include "config.h"
#include "ntp.h"
#include "keypad.h"


Ucglib_ST7735_18x128x160_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);
boolean draw = false;
float c_t;
uint8_t c_hh;
uint8_t c_mm;
uint8_t c_dw;
String mnt[] = {"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};
String dow[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

unsigned long loop_timer = -1;

void lcd_showClock(void);
void lcd_showCT(float ct);
void lcd_drawBar(int16_t value, String unit, uint16_t range, uint8_t t_size, uint8_t x, uint8_t y);
void lcd_drawIPbox(uint8_t *value, uint8_t x, uint8_t y);
void lcd_draw_menu(void);
uint8_t menu_scan(void);


void tftConsole(String a, String b)
{

}

void lcd_init(void)
{
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
}

void lcd_update(void)
{
  draw = true;
}

void lcd_loop(uint8_t screen)
{
  switch (screen) {
    case  _M_CLOCK:
      if (!menu_open) {
        //STATIC
        if (draw) {
          draw = false;
        }
        //DINAMIC
        if ((millis() - loop_timer) > 1000) {
          lcd_showClock();
          loop_timer = millis();
        }
      }
      break;
    case  _M_TEMP:
      if (!menu_open) {
        //STATIC
        if (draw) {
          draw = false;
        }
        //DINAMIC
        if ((millis() - loop_timer) > 1000) {
          lcd_showCT(ct);
          loop_timer = millis();
        }
      }
      break;
    case  _M_HOME:
      if (!menu_open) {
        //STATIC
        if (draw) {
          //tft.fillScreen(_BLACK);
          draw = false;
        }
        //DINAMIC
        if ((millis() - loop_timer) > 1000) {
          lcd_showCT(ct);
          lcd_drawBar(ct, "C", 100, 100, 14, 120);
          lcd_drawIPbox(ntp_ip, 14, 135);
          loop_timer = millis();
        }
      }
      break;
    default:
      break;
  }
  menu_scan();
  lcd_draw_menu();
}
void lcd_showCT(float ct) {
  char *tmpStr = "";
  if (c_t != ct)
  {
    ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
    int8_t d = (int8_t)c_t;
    uint8_t f = (uint8_t)abs((c_t - d) * 100);
    sprintf(tmpStr, "%02d", d);
    ucg_SetFont(ucg.getUcg(), ucg_font_logisoso30_tn);
    ucg_DrawString(ucg.getUcg(), 25, 90, 0, tmpStr);
    sprintf(tmpStr, "%02d", f);
    ucg_SetFont(ucg.getUcg(), ucg_font_logisoso16_tn);
    ucg_DrawString(ucg.getUcg(), 63, 90, 0, tmpStr);
    ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
    d = (int8_t)ct;
    f = (uint8_t)abs((ct - d) * 100);
    sprintf(tmpStr, "%02d", d);
    ucg_SetFont(ucg.getUcg(), ucg_font_logisoso30_tn);
    ucg_DrawString(ucg.getUcg(), 25, 90, 0, tmpStr);
    sprintf(tmpStr, "%02d", f);
    ucg_SetFont(ucg.getUcg(), ucg_font_logisoso16_tn);
    ucg_DrawString(ucg.getUcg(), 63, 90, 0, tmpStr);
    ucg_SetFont(ucg.getUcg(), ucg_font_logisoso30_tr);
    ucg_DrawString(ucg.getUcg(), 83, 90, 0, "C");
    ucg_DrawCircle(ucg.getUcg(), 78, 64, 3, 15);
    ucg_DrawCircle(ucg.getUcg(), 78, 64, 4, 15);
    c_t = ct;
  }
}

void lcd_showClock(void) {
  DateTime now = rtc.now();
  char *tmpStr = "";
  ucg_SetFont(ucg.getUcg(), ucg_font_chikita_tr);
  if (c_dw != now.day())
  {
    sprintf(tmpStr, "%s, %s %02d", dow[now.dayOfTheWeek()].c_str(), mnt[now.month() - 1].c_str(), now.day());
    ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
    ucg.drawBox(0, 90, ucg.getWidth(), 10);
    ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
    ucg_DrawString(ucg.getUcg(), 22, 97, 0, tmpStr);
    c_dw = now.day();
  }
  ucg_SetFont(ucg.getUcg(), ucg_font_logisoso30_tn);
  if (c_hh != now.hour())
  {
    sprintf(tmpStr, "%02d", c_hh);
    ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
    ucg_DrawString(ucg.getUcg(), 20, 90, 0, tmpStr);
    sprintf(tmpStr, "%02d", now.hour());
    ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
    ucg_DrawString(ucg.getUcg(), 20, 90, 0, tmpStr);
    c_hh = now.hour();
  }
  if (c_mm != now.minute())
  {
    sprintf(tmpStr, "%02d", c_mm);
    ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
    ucg_DrawString(ucg.getUcg(), 70, 90, 0, tmpStr);
    sprintf(tmpStr, "%02d", now.minute());
    ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
    ucg_DrawString(ucg.getUcg(), 70, 90, 0, tmpStr);
    c_mm = now.minute();
  }
  if (now.second() % 2)
    ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
  else
    ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
  ucg_DrawString(ucg.getUcg(), 58, 88, 0, ":");
}

void  lcd_drawBar(int16_t value, String unit, uint16_t range, uint8_t t_size, uint8_t x, uint8_t y) {
  uint8_t b_size;
  int16_t tmpVal;
  String tmpStr = "";

  tmpVal = value;
  if (tmpVal < 0)
    tmpVal = 100 + tmpVal;
  b_size = (t_size - 2) * tmpVal / range;

  if (b_size > t_size)
    b_size = t_size;
  ucg_SetColor(ucg.getUcg(), 0, 255, 255, 0);
  ucg.drawBox(x, y, t_size, 11);
  ucg_SetColor(ucg.getUcg(), 0, 100, 100, 100);
  ucg.drawBox(x + 1 + b_size, y + 1, t_size - 2 - b_size, 9);
  ucg_SetColor(ucg.getUcg(), 0, 0, 0, 255);
  ucg.drawBox(x + 1, y + 1, b_size, 9);
  ucg_SetColor(ucg.getUcg(), 0, 255, 255, 0);
  tmpStr = value;
  tmpStr += unit;
  ucg_SetFont(ucg.getUcg(), ucg_font_courB08_tr);
  ucg.setPrintPos(x + (t_size - tmpStr.length() * 6) / 2, y + 9);
  ucg.print(tmpStr);
}
void lcd_drawIPbox(uint8_t *value, uint8_t x, uint8_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  char *temp = "";
  ucg_SetColor(ucg.getUcg(), 0, 255, 255, 0);
  ucg.drawBox(x, y, 100, 11);
  ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
  ucg.drawBox(x + 1, y + 1, 98, 9);
  ucg_SetColor(ucg.getUcg(), 0, 255, 255, 0);
  sprintf(temp, "%03d.%03d.%03d.%03d", value[0], value[1], value[2], value[3]);
  ucg_SetFont(ucg.getUcg(), ucg_font_courB08_tr);
  ucg.setPrintPos(x + 5, y + 9);
  ucg.print(temp);
}

void lcd_draw_menu(void) {
  String m_item[] = {"Home", "Time", "CMS","MQTT", "Thermostat", "Access", "Network", "System", "Status"};
  bool dis = false;
  uint8_t elements = 9;
  if (draw)
  {
    if (menu_open)
    {
      ucg_SetColor(ucg.getUcg(), 0, 0, 255, 0);
      ucg.drawBox(0, 0, 128, 160);
      ucg_SetFont(ucg.getUcg(), ucg_font_courB08_tr);
      for (uint8_t i = 0; i < elements; i++)
      {
        if (i == menu_pos)
        {
          ucg_SetColor(ucg.getUcg(), 0, 100, 100, 100);
          ucg.drawBox(0, i * (160/elements), 128, 20);
          ucg_SetColor(ucg.getUcg(), 0, 255, 255, 255);
        }
        else
        {
          ucg_SetColor(ucg.getUcg(), 0, 0, 0, 0);
        }
        ucg.setPrintPos(2, (160/elements)-3 + i * (160/elements));
        ucg.print(m_item[i]);
      }
      draw = false;
    }
  }
}

uint8_t menu_scan(void) {
  if (keystatus == _KEY_OK)
  {
    if (!menu_open)
    {
      menu_open = true;
      menu_pos = 0;
      draw = true;
    }
    else
    {
      menu_open = false;
      draw = true;
      scr_pos = menu_pos;
    }
    return menu_pos;
  }
  if (keystatus == _KEY_LEFT)
  {
    draw = true;
    if (menu_pos > 0)
      menu_pos--;
    else
      menu_pos = 7;
    return menu_pos;
  }
  if (keystatus == _KEY_RIGHT)
  {
    draw = true;
    if (menu_pos < 7)
      menu_pos++;
    else
      menu_pos = 0;
    return menu_pos;
  }
  draw = false;
  return menu_pos;
}
