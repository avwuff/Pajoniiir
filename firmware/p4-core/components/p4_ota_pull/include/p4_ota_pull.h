#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"
#include "p4_ota_pull_manifest.h"

/*
 * Update check over the temporary STA visit.
 *
 * Owns the whole round trip: leave the AP, fetch the channel document, parse
 * it, come back. Deliberately stops there - it reports what is available and
 * flashes nothing. Downloading and installing is a separate, explicitly
 * confirmed step, because an update that installs itself the moment it is
 * noticed is exactly what a DJ does not want mid-set.
 *
 * Depends on wifi_link; wifi_link does not depend on this, so there is no
 * cycle. The web layer reaches it through app_main, for the same reason.
 */

typedef enum {
    P4_OTA_PULL_IDLE = 0,
    P4_OTA_PULL_CHECKING,
    P4_OTA_PULL_UP_TO_DATE,
    P4_OTA_PULL_AVAILABLE,
    P4_OTA_PULL_DOWNLOADING,
    P4_OTA_PULL_READY_TO_REBOOT,
    P4_OTA_PULL_FAILED,
} p4_ota_pull_state_t;

typedef struct {
    p4_ota_pull_state_t state;
    esp_err_t last_error;
    char detail[64];                        /* stage or failure, operator-facing */
    char available_release[P4_OTA_PULL_RELEASE_MAX + 1u];
    uint32_t available_size;
    uint32_t downloaded;     /* bytes written so far, for a progress readout */
} p4_ota_pull_status_t;

/* ESP_ERR_INVALID_ARG: missing SSID/URL or invalid URL.
 * ESP_ERR_INVALID_STATE: another operation or Wi-Fi transition holds the lease.
 * ESP_ERR_NOT_SUPPORTED: the running version does not support pull updates.
 * ESP_ERR_NO_MEM: worker creation failed. Wi-Fi state is not checked here. The
 * caller must refuse while audio is playing - this component knows nothing
 * about decks. */
esp_err_t p4_ota_pull_check_start(void);

p4_ota_pull_status_t p4_ota_pull_get_status(void);

/*
 * Download and install what the last check found, then reboot.
 *
 * Separate from the check, and requiring the release to be named back, because
 * this is the irreversible half: it writes the inactive OTA slot and reboots
 * into it. Naming the release means a stale page cannot install something the
 * operator never saw.
 *
 * Authenticity is unchanged from the push path - the same signed .ddjota
 * manifest, verified by the same ota_manifest code before a byte reaches
 * flash. Nothing about the bundle is trusted because it arrived over TLS.
 *
 * ESP_ERR_INVALID_STATE: busy operation/transition, unavailable or expired
 * offer, or incomplete offer metadata.
 * ESP_ERR_INVALID_ARG: missing or mismatched expected_release.
 * ESP_ERR_NO_MEM: offer allocation or worker creation failed.
 */
esp_err_t p4_ota_pull_install_start(const char *expected_release);
