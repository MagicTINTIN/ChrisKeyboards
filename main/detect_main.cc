/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "driver/gpio.h"
#include <vector>
#include <string>

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "example";

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/**
 * @brief HID report descriptor
 *
 * In this example we implement Keyboard + Mouse HID device,
 * so we must define both report descriptors
 */
const uint8_t hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE))};

/**
 * @brief String descriptor
 */
const char *hid_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x0c, 0x04},    // 0: is supported language is French, for English (0x0409)
    "MagicTINTIN",           // 1: Manufacturer
    "ChrisT1 Clavier",       // 2: Product
    "123456",                // 3: Serials, should use chip ID
    "Example HID interface", // 4: HID
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
}

/********* Application ***************/

typedef enum
{
    MOUSE_DIR_RIGHT,
    MOUSE_DIR_DOWN,
    MOUSE_DIR_LEFT,
    MOUSE_DIR_UP,
    MOUSE_DIR_MAX,
} mouse_dir_t;

#define DISTANCE_MAX 125
#define DELTA_SCALAR 5

// static void mouse_draw_square_next_delta(int8_t *delta_x_ret, int8_t *delta_y_ret)
// {
//     static mouse_dir_t cur_dir = MOUSE_DIR_RIGHT;
//     static uint32_t distance = 0;

//     // Calculate next delta
//     if (cur_dir == MOUSE_DIR_RIGHT)
//     {
//         *delta_x_ret = DELTA_SCALAR;
//         *delta_y_ret = 0;
//     }
//     else if (cur_dir == MOUSE_DIR_DOWN)
//     {
//         *delta_x_ret = 0;
//         *delta_y_ret = DELTA_SCALAR;
//     }
//     else if (cur_dir == MOUSE_DIR_LEFT)
//     {
//         *delta_x_ret = -DELTA_SCALAR;
//         *delta_y_ret = 0;
//     }
//     else if (cur_dir == MOUSE_DIR_UP)
//     {
//         *delta_x_ret = 0;
//         *delta_y_ret = -DELTA_SCALAR;
//     }

//     // Update cumulative distance for current direction
//     distance += DELTA_SCALAR;
//     // Check if we need to change direction
//     if (distance >= DISTANCE_MAX)
//     {
//         distance = 0;
//         cur_dir++;
//         if (cur_dir == MOUSE_DIR_MAX)
//         {
//             cur_dir = 0;
//         }
//     }
// }

