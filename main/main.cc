// #define CFG_TUD_HID 2
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <vector>
#include <string>
// #include "esp_task_wdt.h"
// #include <idf_additions.h>

#define BUZZER_GPIO 2
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_TIMER LEDC_TIMER_0

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
#define NUMBER_OF_SIMULT_KEYS 6
static const char *TAG = "DBG";

#define print_bits(x)                                    \
    do                                                   \
    {                                                    \
        unsigned long long a__ = (x);                    \
        size_t bits__ = sizeof(x) * 8;                   \
        printf(#x ": ");                                 \
        while (bits__--)                                 \
            putchar(a__ & (1ULL << bits__) ? '1' : '0'); \
        putchar('\n');                                   \
    } while (0)

/************* TinyUSB descriptors ****************/

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

static uint8_t const hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(),
    //   TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))
};

static uint8_t const hid_consumer_report_descriptor[] = {
    //   TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
    //   TUD_HID_REPORT_DESC_KEYBOARD(),
    //   TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))
    // TUD_HID_REPORT_DESC_CONSUMER()
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))

};

#define CONSUMER_REPORT_ID 2

/**
 * @brief String descriptor
 */
const char *hid_string_descriptor[5] = {
    (char[]){4, TUSB_DESC_STRING, 0x0c, 0x04},
    // (char[]){0x0c, 0x04}, // 0: is supported language is French, for English (0x0409)
    "MagicTINTIN",     // 1: Manufacturer
    "ChrisT1 Clavier", // 2: Product
    "123456",          // 3: Serials, should use chip ID
    "CT1 Keyboard",    // 4: HID
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
    // FIXME: booot protocol ?
    TUD_HID_DESCRIPTOR(0, 4, true, sizeof(hid_report_descriptor), 0x81, 16, 10),
    // TUD_HID_DESCRIPTOR(0, 4, true, sizeof(hid_report_descriptor), 0x81, CFG_TUD_HID_EP_BUFSIZE, 10),
    // TUD_HID_DESCRIPTOR(0, 0, false, sizeof(hid_consumer_report_descriptor), 0x81, CFG_TUD_HID_EP_BUFSIZE, 5),
    // TUD_HID_DESCRIPTOR(1, 0, false, sizeof(hid_consumer_report_descriptor), 0x82, CFG_TUD_HID_EP_BUFSIZE, 5),
    // TUD_HID_DESCRIPTOR(1, 0, false, sizeof(hid_consumer_report_descriptor), 0x82, 16, 10),
    TUD_HID_DESCRIPTOR(1, 4, false, sizeof(hid_consumer_report_descriptor), 0x82, 16, 10),
    // TUD_HID_DESCRIPTOR(1, 0, false, sizeof(hid_consumer_report_descriptor), 0x82, CFG_TUD_HID_EP_BUFSIZE, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    // switch (instance)
    // {
    // case 1:
    // case 2:
    //     return hid_consumer_report_descriptor;

    // default:
    //     return hid_report_descriptor;
    // }

    if (instance == 1)
    {
        return hid_consumer_report_descriptor;
    }
    else
    {
        return hid_report_descriptor;
    }
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

void buzzer_init()
{
    // ledc_timer_config_t ledc_timer = {
    //     .speed_mode       = LEDC_LOW_SPEED_MODE,
    //     .timer_num        = BUZZER_TIMER,
    //     .duty_resolution  = LEDC_TIMER_10_BIT,
    //     .freq_hz          = 1000,  // will override this later
    //     .clk_cfg          = LEDC_AUTO_CLK
    // };
    ledc_timer_config_t ledc_timer = {
        LEDC_LOW_SPEED_MODE, // speed_mode
        LEDC_TIMER_10_BIT,   // timer_num
        BUZZER_TIMER,        // duty_resolution
        5000,                // freq_hz
        LEDC_AUTO_CLK,       // clk_cfg
        false                // deconfigure (ajout si nécessaire)
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel);
}

void buzzer_quack()
{
    // Frequency and duty can be tuned to your speaker
    int freq = 880; // A5 note, sounds kind of like a short “quack”
    int duty = 512; // 50% of 10-bit resolution (0–1023)

    ledc_set_freq(LEDC_LOW_SPEED_MODE, BUZZER_TIMER, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);

    vTaskDelay(pdMS_TO_TICKS(150)); // play for 150 ms

    // stop the sound
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
}

#define STACK_SIZE 2048

// Static memory for the task
StaticTask_t buzzerTaskTCB;
StackType_t buzzerTaskStack[STACK_SIZE];

static TaskHandle_t buzzer_task_handle = nullptr;
static volatile bool buzzer_running = false;

void buzzer_task(void *param)
{
    ledc_timer_config_t ledc_timer = {
        LEDC_LOW_SPEED_MODE, // speed_mode
        LEDC_TIMER_10_BIT,   // timer_num
        BUZZER_TIMER,        // duty_resolution
        200,                 // freq_hz
        LEDC_AUTO_CLK,       // clk_cfg
        false                // deconfigure (ajout si nécessaire)
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0, // 50%
        .hpoint = 0};
    ledc_channel_config(&ledc_channel);

    while (1)
    {
        if (buzzer_running)
        {
            // printf("+");
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 512);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        }
        else
        {
            // printf("-");
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // avoid busy wait
        // fflush(stdout);
    }
}

void start_buzzer_task()
{
    TaskHandle_t buzzer_task_handle = xTaskCreateStatic(
        buzzer_task,     // Task function
        "BuzzerTask",    // Name
        STACK_SIZE,      // Stack size in words, not bytes
        NULL,            // Parameter
        5,               // Priority
        buzzerTaskStack, // Stack array
        &buzzerTaskTCB   // Task control block
    );

    // You can store `buzzer_task_handle` if you want to stop it later
}
void buzzer_on()
{
    buzzer_running = true;
}

void buzzer_off()
{
    buzzer_running = false;
}
/********* Application ***************/

#define M_HID_UNDEF 0x0
#define M_HIDMKY_FN_LOCK 0x1
#define M_HIDMK_BACKLIGHT 0x2
#define M_HIDMK_MORSE 0x20
#define M_HIDMK_HEXA 0x21
#define M_HIDMK_BIN 0x22
#define M_HIDUC_SCAN_PREVIOUS 0x40
#define M_HIDUC_PLAY_PAUSE 0x41
#define M_HIDUC_SCAN_NEXT 0x43
#define M_HIDUC_BRIGHTNESS_DECREMENT 0x44
#define M_HIDUC_BRIGHTNESS_INCREMENT 0x45
#define M_HIDUC_AL_CALCULATOR 0x46
#define M_HIDKEY_MUTE 0x60
#define M_HIDKEY_VOLUME_DOWN 0x61
#define M_HIDKEY_VOLUME_UP 0x62
#define M_HIDKEY_FIND 0x63
#define M_HIDKEY_APPLICATION 0x64
#define M_HIDKEY_SCROLLLOCK 0x65

#define KB_COLS 8
#define KB_ROWS 17

#define MAX_RAW_KEYS (KB_COLS * KB_ROWS)

const uint8_t fnMatrix[KB_COLS][KB_ROWS] = {
    {0, 0, M_HIDUC_SCAN_PREVIOUS, M_HIDMKY_FN_LOCK, 0, 0, 0, 0, 0, 0, 0, 0, M_HIDUC_PLAY_PAUSE, 0, 0, M_HIDUC_SCAN_NEXT, 0},
    {0, 0, M_HIDKEY_VOLUME_UP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, M_HIDKEY_MUTE, M_HIDKEY_VOLUME_DOWN, 0, 0, 0, 0, 0, 0, 0, M_HIDKEY_SCROLLLOCK, 0, M_HIDKEY_FIND, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, M_HIDMK_MORSE, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, M_HIDUC_BRIGHTNESS_INCREMENT, M_HIDUC_BRIGHTNESS_DECREMENT, M_HIDMK_BACKLIGHT, 0, 0, 0, 0},
    {0, M_HIDMK_HEXA, M_HIDUC_AL_CALCULATOR, 0, 0, 0, 0, M_HIDKEY_APPLICATION, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {M_HIDMK_BIN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

const uint8_t matrix[KB_COLS][KB_ROWS] = {
    {HID_KEY_G, HID_KEY_EUROPE_2, HID_KEY_F4, HID_KEY_ESCAPE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_ALT_LEFT, HID_KEY_ARROW_UP, HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_0, HID_KEY_F5, HID_KEY_APOSTROPHE, HID_KEY_NONE, HID_KEY_F6, HID_KEY_H},
    {HID_KEY_T, HID_KEY_CAPS_LOCK, HID_KEY_F3, HID_KEY_TAB, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_SHIFT_LEFT, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_DECIMAL, HID_KEY_KEYPAD_DIVIDE, HID_KEY_KEYPAD_ADD, HID_KEY_BACKSPACE, HID_KEY_BRACKET_LEFT, HID_KEY_F7, HID_KEY_BRACKET_RIGHT, HID_KEY_Y},
    {HID_KEY_R, HID_KEY_W, HID_KEY_E, HID_KEY_Q, HID_KEY_PAGE_UP, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NUM_LOCK, HID_KEY_NONE, HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_3, HID_KEY_NONE, HID_KEY_P, HID_KEY_O, HID_KEY_I, HID_KEY_U},
    {HID_KEY_5, HID_KEY_F1, HID_KEY_F2, HID_KEY_GRAVE, HID_KEY_KEYPAD_8, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_CONTROL_LEFT, HID_KEY_NONE, HID_KEY_HOME, HID_KEY_INSERT, HID_KEY_DELETE, HID_KEY_F9, HID_KEY_MINUS, HID_KEY_F8, HID_KEY_EQUAL, HID_KEY_6},
    {HID_KEY_F, HID_KEY_S, HID_KEY_D, HID_KEY_A, HID_KEY_PAGE_DOWN, HID_KEY_EUROPE_1, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_ENTER, HID_KEY_KEYPAD_7, HID_KEY_KEYPAD_9, HID_KEY_NONE, HID_KEY_SEMICOLON, HID_KEY_L, HID_KEY_K, HID_KEY_J},
    {HID_KEY_4, HID_KEY_2, HID_KEY_3, HID_KEY_1, HID_KEY_GUI_LEFT, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_PRINT_SCREEN, HID_KEY_END, HID_KEY_F12, HID_KEY_F11, HID_KEY_F10, HID_KEY_0, HID_KEY_9, HID_KEY_8, HID_KEY_7},
    {HID_KEY_V, HID_KEY_X, HID_KEY_C, HID_KEY_Z, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_NONE, HID_KEY_SHIFT_RIGHT, HID_KEY_CONTROL_RIGHT, HID_KEY_NONE, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_ENTER, HID_KEY_BACKSLASH, HID_KEY_PERIOD, HID_KEY_COMMA, HID_KEY_M},
    {HID_KEY_B, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_KEYPAD_2, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_ALT_RIGHT, HID_KEY_ARROW_LEFT, HID_KEY_ARROW_RIGHT, HID_KEY_ARROW_DOWN, HID_KEY_SPACE, HID_KEY_SLASH, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_N}};

bool fnPressed = false;
bool fnNewPressed = false;
bool fnLocked = false;
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

uint8_t consumerBuffer[2] = {0};
bool noConsumerPressed = true;
bool noConsumerPressedPreviously = true;

void printKeys()
{
    printf("Sending\nCurr: [%x|%x|%x|%x|%x|%x]\n", currentKeys[0], currentKeys[1], currentKeys[2], currentKeys[3], currentKeys[4], currentKeys[5]);
    printf("New: [%x|%x|%x|%x|%x|%x]\n", currentKeys[0], currentKeys[1], currentKeys[2], currentKeys[3], currentKeys[4], currentKeys[5]);
    print_bits(currentMod);
    printf("\n");
}

void sendKeysReport()
{
    if (!noKeyPressed || !noKeyPressedPreviously)
        tud_hid_keyboard_report(0, currentMod, currentKeys);
    // tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, currentMod, currentKeys);

    if (!noConsumerPressed)
        tud_hid_n_report(1, CONSUMER_REPORT_ID, consumerBuffer, sizeof(consumerBuffer));
    else if (!noConsumerPressedPreviously)
        tud_hid_n_report(1, CONSUMER_REPORT_ID, NULL, 0);
    // printKeys();
}

void modPressRegistration(uint8_t k)
{
    noKeyPressed = false;
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

uint32_t freqs[] = {130, 138, 146, 155, 164, 174, 185, 196, 207, 220, 233, 246, 261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987, 1046, 1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975, 2093, 2217, 2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951, 4186, 4434, 4698, 4978, 5274, 5587, 5919, 6271, 6644, 7040, 7458, 7902};

void normalKeyPressRegistration(uint8_t k)
{
    noKeyPressed = false;
    bool alreadyPressed = alreadyPressedKeys[k];
    // printf("k=%d;%d -> [(%d;%d),(%d;%d),(%d;%d),(%d;%d),(%d;%d)...]\n", k, alreadyPressedKeys[k], currentKeys[0], alreadyPressedKeys[currentKeys[0]], currentKeys[1], alreadyPressedKeys[currentKeys[1]], currentKeys[2], alreadyPressedKeys[currentKeys[2]], currentKeys[3], alreadyPressedKeys[currentKeys[3]], currentKeys[4], alreadyPressedKeys[currentKeys[4]]);
    if (!alreadyPressed)
    {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, BUZZER_TIMER, freqs[k % 72]);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL, 512);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, BUZZER_CHANNEL);
        buzzer_on();
    }
    // buzzer_quack();

    // Already pressed keys are priorised
    if (newKeysIndex < NUMBER_OF_SIMULT_KEYS)
    {
        newKeys[newKeysIndex++] = k;
        return;
    }

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

void myKeysRegistration(uint8_t k)
{
    switch (k)
    {
    case M_HIDMKY_FN_LOCK:
        fnLocked = !fnLocked;
        return;
    default:
        return;
    }
}

void languageKeysRegistration(uint8_t k)
{
}

void usageRegistration(uint16_t usage)
{
    consumerBuffer[0] = (uint8_t)(usage & 0xFF);
    consumerBuffer[1] = (uint8_t)(usage >> 8);
    noConsumerPressed = false;
}

void hidUsageKeysRegistration(uint8_t k)
{
    switch (k)
    {
    case M_HIDUC_SCAN_PREVIOUS:
        usageRegistration(HID_USAGE_CONSUMER_SCAN_PREVIOUS);
        return;
    case M_HIDUC_PLAY_PAUSE:
        usageRegistration(HID_USAGE_CONSUMER_PLAY_PAUSE);
        return;
    case M_HIDUC_SCAN_NEXT:
        usageRegistration(HID_USAGE_CONSUMER_SCAN_NEXT);
        return;
    case M_HIDUC_BRIGHTNESS_DECREMENT:
        usageRegistration(HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT);
        return;
    case M_HIDUC_BRIGHTNESS_INCREMENT:
        usageRegistration(HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT);
        return;
    case M_HIDUC_AL_CALCULATOR:
        usageRegistration(HID_USAGE_CONSUMER_AL_CALCULATOR);
        return;
    default:
        return;
    }
}

void otherHidKeysRegistration(uint8_t k)
{
    switch (k)
    {
    case M_HIDKEY_MUTE:
        normalKeyPressRegistration(HID_KEY_MUTE);
        return;
    case M_HIDKEY_VOLUME_DOWN:
        normalKeyPressRegistration(HID_KEY_VOLUME_DOWN);
        return;
    case M_HIDKEY_VOLUME_UP:
        normalKeyPressRegistration(HID_KEY_VOLUME_UP);
        return;
    case M_HIDKEY_FIND:
        normalKeyPressRegistration(HID_KEY_FIND);
        return;
    case M_HIDKEY_APPLICATION:
        normalKeyPressRegistration(HID_KEY_FIND);
        return;
    case M_HIDKEY_SCROLLLOCK:
        normalKeyPressRegistration(HID_KEY_SCROLL_LOCK);
        return;
    default:
        return;
    }
}

void fnKeyPressRegistration(uint8_t k)
{
    // Already pressed keys are priorised
    if (k == 0)
        return;
    // my keyboard features
    else if (k < 0x20)
        myKeysRegistration(k);
    // language key features
    else if (k < 0x40)
        languageKeysRegistration(k);
    // HID Usage Table (consumer Page)
    else if (k < 0x60)
        hidUsageKeysRegistration(k);
    // other HIDs
    else
        otherHidKeysRegistration(k);
}

void keyPressRegistration(uint8_t c, uint8_t r)
{
    // I assigned Fn to Europe 1 as I don't what it is lol
    uint8_t k = matrix[c][r];
    if (k == HID_KEY_EUROPE_1)
    {
        fnNewPressed = true;
        fnPressed = true;
        return;
    }

    if ((fnPressed && (k < HID_KEY_F1 || k > HID_KEY_F12)) || ((fnLocked ^ fnPressed) && k >= HID_KEY_F1 && k <= HID_KEY_F12))
    {
        fnKeyPressRegistration(fnMatrix[c][r]);
        return;
    }

    // normal keys
    if (k >= HID_KEY_CONTROL_LEFT)
    {
        modPressRegistration(k);
        return;
    }

    if (alreadyPressedNewKeysFull)
        return;

    // printf("%d | %d = ", c, r);
    normalKeyPressRegistration(matrix[c][r]);
}

void keyUpdateRegistration()
{
    for (uint8_t i = 0; i < NUMBER_OF_SIMULT_KEYS; i++)
    {
        alreadyPressedKeys[currentKeys[i]] = 0;
    }
    uint8_t *tmp = currentKeys;
    currentKeys = newKeys;
    newKeys = tmp;
    fnPressed = fnNewPressed;
    memset(newKeys, 0, NUMBER_OF_SIMULT_KEYS);

    //
    sendKeysReport();

    newKeysIndex = 0;
    currentMod = 0;

    noKeyPressedPreviously = noKeyPressed;
    noKeyPressed = true;

    fnNewPressed = false;

    noConsumerPressedPreviously = noConsumerPressed;
    noConsumerPressed = true;

    alreadyPressedNewKeysFull = false;
    for (uint8_t i = 0; i < NUMBER_OF_SIMULT_KEYS; i++)
    {
        if (currentKeys[i] > 0)
            alreadyPressedKeys[currentKeys[i]] = 1;
        // printf("{%d,%d,%d}", i, currentKeys[i], alreadyPressedKeys[currentKeys[i]]);
    }
    // printf("<(%d;%d),(%d;%d)...>\n", currentKeys[0], alreadyPressedKeys[currentKeys[0]], currentKeys[1], alreadyPressedKeys[currentKeys[1]]);
}

uint8_t raw[KB_COLS][KB_ROWS] = {0};
uint8_t filteredRaw[KB_COLS][KB_ROWS] = {0};

// --- Deghosting function ---
static void deghostBlockingAndRegister()
{
    for (int c = 0; c < KB_COLS; c++)
    {
        for (int r = 0; r < KB_ROWS; r++)
        {
            filteredRaw[c][r] = raw[c][r];
        }
    }
    // search rectangles (in O(n^4) oskur ~9k boucles)
    for (int c1 = 0; c1 < KB_COLS; c1++)
    {
        for (int c2 = c1 + 1; c2 < KB_COLS; c2++)
        {
            for (int r1 = 0; r1 < KB_ROWS; r1++)
            {
                for (int r2 = r1 + 1; r2 < KB_ROWS; r2++)
                {
                    if ( //(raw[c1][r1] && raw[c1][r2] && raw[c2][r1] && raw[c2][r2])
                        (raw[c1][r1] && raw[c1][r2] && raw[c2][r1]) ||
                        (raw[c1][r1] && raw[c1][r2] && raw[c2][r2]) ||
                        (raw[c1][r1] && raw[c2][r1] && raw[c2][r2]) ||
                        (raw[c1][r2] && raw[c2][r1] && raw[c2][r2]))
                    {
                        // printf("KEYS TO DROP: (%d;%d) (%d;%d) (%d;%d) (%d;%d)\n",
                        //        matrix[c1][r1],
                        //        alreadyPressedKeys[matrix[c1][r1]],
                        //        matrix[c1][r2],
                        //        alreadyPressedKeys[matrix[c1][r2]],
                        //        matrix[c2][r1],
                        //        alreadyPressedKeys[matrix[c2][r1]],
                        //        matrix[c2][r2],
                        //        alreadyPressedKeys[matrix[c2][r2]]);
                        // four corners
                        if (!alreadyPressedKeys[matrix[c1][r1]])
                            filteredRaw[c1][r1] = 0;
                        if (!alreadyPressedKeys[matrix[c1][r2]])
                            filteredRaw[c1][r2] = 0;
                        if (!alreadyPressedKeys[matrix[c2][r1]])
                            filteredRaw[c2][r1] = 0;
                        if (!alreadyPressedKeys[matrix[c2][r2]])
                            filteredRaw[c2][r2] = 0;

                        // if (!alreadyPressedKeys[matrix[c1][r1]])
                        //     printf("DROP: %d | %d = %d\n", c1, r1, matrix[c1][r1]);
                        // if (!alreadyPressedKeys[matrix[c1][r2]])
                        //     printf("DROP: %d | %d = %d\n", c1, r2, matrix[c1][r2]);
                        // if (!alreadyPressedKeys[matrix[c2][r1]])
                        //     printf("DROP: %d | %d = %d\n", c2, r1, matrix[c2][r1]);
                        // if (!alreadyPressedKeys[matrix[c2][r2]])
                        //     printf("DROP: %d | %d = %d\n", c2, r2, matrix[c2][r2]);
                    }
                }
            }
        }
    }
    // register remaining
    // printf("--------REGISTERING KEYS--------\n");
    for (int c = 0; c < KB_COLS; c++)
    {
        for (int r = 0; r < KB_ROWS; r++)
        {
            if (filteredRaw[c][r])
            {
                keyPressRegistration(c, r);
            }
            if (raw[c][r])
            {
                raw[c][r] = 0;
            }
        }
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
        GPIO_NUM_37, GPIO_NUM_38};
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
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&col_conf);

        // all columns are HIGH initially
        // gpio_set_level(cols[i], 1);
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

    start_buzzer_task();

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
                    if (i == col)
                    {
                        gpio_set_direction(cols[i], GPIO_MODE_OUTPUT_OD);
                        gpio_set_level(cols[i],0);
                    }
                    else
                    {
                        gpio_config_t col_conf = {
                            .pin_bit_mask = 1ULL << cols[i],
                            .mode = GPIO_MODE_INPUT,
                            .pull_up_en = GPIO_PULLUP_ENABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE,
                        };
                        gpio_config(&col_conf);
                    }
                    // gpio_set_level(cols[i], i == col ? 0 : 1);
                }

                // small delay for signal to settle
                esp_rom_delay_us(10); // seems sufficient ? check without debug prints

                // read all rows
                for (int row = 0; row < num_rows; ++row)
                {
                    int val = gpio_get_level(rows[row]);
                    if (val == 0)
                    {
                        raw[col][row] = 1;
                        // keyPressRegistration(col, row);
                    }
                }
            }
            deghostBlockingAndRegister();
            keyUpdateRegistration();
        }

        // delay before next scan
        vTaskDelay(pdMS_TO_TICKS(10));

        buzzer_off();
    }
}

/**
 *
 gpio_config_t back = {
        .pin_bit_mask = 1ULL << GPIO_NUM_3,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&back);

    // gpio_config_t skip = {
    //     .pin_bit_mask = 1ULL << GPIO_NUM_2,
    //     .mode = GPIO_MODE_INPUT,
    //     .pull_up_en = GPIO_PULLUP_ENABLE,
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .intr_type = GPIO_INTR_DISABLE,
    // };
    // gpio_config(&skip);
    // OR
    // buzzer_init();
    start_buzzer_task();

    // ESP_LOGI(TAG, "> back/skip buttons configured");
    // vTaskDelay(pdMS_TO_TICKS(20));
 *
 *

            // special keys
            // if (!gpio_get_level(GPIO_NUM_2))
            //     printf("back\n");
            if (!gpio_get_level(GPIO_NUM_3))
                printf("skip\n");


            keyUpdateRegistration();

            // buzzer_on();
 */