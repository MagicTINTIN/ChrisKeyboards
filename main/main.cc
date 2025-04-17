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
#define NUMBER_OF_SIMULT_KEYS 6
static const char *TAG = "DBG";

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

const uint8_t hid_report_descriptor[] = {
    // TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE)),
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD))};

/**
 * @brief String descriptor
 */
const char *hid_string_descriptor[5] = {
    (char[]){0x0c, 0x04}, // 0: is supported language is French, for English (0x0409)
    "MagicTINTIN",        // 1: Manufacturer
    "ChrisT1 Clavier",    // 2: Product
    "123456",             // 3: Serials, should use chip ID
    "CT1 Keyboard",       // 4: HID
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

const uint8_t matrix[8][17] = {
    {HID_KEY_G, HID_KEY_NONE, HID_KEY_F4, HID_KEY_ESCAPE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_ALT_LEFT, HID_KEY_ARROW_UP, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_0, HID_KEY_F5, HID_KEY_APOSTROPHE, HID_KEY_NONE, HID_KEY_F6, HID_KEY_H},
    {HID_KEY_T, HID_KEY_CAPS_LOCK, HID_KEY_F3, HID_KEY_TAB, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_SHIFT_LEFT, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_DECIMAL, HID_KEY_KEYPAD_DIVIDE, HID_KEY_KEYPAD_ADD, HID_KEY_BACKSPACE, HID_KEY_BRACKET_LEFT, HID_KEY_F7, HID_KEY_BRACKET_RIGHT, HID_KEY_Y},
    {HID_KEY_R, HID_KEY_W, HID_KEY_E, HID_KEY_Q, HID_KEY_PAGE_UP, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NUM_LOCK, HID_KEY_NONE, HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_3, HID_KEY_NONE, HID_KEY_P, HID_KEY_O, HID_KEY_I, HID_KEY_U},
    {HID_KEY_5, HID_KEY_F1, HID_KEY_F2, HID_KEY_GRAVE, HID_KEY_KEYPAD_8, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_CONTROL_LEFT, HID_KEY_NONE, HID_KEY_HOME, HID_KEY_INSERT, HID_KEY_DELETE, HID_KEY_F9, HID_KEY_MINUS, HID_KEY_F8, HID_KEY_EQUAL, HID_KEY_6},
    {HID_KEY_F, HID_KEY_S, HID_KEY_D, HID_KEY_A, HID_KEY_PAGE_DOWN, HID_KEY_EUROPE_1, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_ENTER, HID_KEY_KEYPAD_7, HID_KEY_KEYPAD_9, HID_KEY_NONE, HID_KEY_SEMICOLON, HID_KEY_L, HID_KEY_K, HID_KEY_J},
    {HID_KEY_4, HID_KEY_2, HID_KEY_3, HID_KEY_1, HID_KEY_GUI_LEFT, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_PRINT_SCREEN, HID_KEY_END, HID_KEY_F12, HID_KEY_F11, HID_KEY_F10, HID_KEY_0, HID_KEY_9, HID_KEY_8, HID_KEY_7},
    {HID_KEY_V, HID_KEY_X, HID_KEY_C, HID_KEY_Z, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_NONE, HID_KEY_SHIFT_RIGHT, HID_KEY_CONTROL_RIGHT, HID_KEY_NONE, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_ENTER, HID_KEY_BACKSLASH, HID_KEY_PERIOD, HID_KEY_COMMA, HID_KEY_M},
    {HID_KEY_B, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_2, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_ALT_RIGHT, HID_KEY_ARROW_LEFT, HID_KEY_ARROW_RIGHT, HID_KEY_ARROW_DOWN, HID_KEY_SPACE, HID_KEY_SLASH, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_N}};

bool fnPressed = false;
uint8_t _currentKeysContent[NUMBER_OF_SIMULT_KEYS] = {0};
uint8_t _newKeysContent[NUMBER_OF_SIMULT_KEYS] = {0};
uint8_t alreadyPressedKeys[UINT8_MAX + 1] = {0};

uint8_t currentMod = 0;

uint8_t *currentKeys = _currentKeysContent;
uint8_t *newKeys = _newKeysContent;
uint8_t newKeysIndex = 0;
bool alreadyPressedNewKeysFull = false;
bool noKeyPressedPreviously = true;
bool noKeyPressed = true;

void sendKeysReport()
{
    if (noKeyPressed && noKeyPressedPreviously)
        return;
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, currentMod, currentKeys);
}

void modPressRegistration(uint8_t k)
{
    if (k == HID_KEY_CONTROL_LEFT)
        currentMod |= KEYBOARD_MODIFIER_LEFTCTRL;
    else if (k == HID_KEY_SHIFT_LEFT)
        currentMod |= KEYBOARD_MODIFIER_LEFTSHIFT;
    else if (k == HID_KEY_ALT_LEFT)
        currentMod |= KEYBOARD_MODIFIER_LEFTALT;
    else if (k == HID_KEY_GUI_LEFT)
        currentMod |= KEYBOARD_MODIFIER_LEFTGUI;
    else if (k == HID_KEY_CONTROL_RIGHT)
        currentMod |= KEYBOARD_MODIFIER_RIGHTCTRL;
    else if (k == HID_KEY_SHIFT_RIGHT)
        currentMod |= KEYBOARD_MODIFIER_RIGHTSHIFT;
    else if (k == HID_KEY_ALT_RIGHT)
        currentMod |= KEYBOARD_MODIFIER_RIGHTALT;
    else if (k == HID_KEY_GUI_RIGHT)
        currentMod |= KEYBOARD_MODIFIER_RIGHTGUI;
}

