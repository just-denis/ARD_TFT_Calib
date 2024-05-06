// https://wiki.seeedstudio.com/2.8inch_TFT_Touch_Shield_v2.0/
// https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives

// #include 

#include "touch_screen_calib.h"

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
//#include <FreeDefaultFonts.h>

#define SCREEN_COLOR 0x0000 // BLACK
#define LINE_COLOR 0x8410 // 0xFFFF // WHITE
#define TEXT_COLOR 0xFFE0 // YELLOW
#define MINPRESSURE 10
#define STEP_X 20
#define STEP_Y 20
#define VCC_USB_MIN 4600 // 4813..4834  vs 3991..4110 // https://forum.arduino.cc/t/detecting-usb-power/118167
#define DELAY_MS 50
//==============================
// globals

MCUFRIEND_kbv tft;

//https://forum.arduino.cc/t/solved-mapping-tft-touchscreen/679255
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x9341

//const int XP=9,XM=A2,YP=A3,YM=8; 
// It is possible that YM, XP are on 8, 9. It is more common to be on 6, 7.
// https://forum.arduino.cc/t/lcd-touch-screen-not-updating/522907/4

const int TS_LEFT=224,TS_RT=901,TS_TOP=229,TS_BOT=900; //const int TS_LEFT=116,TS_RT=936,TS_TOP=68,TS_BOT=891;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// ~globals ====================

void setup() {
  Serial.begin(9600);

  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) {
    ID = 0x9486; // write-only shield
  }
  tft.begin(ID);
  tft.setRotation(0);            //PORTRAIT

  prep_tft();

  Serial.println(); Serial.println("[tft_calib]");
  Serial.print("width: "); Serial.print(tft.width()); Serial.print("\theight: "); Serial.println(tft.height());
} // ~setup

void prep_tft() {
  tft.fillScreen(SCREEN_COLOR);
  tft.setTextColor(LINE_COLOR);
  tft.setTextSize(1);

  for (short x = 0; x < tft.width(); x += STEP_X) {
    tft.drawFastVLine(x, 0, tft.height(), LINE_COLOR);
    tft.setCursor(x + 1, 0);
    tft.print(x);
  }

  for (short y = 0; y < tft.height(); y += STEP_Y) {
    tft.drawFastHLine(0, y, tft.width(), LINE_COLOR);
    tft.setCursor(0, y + 1);
    tft.print(y);
  }

  tft.setCursor(0, 20);
  //tft.setFont(&FreeSevenSegNumFont);
  tft.setTextColor(TEXT_COLOR);
  tft.setTextSize(1);
}

void loop() {
  // put your main code here, to run repeatedly:
  static int old_x = -1000, old_y = -1000;
  //int x, y;

  TSPoint p = ts.getPoint();
  CoordXY xy;
  xy.x = ts.readTouchX();
  xy.y = ts.readTouchY();  
  
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);

  pinMode(YM, OUTPUT);      //restore shared pins
  pinMode(XP, OUTPUT);
 
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);

  if (abs(old_x - xy.x) > 0 || abs(old_y - xy.y) > 0) {
    old_x = xy.x;
    old_y = xy.y;
    if (p.z > MINPRESSURE) { 
        //Serial.print("=touch= x: "); Serial.print(x);  Serial.print("\ty: "); Serial.print(y); Serial.println();
        //map_xy(x, y);
        
        long vcc = readVcc();
        bool isUSB = vcc > READVCC_USB_MIN;
        map_coords_USB_EXT(&xy, isUSB);

        if (tft.getCursorY() > tft.height() - 15) prep_tft();

        tft.setCursor(22, tft.getCursorY() + 2);
        tft.print(isUSB ? "U=" : "E="); tft.print(vcc); tft.print(", "); 
        tft.print("x: "); tft.print(xy.x); tft.print(", y: "); tft.print(xy.y); tft.print(", z: "); tft.print(p.z); 
        tft.println();
    }
  }
  //tft.println(readVcc());
  _delay_ms(DELAY_MS);
} //~loop

/*
// https://forum.arduino.cc/t/detecting-usb-power/118167
long readVcc1() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

#define AXN1 200L
#define AXD1 -610L
#define BX1 302L

#define AYN1 280L
#define AYD1 637L
#define BY1 -83L

#define AXN2 200L
#define AXD2 -600L
#define BX2 307L

#define AYN2 280L
#define AYD2 595L
#define BY2 -91L

void map_xy(long x, long y) {
  
  long x1 = BX1 + (AXN1 * x) / AXD1;
  long y1 = BY1 + (AYN1 * y) / AYD1;

  long x2 = BX2 + (AXN2 * x) / AXD2;
  long y2 = BY2 + (AYN2 * y) / AYD2;

  //Serial.print("temp1: "); Serial.print(temp1);
  Serial.print("(x1: "); Serial.print(x1); Serial.print(", y1: "); Serial.print(y1); Serial.print(")\t"); 
  Serial.print("(x2: "); Serial.print(x2); Serial.print(", y2: "); Serial.print(y2); Serial.print(")"); 
  Serial.println();

  tft.print("1:("); tft.print(x1); tft.print(", "); tft.print(y1); tft.print(") + ");
  tft.print("2:("); tft.print(x2); tft.print(", "); tft.print(y2); tft.print(") & ");
}
*/