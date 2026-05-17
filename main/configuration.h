#pragma once

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

void cfg_toggle_gamepad_interfaces(uint8_t current_state);
uint8_t cfg_read_enabled_gamepads(void);

uint8_t cfg_toggle_gamepad_keys(uint8_t current_state);
uint8_t cfg_read_enabled_gamepads_keys(void);

uint8_t cfg_set_sounds_level(uint8_t lvl);
uint8_t cfg_get_sounds_level(void);

#endif // CONFIGURATION_H