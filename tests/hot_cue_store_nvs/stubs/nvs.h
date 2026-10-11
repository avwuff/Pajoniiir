#pragma once
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef unsigned nvs_handle_t;
typedef enum { NVS_READONLY, NVS_READWRITE } nvs_open_mode_t;
#define ESP_ERR_NVS_NOT_FOUND 0x1102

esp_err_t nvs_open(const char *ns, nvs_open_mode_t mode, nvs_handle_t *out);
esp_err_t nvs_get_blob(nvs_handle_t h, const char *key, void *data, size_t *len);
esp_err_t nvs_set_blob(nvs_handle_t h, const char *key, const void *data, size_t len);
esp_err_t nvs_erase_key(nvs_handle_t h, const char *key);
esp_err_t nvs_commit(nvs_handle_t h);
void nvs_close(nvs_handle_t h);
