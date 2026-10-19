/* SPDX-License-Identifier: MIT
 * Board-owned adaptation of donor 981225e5 / 1e288dbd, not a host reset. */
#include "sd_cold_power.h"

esp_err_t jc_sd_cold_power_once(jc_sd_power_state_t *state,
                               const jc_sd_power_ops_t *ops, void *ctx,
                               bool slot_unclaimed)
{
    if (!state || !ops || !ops->power_on || !ops->power_off ||
        !ops->clamp_low || !ops->restore_pin || !ops->wait_ms) {
        return ESP_ERR_INVALID_ARG;
    }
    if (state->attempted) return state->result;
    state->attempted = true;
    state->result = ESP_ERR_INVALID_STATE;
    if (!slot_unclaimed) return state->result;

    esp_err_t rc = ops->power_on(ctx);
    if (rc == ESP_OK) rc = ops->power_off(ctx);
    /* A failed release can retain a live driver/rail. Do not drive pins then. */
    if (rc != ESP_OK) return state->result = rc;

    for (size_t i = 0; i < 6; ++i) {
        rc = ops->clamp_low(ctx, i);
        if (rc != ESP_OK) break;
    }
    if (rc == ESP_OK) ops->wait_ms(ctx, 150);

    /* Restore ALL pins, including the partially configured failed one. Never
     * power a card back up while a failed cleanup may still drive its lines. */
    for (size_t i = 0; i < 6; ++i) {
        esp_err_t cleanup = ops->restore_pin(ctx, i);
        if (rc == ESP_OK && cleanup != ESP_OK) rc = cleanup;
    }
    if (rc == ESP_OK) rc = ops->power_on(ctx);
    if (rc == ESP_OK) ops->wait_ms(ctx, 150);
    return state->result = rc;
}
