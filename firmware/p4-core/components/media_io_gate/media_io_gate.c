#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "media_io_gate.h"

#if defined(MEDIA_IO_GATE_STANDALONE_TEST)

static bool s_locked;
static bool s_available;
#if defined(_WIN32)
#include <windows.h>
static uint64_t host_ms(void) { return GetTickCount64(); }
static void host_yield(void) { Sleep(1); }
#else
#include <time.h>
static uint64_t host_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000u + (uint64_t)now.tv_nsec / 1000000u;
}
static void host_yield(void) { const struct timespec tick = {0, 1000000}; nanosleep(&tick, NULL); }
#endif

esp_err_t media_io_gate_init(void)
{
    __atomic_store_n(&s_locked, false, __ATOMIC_RELEASE);
    __atomic_store_n(&s_available, false, __ATOMIC_RELEASE);
    return ESP_OK;
}

void media_io_gate_begin(void)
{
    while (!media_io_gate_try_begin(0)) host_yield();
}

bool media_io_gate_try_begin(uint32_t timeout_ms)
{
    const uint64_t started = host_ms();
    for (;;) {
        bool expected = false;
        if (__atomic_compare_exchange_n(&s_locked, &expected, true, false,
                                        __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) return true;
        if (!timeout_ms || host_ms() - started >= timeout_ms) return false;
        host_yield();
    }
}

void media_io_gate_end(void)
{
    __atomic_store_n(&s_locked, false, __ATOMIC_RELEASE);
}

void media_io_gate_set_available(bool available)
{
    __atomic_store_n(&s_available, available, __ATOMIC_RELEASE);
}

bool media_io_gate_is_available(void)
{
    return __atomic_load_n(&s_available, __ATOMIC_ACQUIRE);
}

#else

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "stdatomic.h"

static const char *TAG = "media_io_gate";
static SemaphoreHandle_t s_gate;
static atomic_bool s_available;

esp_err_t media_io_gate_init(void)
{
    if (__atomic_load_n(&s_gate, __ATOMIC_ACQUIRE)) {
        return ESP_OK;
    }
    SemaphoreHandle_t candidate = xSemaphoreCreateMutex();
    if (!candidate) {
        if (__atomic_load_n(&s_gate, __ATOMIC_ACQUIRE)) return ESP_OK;
        ESP_LOGE(TAG, "failed to create USB media I/O mutex");
        return ESP_FAIL;
    }
    SemaphoreHandle_t expected = NULL;
    if (!__atomic_compare_exchange_n(&s_gate, &expected, candidate, false,
                                    __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
        vSemaphoreDelete(candidate);
    return ESP_OK;
}

void media_io_gate_begin(void)
{
    ESP_ERROR_CHECK(media_io_gate_init());
    if (xSemaphoreTake(__atomic_load_n(&s_gate, __ATOMIC_ACQUIRE), portMAX_DELAY) != pdTRUE)
        ESP_ERROR_CHECK(ESP_ERR_INVALID_STATE);
}

bool media_io_gate_try_begin(uint32_t timeout_ms)
{
    if (!__atomic_load_n(&s_gate, __ATOMIC_ACQUIRE)) {
        if (media_io_gate_init() != ESP_OK) {
            return false;
        }
    }
    return xSemaphoreTake(__atomic_load_n(&s_gate, __ATOMIC_ACQUIRE), pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void media_io_gate_end(void)
{
    SemaphoreHandle_t gate = __atomic_load_n(&s_gate, __ATOMIC_ACQUIRE);
    if (gate) {
        xSemaphoreGive(gate);
    }
}

void media_io_gate_set_available(bool available)
{
    atomic_store_explicit(&s_available, available, memory_order_release);
}

bool media_io_gate_is_available(void)
{
    return atomic_load_explicit(&s_available, memory_order_acquire);
}

#endif
