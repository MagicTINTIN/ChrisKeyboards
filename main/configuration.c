#include "configuration.h"
#include "constants.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "nvs.h"
#include <stdint.h>

void cfg_toggle_gamepad_interfaces(uint8_t current_state) {
  nvs_handle_t h;
  uint8_t next = current_state >= 2 ? 0 : current_state + 1;

  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
  ESP_ERROR_CHECK(nvs_set_u8(h, NVS_KEY_GAMEPAD_ITFS, next));
  ESP_ERROR_CHECK(nvs_commit(h));
  nvs_close(h);

  if (next > 0)
    cfg_toggle_gamepad_keys(0);
  else
    cfg_toggle_gamepad_keys(1);

  ESP_LOGI(TAG, "Enabled gamepads: %d, rebooting", next);
  vTaskDelay(pdMS_TO_TICKS(50)); // let the log message flush over UART
  esp_restart();
}

uint8_t cfg_read_enabled_gamepads(void) {
  nvs_handle_t h;
  uint8_t val = 0; // by default no gamepad interfae is active

  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
  if (err == ESP_OK) {
    nvs_get_u8(h, NVS_KEY_GAMEPAD_ITFS, &val); // ignore ERR_NOT_FOUND
    nvs_close(h);
  }
  // ERR_NVS_NOT_FOUND defaults to 0
  return val;
}

uint8_t cfg_toggle_gamepad_keys(uint8_t current_state) {
  nvs_handle_t h;
  uint8_t next = current_state ? 0 : 1;

  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
  ESP_ERROR_CHECK(nvs_set_u8(h, NVS_KEY_GAMEPAD, next));
  ESP_ERROR_CHECK(nvs_commit(h));
  nvs_close(h);
  return next;
}

uint8_t cfg_read_enabled_gamepads_keys(void) {
  nvs_handle_t h;
  uint8_t val = 0; // by default no gamepad is active

  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
  if (err == ESP_OK) {
    nvs_get_u8(h, NVS_KEY_GAMEPAD, &val); // ignore ERR_NOT_FOUND
    nvs_close(h);
  }
  // ERR_NVS_NOT_FOUND defaults to 0
  return val;
}


uint8_t cfg_set_sounds_level(uint8_t lvl) {
  nvs_handle_t h;

  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
  ESP_ERROR_CHECK(nvs_set_u8(h, NVS_KEY_SOUNDS_LEVEL, lvl));
  ESP_ERROR_CHECK(nvs_commit(h));
  nvs_close(h);
  return lvl;
}

uint8_t cfg_read_sounds_level(void) {
  nvs_handle_t h;
  uint8_t val = 1; // by default, sounds level is 1

  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &h);
  if (err == ESP_OK) {
    nvs_get_u8(h, NVS_KEY_SOUNDS_LEVEL, &val); // ignore ERR_NOT_FOUND
    nvs_close(h);
  }
  // ERR_NVS_NOT_FOUND defaults to 0
  return val;
}