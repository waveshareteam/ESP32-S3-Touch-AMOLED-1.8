#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/i2c_master.h"
#include "board_variant.h"

#define PMU_INPUT_PIN (gpio_num_t) CONFIG_PMU_INTERRUPT_PIN /*!< axp power chip interrupt Pin*/

#define I2C_MASTER_NUM (i2c_port_t) CONFIG_I2C_MASTER_PORT_NUM
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_MASTER_SDA_IO (gpio_num_t) CONFIG_PMU_I2C_SDA
#define I2C_MASTER_SCL_IO (gpio_num_t) CONFIG_PMU_I2C_SCL

#define I2C_MASTER_TIMEOUT_MS 1000

/*
! WARN:
Please do not run the example without knowing the external load voltage of the PMU,
it may burn your external load, please check the voltage setting before running the example,
if there is any loss, please bear it by yourself
*/
// #ifndef XPOWERS_NO_ERROR
// #error "Running this example is known to not damage the device! Please go and uncomment this!"
// #endif

static const char *TAG = "mian";
static i2c_master_bus_handle_t pmu_i2c_bus = NULL;
static i2c_master_dev_handle_t pmu_i2c_dev = NULL;
static uint8_t pmu_i2c_dev_addr = 0;

extern esp_err_t pmu_init();
extern esp_err_t i2c_init(void);
extern void pmu_isr_handler();

static void pmu_hander_task(void *);
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR pmu_irq_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void irq_init()
{
    if ((int)PMU_INPUT_PIN < 0) {
        ESP_LOGI(TAG, "PMU interrupt pin is disabled");
        return;
    }

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << PMU_INPUT_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_set_intr_type(PMU_INPUT_PIN, GPIO_INTR_NEGEDGE);
    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(PMU_INPUT_PIN, pmu_irq_handler, (void *)PMU_INPUT_PIN);
}

static esp_err_t get_pmu_i2c_device(uint8_t devAddr, i2c_master_dev_handle_t *dev)
{
    if (pmu_i2c_dev != NULL && pmu_i2c_dev_addr == devAddr) {
        *dev = pmu_i2c_dev;
        return ESP_OK;
    }

    if (pmu_i2c_dev != NULL) {
        ESP_RETURN_ON_ERROR(i2c_master_bus_rm_device(pmu_i2c_dev), TAG, "remove PMU I2C device failed");
        pmu_i2c_dev = NULL;
        pmu_i2c_dev_addr = 0;
    }

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = devAddr;
    dev_cfg.scl_speed_hz = I2C_MASTER_FREQ_HZ;
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(pmu_i2c_bus, &dev_cfg, &pmu_i2c_dev), TAG, "add PMU I2C device failed");

    pmu_i2c_dev_addr = devAddr;
    *dev = pmu_i2c_dev;
    return ESP_OK;
}

/**
 * @brief Read a sequence of bytes from a pmu registers
 */
int pmu_register_read(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
    if (len == 0)
    {
        return ESP_OK;
    }
    if (data == NULL)
    {
        return ESP_FAIL;
    }

    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = get_pmu_i2c_device(devAddr, &dev);
    if (ret == ESP_OK) {
        ret = i2c_master_transmit_receive(dev, &regAddr, 1, data, len, I2C_MASTER_TIMEOUT_MS);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "PMU READ FAILED! > ");
    }
    return ret == ESP_OK ? 0 : -1;
}

/**
 * @brief Write a byte to a pmu register
 */
int pmu_register_write_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
    if (data == NULL)
    {
        return ESP_FAIL;
    }

    uint8_t write_buf[32] = {0};
    if (len + 1 > sizeof(write_buf)) {
        ESP_LOGE(TAG, "PMU write length too large");
        return ESP_FAIL;
    }
    write_buf[0] = regAddr;
    memcpy(&write_buf[1], data, len);

    i2c_master_dev_handle_t dev = NULL;
    esp_err_t ret = get_pmu_i2c_device(devAddr, &dev);
    if (ret == ESP_OK) {
        ret = i2c_master_transmit(dev, write_buf, len + 1, I2C_MASTER_TIMEOUT_MS);
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "PMU WRITE FAILED! < ");
    }
    return ret == ESP_OK ? 0 : -1;
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t i2c_conf = {};
    i2c_conf.i2c_port = I2C_MASTER_NUM;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_IO;
    i2c_conf.scl_io_num = I2C_MASTER_SCL_IO;
    i2c_conf.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_conf.glitch_ignore_cnt = 7;
    i2c_conf.flags.enable_internal_pullup = true;
    return i2c_new_master_bus(&i2c_conf, &pmu_i2c_bus);
}

extern "C" void app_main(void)
{
    board_variant_t variant = board_variant_detect();
    ESP_LOGI(TAG, "Running on %s", board_variant_to_name(variant));

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(5, sizeof(uint32_t));

    // Register PMU interrupt pins
    irq_init();

    ESP_ERROR_CHECK(i2c_init());

    ESP_LOGI(TAG, "I2C initialized successfully");

    ESP_ERROR_CHECK(pmu_init());

    xTaskCreate(pmu_hander_task, "App/pwr", 4 * 1024, NULL, 10, NULL);
}

static void pmu_hander_task(void *args)
{
    while (1)
    {
        pmu_isr_handler();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
