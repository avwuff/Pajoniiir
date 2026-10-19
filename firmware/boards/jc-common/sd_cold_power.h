/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

/* Startup-only, single-owner operation. No SDMMC slot may own these pins yet.
 * Never call this from a mount retry, hotplug callback or runtime recovery.
 * The callbacks own the LDO handle and must preserve it when release fails. */
typedef struct {
    esp_err_t (*power_on)(void *ctx);
    esp_err_t (*power_off)(void *ctx);
    esp_err_t (*clamp_low)(void *ctx, size_t pin);
    esp_err_t (*restore_pin)(void *ctx, size_t pin);
    void (*wait_ms)(void *ctx, unsigned ms);
} jc_sd_power_ops_t;

typedef struct {
    bool attempted;
    esp_err_t result;
} jc_sd_power_state_t;

esp_err_t jc_sd_cold_power_once(jc_sd_power_state_t *state,
                               const jc_sd_power_ops_t *ops, void *ctx,
                               bool slot_unclaimed);
