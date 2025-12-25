#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include <Adafruit_XCA9554.h>

#include "pin_config.h"
#include "lv_conf.h"
#include <demos/lv_demos.h>
#include "HWCDC.h"
#include <SensorQMI8658.hpp>
#include <Wire.h>
#include <ui_c.h>
#include <WiFi.h>

SensorQMI8658 qmi;
IMUdata acc;
float angleX = 1;
float angleY = 0;

#define SSID "luckfox"
#define PWD "12345678"

bool rotation = false;

HWCDC USBSerial;
#define EXAMPLE_LVGL_TICK_PERIOD_MS 2


Adafruit_XCA9554 expander;


uint32_t screenWidth;
uint32_t screenHeight;

static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf[screenWidth * screenHeight / 10];

int i = 0, j = 0, b = 255, brightness_flag = 0;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */);

std::shaRGB565_RED_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shaRGB565_RED<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

const char *ntpServer = "120.25.108.11";
int net_flag = 0;


void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  FT3168->IIC_Interrupt_Flag = true;
}

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void example_increase_lvgl_tick(void *arg) {
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static uint8_t count = 0;
void example_increase_reboot(void *arg) {
  count++;
  if (count == 30) {
    esp_restart();
  }
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  int32_t touchX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t touchY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (FT3168->IIC_Interrupt_Flag == true) {
    FT3168->IIC_Interrupt_Flag = false;
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    USBSerial.print("Data x ");
    USBSerial.print(touchX);

    USBSerial.print("Data y ");
    USBSerial.println(touchY);

    if (touchY <= 90 && touchY >= 20) { brightness_flag = 1; }

  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void wifi_init() {
  WiFi.begin(SSID, PWD);

  int connect_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500);
    USBSerial.print(".");
    connect_count++;
  }

  USBSerial.println("Wifi connect");
  configTime((const long)(8 * 3600), 0, ntpServer);

  net_flag = 1;
}

long task_runtime_1 = 0;
void Task_my(void *pvParameters) {
  while (1) {

    if (net_flag == 1)
      if ((millis() - task_runtime_1) > 1000) {
        display_time();

        task_runtime_1 = millis();
      }

    vTaskDelay(100);
  }
}

void display_time() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    USBSerial.println("Failed to obtain time");
    return;
  } else {
    int year = timeinfo.tm_year + 1900;
    int month = timeinfo.tm_mon + 1;
    int day = timeinfo.tm_mday;
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    int sec = timeinfo.tm_sec;

    if (min < 10) {
      lv_label_set_text_fmt(ui_Label10, "%d:0%d", hour, min);
    } else {
      lv_label_set_text_fmt(ui_Label10, "%d:%d", hour, min);
    }
    lv_label_set_text_fmt(ui_Label30, "%d", sec);
  }
}

void setup() {
  USBSerial.begin(115200); /* prepare for possible serial debug */

  wifi_init();

  Wire.begin(IIC_SDA, IIC_SCL);
  if (!expander.begin(0x20)) {  // Replace with actual I2C address if different
    Serial.println("Failed to find XCA9554 chip");
    while (1)
      ;
  }

  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
  expander.pinMode(6, OUTPUT);
  expander.digitalWrite(0, LOW);
  expander.digitalWrite(1, LOW);
  expander.digitalWrite(2, LOW);
  expander.digitalWrite(6, LOW);
  delay(20);
  expander.digitalWrite(0, HIGH);
  expander.digitalWrite(1, HIGH);
  expander.digitalWrite(2, HIGH);
  expander.digitalWrite(6, HIGH);
  if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {
    Serial.println("Failed to find QMI8658 - check your wiring!");
    while (1) {
      delay(1000);
    }
  }

  // 设置加速度计
  qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
  qmi.enableAccelerometer();

  while (FT3168->begin() == false) {
    USBSerial.println("FT3168 initialization fail");
    delay(2000);
  }
  USBSerial.println("FT3168 initialization successfully");

  gfx->begin();
  gfx->setBrightness(b);

  screenWidth = gfx->width();
  screenHeight = gfx->height();

  lv_init();

  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);

  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  USBSerial.println(LVGL_Arduino);
  USBSerial.println("I am LVGL_Arduino");



#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                 FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);


  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 4);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 1;  // add for rotation
  disp_drv.rotated = LV_DISP_ROT_90;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };

  const esp_timer_create_args_t reboot_timer_args = {
    .callback = &example_increase_reboot,
    .name = "reboot"
  };

  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

  ui_init();

  USBSerial.println("Setup done");
  xTaskCreatePinnedToCore(Task_my, "Task_my", 20000, NULL, 1, NULL, 1);
}

void loop() {
  if (qmi.getDataReady()) {
    if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
      angleY = acc.y;
      if (angleY < -0.8 && !rotation) {
        lv_disp_set_rotation(NULL, LV_DISP_ROT_90);
        rotation = true;
      } else if (angleY > 0.8 && !rotation) {
        lv_disp_set_rotation(NULL, LV_DISP_ROT_270);
        rotation = true;
      }
      if (angleY <= 0.8 && angleY >= -0.8) {
        rotation = false;  // 允许重新执行旋转
      }
    }
  }

  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
  if (brightness_flag == 1 && FT3168->IIC_Interrupt_Flag == true) {
    delay(100);
    if (brightness_flag == 1 && FT3168->IIC_Interrupt_Flag == true) {
      brightness_flag = 0;
      b = b - 50;
      if (b == 5) { b = 255; }
      gfx->setBrightness(b);
      lv_label_set_text_fmt(ui_Label29, "%d", b);
    }
  }

  i = i + 1;
  lv_arc_set_value(ui_Arc1, i);
  if (i > 100) { i = 0; }
}
