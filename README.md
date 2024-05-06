# ARD_TFT_Calib
 A side project to calibrate TFT touch screen for Arduino UNO R3.
 In my case, the screen sensivity was a bit skewed, that is, slightly changing in a non-linear manner when you move from top-left corner to bottom-left, or - even worse - bottom-right corner of the screen. So some manual adjustments had to be introduced (see map_coords_USB_EXT in touch_screen_calib.h).
 And yes, .h files also contain code (with some hard-coded screen dimensions, to boot) - it happens.
On the bright side - the sketch supports both USB- and external power (the screen sensivity differs).