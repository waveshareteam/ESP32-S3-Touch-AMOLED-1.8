#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include <Wire.h>
#include "HWCDC.h"

HWCDC USBSerial;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */);

#if defined(ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#define log_i(format, ...) USBSerial.printf(format, ##__VA_ARGS__)
#endif

int16_t w, h, banner_text_size, banner_height, graph_baseline, graph_height, channel_width, signal_width;

// RSSI RANGE
#define RSSI_CEILING -30
#define RSSI_SHOW_SSID -70
#define RSSI_FLOOR -100

// Channel legend mapping
uint8_t channel_legend[] = {
    1, 2, 3, 4, 5, 6, 7,      // 0-6:     1,  2,  3,  4,  5,  6,  7,
    8, 9, 10, 11, 12, 13, 0}; // 7-13:    8,  9, 10, 11, 12, 13, 14

// Channel color mapping
uint16_t channel_color[] = {
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_LIME, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA,
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_LIME, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA};

uint8_t scan_count = 0;

void setup()
{
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  USBSerial.begin(115200);
  // USBSerial.setDebugOutput(true);
  // while(!USBSerial);
  USBSerial.println("Arduino_GFX ESP WiFi Analyzer example");

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

#if defined(LCD_PWR_PIN)
  pinMode(LCD_PWR_PIN, OUTPUT);    // sets the pin as output
  digitalWrite(LCD_PWR_PIN, HIGH); // power on
#endif

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  // Init Display
  if (!gfx->begin())
  {
    USBSerial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  w = gfx->width();
  h = gfx->height();
  banner_text_size = (w < 300) ? 1 : 2;
  banner_height = (banner_text_size * 8) + 2;
  graph_height = h - banner_height - 30; // minus 3 text lines
  graph_baseline = banner_height + 10 + graph_height;
  channel_width = w / 16;
  signal_width = channel_width * 2;

  // init banner
  gfx->fillRect(0, 0, w, banner_text_size * 8, RGB565_PURPLE);
  gfx->setTextSize(banner_text_size);
  gfx->setCursor(0, 0);
  gfx->setTextColor(RGB565_WHITE, RGB565_RED);
  gfx->print(" ESP");
  gfx->setTextColor(RGB565_WHITE, RGB565_DARKORANGE);
#if defined(ESP32)
  gfx->print("32 ");
#elif defined(ESP8266)
  gfx->print("8266 ");
#endif
  gfx->setTextColor(RGB565_WHITE, RGB565_MEDIUMBLUE);
  gfx->print(" WiFi ");
  gfx->setTextColor(RGB565_WHITE, RGB565_PURPLE);
  gfx->print(" Analyzer");
  gfx->setTextSize(1);

#ifdef CANVAS
  gfx->flush();
#endif
}

bool matchBssidPrefix(uint8_t *a, uint8_t *b)
{
  for (uint8_t i = 0; i < 5; i++)
  { // only compare first 5 bytes
    if (a[i] != b[i])
    {
      return false;
    }
  }
  return true;
}

void loop()
{
  uint8_t ap_count_list[sizeof(channel_legend)];
  int32_t noise_list[sizeof(channel_legend)];
  int32_t peak_list[sizeof(channel_legend)];
  int16_t peak_id_list[sizeof(channel_legend)];
  int32_t channel;
  int16_t idx;
  int32_t rssi;
  uint8_t *bssid;
  String ssid;
  uint16_t color;
  int16_t height, offset, text_width;

  // init array value
  for (int i = 0; i < sizeof(channel_legend); ++i)
  {
    ap_count_list[i] = 0;
    noise_list[i] = RSSI_FLOOR;
    peak_list[i] = RSSI_FLOOR;
    peak_id_list[i] = -1;
  }

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */);

  // clear old graph
  gfx->fillRect(0, banner_height, w, h - banner_height, RGB565_BLACK);

  if (n == 0)
  {
    gfx->setTextColor(RGB565_WHITE);
    gfx->setCursor(0, banner_height);
    gfx->println("No networks found");
  }
  else
  {
    for (int i = 0; i < n; i++)
    {
      channel = WiFi.channel(i);
      idx = channel - 1;
      rssi = WiFi.RSSI(i);
      bssid = WiFi.BSSID(i);
      ssid = WiFi.SSID(i);

      // channel peak stat, find peak ssid
      if (ssid.length() > 0)
      {
        if (peak_list[idx] < rssi)
        {
          peak_list[idx] = rssi;
          peak_id_list[idx] = i;
        }
      }

      // check signal come from same AP
      bool duplicate_SSID = false;
      for (int j = 0; j < i; j++)
      {
        if ((WiFi.channel(j) == channel) && matchBssidPrefix(WiFi.BSSID(j), bssid))
        {
          duplicate_SSID = true;
          break;
        }
      }

      if (!duplicate_SSID)
      {
        ap_count_list[idx]++;

        // noise stat
        int32_t noise = rssi - RSSI_FLOOR;
        noise *= noise;
        if (channel > 4)
        {
          noise_list[idx - 4] += noise;
        }
        if (channel > 3)
        {
          noise_list[idx - 3] += noise;
        }
        if (channel > 2)
        {
          noise_list[idx - 2] += noise;
        }
        if (channel > 1)
        {
          noise_list[idx - 1] += noise;
        }
        noise_list[idx] += noise;
        if (channel < 14)
        {
          noise_list[idx + 1] += noise;
        }
        if (channel < 13)
        {
          noise_list[idx + 2] += noise;
        }
        if (channel < 12)
        {
          noise_list[idx + 3] += noise;
        }
        if (channel < 11)
        {
          noise_list[idx + 4] += noise;
        }
      }
    }

    // plot found WiFi info
    for (int i = 0; i < n; i++)
    {
      channel = WiFi.channel(i);
      idx = channel - 1;
      rssi = WiFi.RSSI(i);
      color = channel_color[idx];
      height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, graph_height), 1, graph_height);
      offset = (idx + 2) * channel_width;

      // trim rssi with RSSI_FLOOR
      if (rssi < RSSI_FLOOR)
      {
        rssi = RSSI_FLOOR;
      }

      // plot chart
      // gfx->drawLine(offset, graph_baseline - height, offset - signal_width, graph_baseline + 1, color);
      // gfx->drawLine(offset, graph_baseline - height, offset + signal_width, graph_baseline + 1, color);
      gfx->startWrite();
      gfx->writeEllipseHelper(offset, graph_baseline + 1, signal_width, height, 0b0011, color);
      gfx->endWrite();

      if ((rssi >= RSSI_SHOW_SSID) && (i == peak_id_list[idx]))
      {
        // Print SSID, signal strengh and if not encrypted
        ssid = WiFi.SSID(i);
        if (ssid.length() == 0)
        {
          ssid = WiFi.BSSIDstr(i);
        }
        text_width = (ssid.length() + 6) * 6;
        if (text_width > w)
        {
          offset = 0;
        }
        else
        {
          offset -= signal_width;
          if ((offset + text_width) > w)
          {
            offset = w - text_width;
          }
        }
        gfx->setTextColor(color);
        gfx->setCursor(offset, ((height + 8) > graph_height) ? (graph_baseline - graph_height) : (graph_baseline - 10 - height));
        gfx->print(ssid);
        gfx->print('(');
        gfx->print(rssi);
        gfx->print(')');
#if defined(ESP32)
        if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
#else
        if (WiFi.encryptionType(i) == ENC_TYPE_NONE)
#endif
        {
          gfx->print('*');
        }
      }
    }
  }

  // print WiFi stat
  gfx->setTextColor(RGB565_WHITE);
  gfx->setCursor(0, banner_height);
  gfx->print(n);
  gfx->print(" networks found, lesser noise channels: ");
  bool listed_first_channel = false;
  int32_t min_noise = noise_list[0];          // init with channel 1 value
  for (channel = 2; channel <= 11; channel++) // channels 12-14 may not available
  {
    idx = channel - 1;
    // log_i("min_noise: %d, noise_list[%d]: %d", min_noise, idx, noise_list[idx]);
    if (noise_list[idx] < min_noise)
    {
      min_noise = noise_list[idx];
    }
  }

  for (channel = 1; channel <= 11; channel++) // channels 12-14 may not available
  {
    idx = channel - 1;
    // check channel with min noise
    if (noise_list[idx] == min_noise)
    {
      if (!listed_first_channel)
      {
        listed_first_channel = true;
      }
      else
      {
        gfx->print(", ");
      }
      gfx->print(channel);
    }
  }

  // draw 2.4 GHz graph base axle
  gfx->drawFastHLine(0, graph_baseline, w, RGB565_WHITE);
  for (idx = 0; idx < 14; idx++)
  {
    channel = channel_legend[idx];
    offset = (idx + 2) * channel_width;
    if (channel > 0)
    {
      gfx->setTextColor(channel_color[idx]);
      gfx->setCursor(offset - ((channel < 10) ? 3 : 6), graph_baseline + 2);
      gfx->print(channel);
    }
    if (ap_count_list[idx] > 0)
    {
      gfx->setTextColor(RGB565_LIGHTGREY);
      gfx->setCursor(offset - ((ap_count_list[idx] < 10) ? 3 : 6), graph_baseline + 8 + 2);
      gfx->print(ap_count_list[idx]);
    }
  }

#ifdef CANVAS
  gfx->flush();
#endif

  // Wait a bit before scanning again
  delay(3000);

#if defined(SCAN_COUNT_SLEEP)
  // POWER SAVING
  if (++scan_count >= SCAN_COUNT_SLEEP)
  {
#if defined(LCD_PWR_PIN)
    pinMode(LCD_PWR_PIN, INPUT); // disable pin
#endif

#if defined(GFX_BL)
    pinMode(GFX_BL, INPUT); // disable pin
#endif

#if defined(ESP32)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);
    esp_deep_sleep_start();
#else
    ESP.deepSleep(0);
#endif
  }
#endif // defined(SCAN_COUNT_SLEEP)
}
