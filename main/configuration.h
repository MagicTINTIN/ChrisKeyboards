#pragma once

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

void cfg_toggle_gamepad_interfaces(uint8_t current_state);
uint8_t cfg_read_enabled_gamepads(void);

uint8_t cfg_toggle_gamepad_keys(uint8_t current_state);
uint8_t cfg_read_enabled_gamepads_keys(void);

#endif // CONFIGURATION_H