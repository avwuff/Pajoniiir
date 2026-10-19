/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "sd_cold_power.h"

typedef struct {
    int step, fail_at, fail_restore;
    int on, off, clamp, restore, waits;
    bool powered, pins[6];
    char trace[64];
} fixture_t;

static esp_err_t event(fixture_t *f, char code)
{
    f->trace[f->step++] = code;
    return f->step == f->fail_at ? ESP_FAIL : ESP_OK;
}
static esp_err_t on(void *ctx)
{
    fixture_t *f = ctx;
    for (int i = 0; i < 6; i++) assert(!f->pins[i]);
    ++f->on;
    esp_err_t rc = event(f, 'N');
    if (rc == ESP_OK) f->powered = true;
    return rc;
}
static esp_err_t off(void *ctx)
{
    fixture_t *f = ctx;
    assert(f->powered);
    ++f->off;
    esp_err_t rc = event(f, 'F');
    if (rc == ESP_OK) f->powered = false;
    return rc;
}
static esp_err_t clamp(void *ctx, size_t pin)
{
    fixture_t *f = ctx;
    assert(!f->powered && pin < 6);
    f->pins[pin] = true; /* Failed call may already have changed the pin. */
    ++f->clamp;
    return event(f, 'L');
}
static esp_err_t restore(void *ctx, size_t pin)
{
    fixture_t *f = ctx;
    assert(!f->powered && pin < 6);
    ++f->restore;
    esp_err_t rc = event(f, 'R');
    if ((int)pin == f->fail_restore) rc = ESP_ERR_TIMEOUT;
    if (rc == ESP_OK) f->pins[pin] = false;
    return rc;
}
static void wait_ms(void *ctx, unsigned ms)
{
    fixture_t *f = ctx;
    assert(ms == 150);
    ++f->waits;
    assert(event(f, 'W') == ESP_OK);
}
static const jc_sd_power_ops_t ops = {on, off, clamp, restore, wait_ms};

int main(void)
{
    fixture_t good = {.fail_restore = -1};
    jc_sd_power_state_t state = {0};
    assert(jc_sd_cold_power_once(&state, &ops, &good, true) == ESP_OK);
    assert(strcmp(good.trace, "NFLLLLLLWRRRRRRNW") == 0);
    assert(good.powered && good.on == 2 && good.off == 1 && good.waits == 2);
    assert(jc_sd_cold_power_once(&state, &ops, &good, false) == ESP_OK);
    assert(good.step == 17); /* No second cycle after slot ownership transfers. */

    /* Every fallible operation fails independently. Both waits are infallible. */
    for (int fault = 1; fault <= 16; ++fault) {
        if (fault == 9) continue;
        fixture_t f = {.fail_at = fault, .fail_restore = -1};
        state = (jc_sd_power_state_t){0};
        assert(jc_sd_cold_power_once(&state, &ops, &f, true) == ESP_FAIL);
        int steps = f.step;
        assert(jc_sd_cold_power_once(&state, &ops, &f, true) == ESP_FAIL);
        assert(f.step == steps); /* A failed attempt is latched, never mounted. */
        if (fault <= 2) assert(f.clamp == 0 && f.restore == 0);
        else assert(f.restore == 6);
        if (fault == 2) assert(f.powered); /* Failed release retains ownership. */
        if (fault >= 3) assert(!f.powered);
    }
    /* Secondary cleanup failure must not permit power-on, nor hide first error. */
    fixture_t f = {.fail_at = 5, .fail_restore = 0};
    state = (jc_sd_power_state_t){0};
    assert(jc_sd_cold_power_once(&state, &ops, &f, true) == ESP_FAIL);
    assert(f.restore == 6 && f.on == 1 && !f.powered);
    state = (jc_sd_power_state_t){0};
    f = (fixture_t){.fail_restore = -1};
    assert(jc_sd_cold_power_once(&state, &ops, &f, false) == ESP_ERR_INVALID_STATE);
    assert(f.step == 0);
    assert(jc_sd_cold_power_once(NULL, &ops, &f, true) == ESP_ERR_INVALID_ARG);
    state = (jc_sd_power_state_t){0};
    jc_sd_power_ops_t bad = ops;
    bad.restore_pin = NULL;
    assert(jc_sd_cold_power_once(&state, &bad, &f, true) == ESP_ERR_INVALID_ARG);
    assert(!state.attempted && f.step == 0);
    puts("PASS SD cold power: sequencing, 15 injected failures, cleanup, ownership, one-shot");
    return 0;
}
