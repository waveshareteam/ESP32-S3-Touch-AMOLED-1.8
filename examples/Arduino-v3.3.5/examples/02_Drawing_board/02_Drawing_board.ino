#include <Wire.h>
#include <Arduino.h>
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include <Adafruit_XCA9554.h>
#include "HWCDC.h"
HWCDC USBSerial;

Adafruit_XCA9554 expander;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

void setup() {
  USBSerial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);
  if (!expander.begin(0x20)) {  // Replace with actual I2C address if different
    Serial.println("Failed to find XCA9554 chip");
    while (1)
      ;
  }
  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
  expander.digitalWrite(0, LOW);
  expander.digitalWrite(1, LOW);
  expander.digitalWrite(2, LOW);
  delay(20);
  expander.digitalWrite(0, HIGH);
  expander.digitalWrite(1, HIGH);
  expander.digitalWrite(2, HIGH);

  while (FT3168->begin() == false) {
    USBSerial.println("FT3168 initialization fail");
    delay(2000);
  }
  USBSerial.println("FT3168 initialization successfully");

  FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                 FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

  gfx->begin();
  gfx->fillScreen(RGB565_WHITE);

  for (int i = 0; i <= 255; i++)  //0-255
  {
    gfx->setBrightness(i);
    gfx->setCursor(30, 150);
    gfx->setTextColor(RGB565_BLUE);
    gfx->setTextSize(4);
    gfx->println("Loading board");
    delay(3);
  }
  delay(500);
  gfx->fillScreen(RGB565_WHITE);
}

void loop() {
  int32_t touchX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t touchY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (FT3168->IIC_Interrupt_Flag == true) {
    FT3168->IIC_Interrupt_Flag = false;
    USBSerial.printf("Touch X:%d Y:%d\n", touchX, touchY);
    if (touchX > 20 && touchY > 20) { gfx->fillCircle(touchX, touchY, 5, RGB565_BLUE); }
  }
}
