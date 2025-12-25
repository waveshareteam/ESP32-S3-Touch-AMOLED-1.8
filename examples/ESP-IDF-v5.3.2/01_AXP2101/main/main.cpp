#include <stdio.h>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/i2c.h"

#define PMU_INPUT_PIN (gpio_num_t) CONFIG_PMU_INTERRUPT_PIN /*!< axp power chip interrupt Pin*/
#define PMU_INPUT_PIN_SEL (1ULL << PMU_INPUT_PIN)

#define I2C_MASTER_NUM (i2c_port_t) CONFIG_I2C_MASTER_PORT_NUM
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY /*!< I2C master clock frequency */
#define I2C_MASTER_SDA_IO (gpio_num_t) CONFIG_PMU_I2C_SDA
#define I2C_MASTER_SCL_IO (gpio_num_t) CONFIG_PMU_I2C_SCL

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define WRITE_BIT I2C_MASTER_WRITE   /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ     /*!< I2C master read */
#define ACK_CHECK_EN 0x1             /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0            /*!< I2C master will not check ack from slave */
#define ACK_VAL (i2c_ack_type_t)0x0  /*!< I2C ack value */
#define NACK_VAL (i2c_ack_type_t)0x1 /*!< I2C nack value */

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
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = PMU_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_set_intr_type(PMU_INPUT_PIN, GPIO_INTR_NEGEDGE);
    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(PMU_INPUT_PIN, pmu_irq_handler, (void *)PMU_INPUT_PIN);
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
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, regAddr, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdTICKS_TO_MS(1000));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "PMU i2c_master_cmd_begin FAILED! > ");
        return ESP_FAIL;
    }
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | READ_BIT, ACK_CHECK_EN);
    if (len > 1)
    {
        i2c_master_read(cmd, data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, &data[len - 1], NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdTICKS_TO_MS(1000));
    i2c_cmd_link_delete(cmd);
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
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, regAddr, ACK_CHECK_EN);
    i2c_master_write(cmd, data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdTICKS_TO_MS(1000));
    i2c_cmd_link_delete(cmd);
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
    i2c_config_t i2c_conf;
    memset(&i2c_conf, 0, sizeof(i2c_conf));
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_IO;
    i2c_conf.scl_io_num = I2C_MASTER_SCL_IO;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    return i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

extern "C" void app_main(void)
{
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
