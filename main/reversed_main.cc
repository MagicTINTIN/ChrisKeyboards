extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "driver/gpio.h"
}

#include <vector>

static const char *TAG = "KEYBOARD_MATRIX";

// Define your GPIOs
const gpio_num_t row_pins[] = {
    GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8,
    GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13,
    GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18,
    GPIO_NUM_19, GPIO_NUM_20
};

const gpio_num_t col_pins[] = {
    GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42, GPIO_NUM_35,
    GPIO_NUM_36, GPIO_NUM_45, GPIO_NUM_47, GPIO_NUM_48
};

void configure_gpio()
{
    // Configure rows as OUTPUTs
    for (gpio_num_t pin : row_pins)
    {
        gpio_config_t row_conf = {
            .pin_bit_mask = 1ULL << pin,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&row_conf);
        gpio_set_level(pin, 0); // Set LOW initially
    }

    // Configure columns as INPUTs
    for (gpio_num_t pin : col_pins)
    {
        gpio_config_t col_conf = {
            .pin_bit_mask = 1ULL << pin,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&col_conf);
    }
}

extern "C" void app_main(void)
{
    configure_gpio();
    ESP_LOGI(TAG, "Starting keyboard matrix scan...");

    while (true)
    {
        for (gpio_num_t row : row_pins)
        {
            // Set current row HIGH
            gpio_set_level(row, 1);
            esp_rom_delay_us(30); // small delay for signal to propagate

            std::vector<int> pressed_cols;

            for (gpio_num_t col : col_pins)
            {
                int level = gpio_get_level(col);
                if (level == 1)
                {
                    pressed_cols.push_back(col);
                }
            }

            if (!pressed_cols.empty())
            {
                printf("Row %02d: Pressed columns:", row);
                for (int col : pressed_cols)
                {
                    printf(" %d", col);
                }
                printf("\n");
            }

            gpio_set_level(row, 0); // Set row back to LOW
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Scan every 100 ms
    }
}