/**
 * Already pressed keys are priorised
 */
void keyPressRegistration(uint8_t k)
{
    noKeyPressed = false;
    if (k >= HID_KEY_CONTROL_LEFT)
        modPressRegistration(k);

    if (alreadyPressedNewKeysFull)
        return;

    if (newKeysIndex < NUMBER_OF_SIMULT_KEYS)
    {
        newKeys[newKeysIndex++] = k;
        return;
    }
    bool alreadyPressed = alreadyPressedKeys[k];

    if (!alreadyPressed)
        return; // buffer already full, and not priorised, ignored

    for (uint8_t i = 0; i < NUMBER_OF_SIMULT_KEYS; i++)
    {
        if (!alreadyPressedKeys[newKeys[i]])
        {
            newKeys[i] = k;
            return;
        }
    }
    alreadyPressedNewKeysFull = true;
}

void keyUpdateRegistration()
{
    for (uint8_t i = 1; i < NUMBER_OF_SIMULT_KEYS; i++)
    {
        alreadyPressedKeys[currentKeys[i]] = 0;
    }
    uint8_t *tmp = currentKeys;
    currentKeys = newKeys;
    newKeys = tmp;
    memset(newKeys, 0, NUMBER_OF_SIMULT_KEYS);

    //
    sendKeysReport();

    newKeysIndex = 0;
    currentMod = 0;

    noKeyPressedPreviously = noKeyPressed;
    noKeyPressed = true;

    alreadyPressedNewKeysFull = false;
    for (uint8_t i = 1; i < NUMBER_OF_SIMULT_KEYS; i++)
    {
        alreadyPressedKeys[currentKeys[i]] = 1;
    }
}

static void app_send_hid_demo(void)
{
    // Keyboard output: Send key 'a/A' pressed and released
    // ESP_LOGI(TAG, "Sending Keyboard report");
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
}

extern "C" void app_main(void)
{
    // GPIOs for columns (KSIs, ESP outputs)
    const gpio_num_t cols[] = {
        GPIO_NUM_40, GPIO_NUM_41, GPIO_NUM_42, GPIO_NUM_35,
        GPIO_NUM_36, GPIO_NUM_45, GPIO_NUM_47, GPIO_NUM_48};
    const int num_cols = sizeof(cols) / sizeof(cols[0]);

    // GPIOs for rows (KSOs, ESP inputs)
    const gpio_num_t rows[] = {
        GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7, GPIO_NUM_8,
        GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_12, GPIO_NUM_13,
        GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18,
        GPIO_NUM_19, GPIO_NUM_20};
    const int num_rows = sizeof(rows) / sizeof(rows[0]);

    // --- Configuring rows ---
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

    // --- Configuring columns ---
    for (int i = 0; i < num_cols; ++i)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_config_t col_conf = {
            .pin_bit_mask = 1ULL << cols[i],
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&col_conf);

        // all columns are HIGH initially
        gpio_set_level(cols[i], 1);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

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

    // ESP_LOGI(TAG, "> back/skip buttons configured");
    // vTaskDelay(pdMS_TO_TICKS(20));

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_string_descriptor,
        .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
        .external_phy = false,
#if (TUD_OPT_HIGH_SPEED)
        .fs_configuration_descriptor = hid_configuration_descriptor, // HID configuration descriptor for full-speed and high-speed are the same
        .hs_configuration_descriptor = hid_configuration_descriptor,
        .qualifier_descriptor = NULL,
#else
        .configuration_descriptor = hid_configuration_descriptor,
#endif // TUD_OPT_HIGH_SPEED
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    while (true)
    {
        if (tud_mounted())
        {
            for (int col = 0; col < num_cols; ++col)
            {
                // current to column LOW, rest HIGH
                for (int i = 0; i < num_cols; ++i)
                {
                    gpio_set_level(cols[i], i == col ? 0 : 1);
                }

                // small delay for signal to settle
                esp_rom_delay_us(50);

                // read all rows
                for (int row = 0; row < num_rows; ++row)
                {
                    int val = gpio_get_level(rows[row]);
                    if (val == 0)
                    {
                        keyPressRegistration(matrix[col][row]);
                        // printf("Key pressed at [col=%d, row=%d]\n", col, row);
                    }
                }
            }

            // special keys
            if (!gpio_get_level(GPIO_NUM_2))
                printf("back\n");
            if (!gpio_get_level(GPIO_NUM_3))
                printf("skip\n");

            keyUpdateRegistration();
        }

        // delay before next scan
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}