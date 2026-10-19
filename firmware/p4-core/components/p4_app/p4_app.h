#pragma once
#include <stdbool.h>
#include "esp_err.h"
typedef struct {
    bool required;
    esp_err_t (*start)(void);
    bool (*ready)(void);
} p4_app_network_provider_t;
void pajoniiir_common_app_main(void);
void pajoniiir_common_app_main_with_network(const p4_app_network_provider_t *provider);
