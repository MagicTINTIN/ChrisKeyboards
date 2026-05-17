#pragma once

#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "class/hid/hid.h"
#include <stdint.h>

////////////////////////////////
// CONSTANTS FOR THE KEYBOARD //
////////////////////////////////

#define HIDD_DEVICE_NAME "ChrisT1 Clavier"
#define TAG "CT1"

////////////////////////////////
// INTERFACES

#define HID_ITF_MOUSEKYB 0
#define HID_ITF_CONSUMER 1
#define HID_ITF_GAMEPAD1 2
#define HID_ITF_GAMEPAD2 3
#define NUMBER_OF_MYKEYS 32

////////////////////////////////
// NON-VOLATILE MEMORY

#define NVS_NAMESPACE "ctrl_cfg"
#define NVS_KEY_GAMEPAD_ITFS "gp_itfs" // gamepads interfaces enable
#define NVS_KEY_GAMEPAD "gp_en"        // gamepads enable
#define NVS_KEY_SOUNDS_LEVEL "sound_lvl" // level of enabled sounds // 0=none, 1=ony usefull, 2=all
#define NVS_KEY_SOUNDS_MUTE "sound_mute" // bypass sounds level to mute

////////////////////////////////
// MATRIX SPECS

#define KB_COLS 8
#define KB_ROWS 17
#define MAX_RAW_KEYS (KB_COLS * KB_ROWS)

#define GPIO_CAPS_LED GPIO_NUM_21

////////////////////////////////
// BUZZER CONFIG

#define BUZZER_GPIO 2
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_TIMER LEDC_TIMER_0

////////////////////////////////
// KEYBOARD SPECS

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
#define NUMBER_OF_SIMULT_KEYS 6

////////////////////////////////
// KEYS

#define M_HID_UNDEF 0x0

// MY KEYS
#define M_HIDMKY_FN_LOCK 0x1
#define M_HIDMK_BACKLIGHT 0x2
#define M_HIDMK_GAMEPADS 0x3

// #define M_HIDMK_SOUNDS_MUTE 0x4
// #define M_HIDMK_SOUNDS_SOME 0x5
// #define M_HIDMK_SOUNDS_ALL 0x6

// LANGUAGES
#define M_HIDMK_MORSE 0x20
#define M_HIDMK_HEXA 0x21
#define M_HIDMK_BIN 0x22

// UC
#define M_HIDUC_SCAN_PREVIOUS 0x40
#define M_HIDUC_PLAY_PAUSE 0x41
#define M_HIDUC_SCAN_NEXT 0x43
#define M_HIDUC_BRIGHTNESS_DECREMENT 0x44
#define M_HIDUC_BRIGHTNESS_INCREMENT 0x45
#define M_HIDUC_AL_CALCULATOR 0x46

// CLASSIC KEYS
#define M_HIDKEY_MUTE 0x60
#define M_HIDKEY_VOLUME_DOWN 0x61
#define M_HIDKEY_VOLUME_UP 0x62
#define M_HIDKEY_FIND 0x63
#define M_HIDKEY_APPLICATION 0x64
#define M_HIDKEY_SCROLLLOCK 0x65
#define M_HIDKEY_ARROW_PAGE_DOWN 0X66
#define M_HIDKEY_ARROW_PAGE_UP 0X67
#define M_HIDKEY_ARROW_BEGIN 0X68
#define M_HIDKEY_ARROW_END 0X69

#define NUMBER_OF_FN_KEYS 128

////////////////////////////////
// GAMEPADS KEYS

// common controls
#define KEY_CONTROLLER_LEFT_STICK_RIGHT 0
#define KEY_CONTROLLER_LEFT_STICK_LEFT 1
#define KEY_CONTROLLER_LEFT_STICK_DOWN 2
#define KEY_CONTROLLER_LEFT_STICK_UP 3

#define KEY_CONTROLLER_RIGHT_STICK_RIGHT 4
#define KEY_CONTROLLER_RIGHT_STICK_LEFT 5
#define KEY_CONTROLLER_RIGHT_STICK_DOWN 6
#define KEY_CONTROLLER_RIGHT_STICK_UP 7

#define KEY_CONTROLLER_DPAD_RIGHT 8
#define KEY_CONTROLLER_DPAD_LEFT 9
#define KEY_CONTROLLER_DPAD_DOWN 10
#define KEY_CONTROLLER_DPAD_UP 11

#define KEY_CONTROLLER_BUTTON_X 12
#define KEY_CONTROLLER_BUTTON_Y 13
#define KEY_CONTROLLER_BUTTON_A 14
#define KEY_CONTROLLER_BUTTON_B 15

#define KEY_CONTROLLER_BUMPER_LEFT 16
#define KEY_CONTROLLER_BUMPER_RIGHT 17

#define KEY_CONTROLLER_BUTTON_VIEW 18
#define KEY_CONTROLLER_BUTTON_HOME 19
#define KEY_CONTROLLER_BUTTON_MENU 20

#define KEY_CONTROLLER_END 21

#define GC_BTN_A (1u << 0)
#define GC_BTN_B (1u << 1)
#define GC_BTN_X (1u << 2)
#define GC_BTN_Y (1u << 3)
#define GC_BTN_LB (1u << 4)
#define GC_BTN_RB (1u << 5)
#define GC_BTN_LT_DIG (1u << 6) /* digital LT press */
#define GC_BTN_RT_DIG (1u << 7) /* digital RT press */
#define GC_BTN_SELECT (1u << 8)
#define GC_BTN_START (1u << 9)
#define GC_BTN_L3 (1u << 10) /* left-stick click  */
#define GC_BTN_R3 (1u << 11) /* right-stick click */
#define GC_BTN_HOME (1u << 12)
#define GC_BTN_SHARE (1u << 13)

const static uint8_t DPADS_DIRECTIONS[3][3] = {
    {GAMEPAD_HAT_UP_LEFT, GAMEPAD_HAT_UP, GAMEPAD_HAT_UP_RIGHT},
    {GAMEPAD_HAT_LEFT, GAMEPAD_HAT_CENTERED, GAMEPAD_HAT_RIGHT},
    {GAMEPAD_HAT_DOWN_LEFT, GAMEPAD_HAT_DOWN, GAMEPAD_HAT_DOWN_RIGHT},
};

// gamepads
#define CONTROLER1_OFFSET 1
#define CONTROLER2_OFFSET (CONTROLER1_OFFSET + KEY_CONTROLLER_END)

#define GC1O(control) ((control) + CONTROLER1_OFFSET)
#define GC2O(control) ((control) + CONTROLER2_OFFSET)

#endif // CONSTANTS_H