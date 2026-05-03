#pragma once

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <stdint.h>

void cfg_toggle_gamepad_interfaces(uint8_t current_state);
uint8_t cfg_read_enabled_gamepads(void);

#endif // CONFIGURATION_H