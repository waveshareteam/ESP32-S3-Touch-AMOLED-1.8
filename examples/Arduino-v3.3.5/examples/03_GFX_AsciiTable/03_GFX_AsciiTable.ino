#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"

HWCDC USBSerial;
// SensorPCF85063 rtc;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */);

void setup(void) {
  USBSerial.begin(115200);
  // USBSerial.setDebugOutput(true);
  // while(!USBSerial);
  USBSerial.println("Arduino_GFX AsciiTable example");

  int numCols = LCD_WIDTH / 8;
  int numRows = LCD_HEIGHT / 10;

#ifdef GFX_EXTRA_PRE_INIT
  GFX_EXTRA_PRE_INIT();
#endif

  // Init Display
  if (!gfx->begin()) {
    USBSerial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  gfx->setBrightness(255);

  gfx->setTextColor(RGB565_GREEN);
  for (int x = 0; x < numRows; x++) {
    gfx->setCursor(10 + x * 8, 2);
    gfx->print(x, 16);
  }
  gfx->setTextColor(RGB565_BLUE);
  for (int y = 0; y < numCols; y++) {
    gfx->setCursor(2, 12 + y * 10);
    gfx->print(y, 16);
  }

  char c = 0;
  for (int y = 0; y < numRows; y++) {
    for (int x = 0; x < numCols; x++) {
      gfx->drawChar(10 + x * 8, 12 + y * 10, c++, RGB565_WHITE, RGB565_BLACK);
    }
  }

  delay(5000);  // 5 seconds
}

void loop() {
}