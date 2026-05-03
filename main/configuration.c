#include "configuration.h"
#include "constants.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "nvs.h"

void cfg_toggle_gamepad_interfaces(uint8_t current_state) {
  nvs_handle_t h;
  uint8_t next = current_state >= 2 ? 0 : current_state + 1;

  ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h));
  ESP_ERROR_CHECK(nvs_set_u8(h, NVS_KEY_GAMEPAD, next));
  ESP_ERROR_CHECK(nvs_commit(h));
  nvs_close(h);

  ESP_LOGI(TAG, "Enabled gamepads: %d, rebooting", next);
  vTaskDelay(pdMS_TO_TICKS(100)); // let the log message flush over UART
  esp_restart();
}

uint8_t cfg_read_enabled_gamepads(void) {
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