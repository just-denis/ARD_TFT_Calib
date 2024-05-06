/*
************************
v 0.1 2024-04-25.11.15


= Useful links:
Checking USB vs external power for Arduino
https://forum.arduino.cc/t/detecting-usb-power/118167

= Functions:
short map_coord(short x, MappingParams *mp)
  ! does NOT change mp 

long readVcc()
  ! expected to return > 4500 on USB-powered Arduino Uno

************************
*/

#ifndef TOUCH_SCREEN_CALIB_H_
#define TOUCH_SCREEN_CALIB_H_ 1

#ifndef SAMPLE_CALIB
#define SAMPLE_CALIB 1
#endif

// STRUCTURES

struct MappingParams {
  long an;
  long ad;
  long b;
};

struct CoordXY {
  short x;
  short y;
};

// IMPLEMENTATION

short map_coord(short z, const MappingParams *mp) {
  long result = z;
  result = mp->b + (mp->an * result) / mp->ad;
  return (short)result;
} //~map_coord

// https://forum.arduino.cc/t/detecting-usb-power/118167
long readVcc() {
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

#ifdef SAMPLE_CALIB

#define READVCC_USB_MIN 4500
const MappingParams TS_MP_X_USB = { 200L, -600L, 307L };
const MappingParams TS_MP_Y_USB = { 280L, 595L, -91L };

const MappingParams TS_MP_X_EXT = { 200L, -660L, 273L };
const MappingParams TS_MP_Y_EXT = { 280L, 665L, -37L };

void map_coords_USB_EXT(CoordXY *xy, bool isUSB) {
  xy->x = map_coord(xy->x, isUSB ? &TS_MP_X_USB : &TS_MP_X_EXT);
  xy->y = map_coord(xy->y, isUSB ? &TS_MP_Y_USB : &TS_MP_Y_EXT);

  // extra correction, as y coord gets a bit bigger around left-bottom corner of the screen, up to 320+ per formulas vs 300, actual; slighly better with USB
  long yy = xy->y;
  yy = yy * abs(240L - xy->x) / 240L;
  yy = yy / (isUSB ? 15L : 13L);
  xy->y = xy->y - yy;
}

long readVccSamples(int samples = 10, int delay_ms = 0) {
  long result = 0L;
  if (samples < 2) samples = 10;
  for (int i = 0; i < samples; i++) {
    result += readVcc();
    if (delay_ms > 0) delay(delay_ms);
  }

  result = result / (long)samples;
  return result;
}

bool checkIsUSB(int samples = 10, int usb_min = READVCC_USB_MIN, int delay_ms = 0) {
  if (usb_min <= 0) usb_min = READVCC_USB_MIN;
  return (readVccSamples(samples, delay_ms) > usb_min);
}

#endif


#endif // main