static void app_send_hid_demo(void)
{
    // Keyboard output: Send key 'a/A' pressed and released
    ESP_LOGI(TAG, "Sending Keyboard report");
    uint8_t keycode[6] = {HID_KEY_A, 0, 0, 0, 0, 0};
    for (unsigned char i = 0; i < 4; i++)
    {
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
        vTaskDelay(pdMS_TO_TICKS(50));
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    for (unsigned char i = 4; i < 0; i++)
    {
        uint8_t keycode[6] = {i, 0, 0, 0, 0, 0}; // HID_KEY_A
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
        vTaskDelay(pdMS_TO_TICKS(50));
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Mouse output: Move mouse cursor in square trajectory
    // ESP_LOGI(TAG, "Sending Mouse report");
    // int8_t delta_x;
    // int8_t delta_y;
    // for (int i = 0; i < (DISTANCE_MAX / DELTA_SCALAR) * 4; i++)
    // {
    //     // Get the next x and y delta in the draw square pattern
    //     mouse_draw_square_next_delta(&delta_x, &delta_y);
    //     tud_hid_mouse_report(HID_ITF_PROTOCOL_MOUSE, 0x00, delta_x, delta_y, 0, 0);
    //     vTaskDelay(pdMS_TO_TICKS(20));
    // }
}

std::string paddString(std::string input, unsigned char size, bool notFinal)
{
    std::string s = "";
    for (char i = 0; i < size - input.size(); i++)
    {
        s += " ";
    }
    if (notFinal)
        return s + input.substr(8);
    return s + input;
}

extern "C" void app_main(void)
{
    // for (int i = 0; i < 1000; i++)
    // {
    //     ESP_LOGI(TAG, "> Go to");
    //     printf("STARTING.%d\n", i);
    // }

    // fflush(stdout);                // flush printf
    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_LOGI(TAG, "> starting...");
    vTaskDelay(pdMS_TO_TICKS(20));
    // ESP_LOGI(TAG, "> Go to");
    // ESP_LOGI(TAG, "> Start\n");
    // const char *TAG = "KEYBOARD_MATRIX";
    const char *TAG = "GPIO_MONITOR";

    // GPIOs for columns (outputs)
    const gpio_num_t cols[] = {
        GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42, GPIO_NUM_35, // GPIO_NUM_43,
        GPIO_NUM_36, GPIO_NUM_45, GPIO_NUM_47, GPIO_NUM_48  // GPIO_NUM_44
    };
    const int num_cols = sizeof(cols) / sizeof(cols[0]);

    ESP_LOGI(TAG, "> cols defined");
    vTaskDelay(pdMS_TO_TICKS(20));

    // GPIOs for rows (inputs)
    const gpio_num_t rows[] = {
        GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8,
        GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13,
        GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18,
        GPIO_NUM_19, GPIO_NUM_20};
    const int num_rows = sizeof(rows) / sizeof(rows[0]);

    ESP_LOGI(TAG, "> rows defined");
    vTaskDelay(pdMS_TO_TICKS(20));

    // --- Configure rows (inputs) ---
    for (int i = 0; i < num_rows; ++i)
    {
        gpio_config_t row_conf = {
            .pin_bit_mask = 1ULL << rows[i],
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&row_conf);
    }

    ESP_LOGI(TAG, "> rows gpio configured");
    vTaskDelay(pdMS_TO_TICKS(20));

    // --- Configure columns (outputs) ---
    for (int i = 0; i < num_cols; ++i)
    {
        // ESP_LOGI(TAG, "> col:%i gpio:%02d... configuring...", i, cols[i]);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_config_t col_conf = {
            .pin_bit_mask = 1ULL << cols[i],
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&col_conf);

        // Set all columns HIGH initially
        gpio_set_level(cols[i], 1);

        // ESP_LOGI(TAG, "> col:%i gpio:%02d configured", i, cols[i]);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    ESP_LOGI(TAG, "> cols gpio configured");
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_config_t back = {
        .pin_bit_mask = 1ULL << GPIO_NUM_3,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&back);

    gpio_config_t skip = {
        .pin_bit_mask = 1ULL << GPIO_NUM_2,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&skip);

    ESP_LOGI(TAG, "> back/skip buttons configured");
    vTaskDelay(pdMS_TO_TICKS(20));

    std::vector<std::string> keys = {"HID_KEY_UNDEF", "HID_KEY_A", "HID_KEY_B", "HID_KEY_C", "HID_KEY_D", "HID_KEY_E", "HID_KEY_F", "HID_KEY_G", "HID_KEY_H", "HID_KEY_I", "HID_KEY_J", "HID_KEY_K", "HID_KEY_L", "HID_KEY_M", "HID_KEY_N", "HID_KEY_O", "HID_KEY_P", "HID_KEY_Q", "HID_KEY_R", "HID_KEY_S", "HID_KEY_T", "HID_KEY_U", "HID_KEY_V", "HID_KEY_W", "HID_KEY_X", "HID_KEY_Y", "HID_KEY_Z", "HID_KEY_1", "HID_KEY_2", "HID_KEY_3", "HID_KEY_4", "HID_KEY_5", "HID_KEY_6", "HID_KEY_7", "HID_KEY_8", "HID_KEY_9", "HID_KEY_0", "HID_KEY_ENTER", "HID_KEY_ESCAPE", "HID_KEY_BACKSPACE", "HID_KEY_TAB", "HID_KEY_SPACE", "HID_KEY_MINUS", "HID_KEY_EQUAL", "HID_KEY_BRACKET_LEFT", "HID_KEY_BRACKET_RIGHT", "HID_KEY_BACKSLASH", "HID_KEY_EUROPE_1", "HID_KEY_SEMICOLON", "HID_KEY_APOSTROPHE", "HID_KEY_GRAVE", "HID_KEY_COMMA", "HID_KEY_PERIOD", "HID_KEY_SLASH", "HID_KEY_CAPS_LOCK", "HID_KEY_F1", "HID_KEY_F2", "HID_KEY_F3", "HID_KEY_F4", "HID_KEY_F5", "HID_KEY_F6", "HID_KEY_F7", "HID_KEY_F8", "HID_KEY_F9", "HID_KEY_F10", "HID_KEY_F11", "HID_KEY_F12", "HID_KEY_PRINT_SCREEN", "HID_KEY_SCROLL_LOCK", "HID_KEY_PAUSE", "HID_KEY_INSERT", "HID_KEY_HOME", "HID_KEY_PAGE_UP", "HID_KEY_DELETE", "HID_KEY_END", "HID_KEY_PAGE_DOWN", "HID_KEY_ARROW_RIGHT", "HID_KEY_ARROW_LEFT", "HID_KEY_ARROW_DOWN", "HID_KEY_ARROW_UP", "HID_KEY_NUM_LOCK", "HID_KEY_KEYPAD_DIVIDE", "HID_KEY_KEYPAD_MULTIPLY", "HID_KEY_KEYPAD_SUBTRACT", "HID_KEY_KEYPAD_ADD", "HID_KEY_KEYPAD_ENTER", "HID_KEY_KEYPAD_1", "HID_KEY_KEYPAD_2", "HID_KEY_KEYPAD_3", "HID_KEY_KEYPAD_4", "HID_KEY_KEYPAD_5", "HID_KEY_KEYPAD_6", "HID_KEY_KEYPAD_7", "HID_KEY_KEYPAD_8", "HID_KEY_KEYPAD_9", "HID_KEY_KEYPAD_0", "HID_KEY_KEYPAD_DECIMAL", "HID_KEY_CONTROL_LEFT", "HID_KEY_SHIFT_LEFT", "HID_KEY_ALT_LEFT", "HID_KEY_GUI_LEFT", "HID_KEY_CONTROL_RIGHT", "HID_KEY_SHIFT_RIGHT", "HID_KEY_ALT_RIGHT"};
    std::vector<std::string> skippedKeys = {};
    int matrix[num_cols][num_rows] = {0};

    printf("Successfully set up.\n");

    ESP_LOGI(TAG, "> matrix scanning started");
    vTaskDelay(pdMS_TO_TICKS(20));
    // ESP_LOGI(TAG, "> Go to");
    // ESP_LOGI(TAG, "> MAT\n");
    // --- Matrix scan loop ---
    for (unsigned int keyIndex = 1; keyIndex < keys.size();)
    {
        printf("Press the key '%s'\n-", keys.at(keyIndex).c_str());
        fflush(stdout);
        bool notFound = true;
        while (notFound)
        {
            for (int col = 0; col < num_cols; ++col)
            {
                for (int i = 0; i < num_cols; ++i)
                {
                    gpio_set_level(cols[i], i == col ? 0 : 1);
                }
                esp_rom_delay_us(50);

                for (int row = 0; row < num_rows; ++row)
                {
                    int val = gpio_get_level(rows[row]);
                    if (val == 0)
                    {
                        // printf("Key pressed at [col=%d, row=%d]\n", col, row);
                        if (matrix[col][row] == 0)
                        {
                            matrix[col][row] = keyIndex;
                            printf("\r'%s' has been assigned to [%d,%d]\n\n", keys.at(keyIndex).c_str(), col, row);
                            keyIndex++;
                            notFound = false;
                        }
                        else
                        {
                            printf("\r%d,%d already assigned.", col, row);
                            fflush(stdout);
                        }
                    }
                }
            }
            if (!gpio_get_level(GPIO_NUM_2))
            {
                printf("back\n");
                keyIndex--;
                if (keyIndex < 1)
                    keyIndex = 1;
                for (unsigned c = 0; c < num_cols; c++)
                    for (unsigned r = 1; r < num_rows; r++)
                        if (matrix[c][r] == keyIndex)
                            matrix[c][r] = 0;
                notFound = false;
            }
            if (!gpio_get_level(GPIO_NUM_3))
            {
                printf("skip\n");
                skippedKeys.push_back(keys.at(keyIndex));
                keyIndex++;
                notFound = false;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        printf("Current matrix:\n{\n");
        for (unsigned c = 0; c < num_cols; c++)
        {
            printf("    {%s", paddString(keys.at(matrix[c][0]), 24, true).c_str());
            for (unsigned r = 1; r < num_rows; r++)
            {
                printf(",%s", paddString(keys.at(matrix[c][r]), 24, true).c_str());
            }
            if (c == num_cols - 1)
                printf("}\n");
            else
                printf("},\n");
        }
        printf("}\nSkipped keys:");

        for (std::string s : skippedKeys)
        {
            printf(" %s", s.c_str());
        }
        printf("\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("All keys have been registered into the matrix.\n");
    printf("Current matrix:\n{\n");
    for (unsigned c = 0; c < num_cols; c++)
    {
        printf("    {%s", paddString(keys.at(matrix[c][0]), 24, false).c_str());
        for (unsigned r = 1; r < num_rows; r++)
        {
            printf(",%s", paddString(keys.at(matrix[c][r]), 24, false).c_str());
        }
        if (c == num_cols - 1)
            printf("}\n");
        else
            printf("},\n");
    }
    printf("}\nSkipped keys:");

    for (std::string s : skippedKeys)
    {
        printf(" %s", s.c_str());
    }
    printf("\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    while (true)
    {
        for (int col = 0; col < num_cols; ++col)
        {
            // Set current column LOW, rest HIGH
            for (int i = 0; i < num_cols; ++i)
            {
                gpio_set_level(cols[i], i == col ? 0 : 1);
            }

            // Small delay for signal to settle
            esp_rom_delay_us(50);

            // Read all rows
            for (int row = 0; row < num_rows; ++row)
            {
                int val = gpio_get_level(rows[row]);
                if (val == 0)
                {
                    // ESP_LOGI(TAG, "Key pressed at [col=%d, row=%d]", col, row);
                    printf("Key pressed at [col=%d, row=%d]\n", col, row);
                }
            }
        }

        // Delay before next scan
        vTaskDelay(pdMS_TO_TICKS(10));
        if (!gpio_get_level(GPIO_NUM_2))
            printf("back\n");
        if (!gpio_get_level(GPIO_NUM_3))
            printf("skip\n");
    }
}