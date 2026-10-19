/* JC1060 entrypoint: common P4 startup/core, board-local Ethernet bring-up. */
#include "p4_app.h"
#include "board_adapter.h"
#include "esp_log.h"
#include "board_ethernet.h"
#include "dj_link_service.h"
#include "web_server.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "sdkconfig.h"
static bool remote_address(char *out, size_t size)
{
    esp_netif_t *netif = board_ethernet_netif();
    esp_netif_ip_info_t ip = {0};
    return netif && esp_netif_is_netif_up(netif) &&
        esp_netif_get_ip_info(netif, &ip) == ESP_OK && ip.ip.addr &&
        esp_ip4addr_ntoa(&ip.ip, out, size) != NULL;
}
static esp_err_t remote_start(void)
{
    web_server_set_address_probe(remote_address);
    return web_server_start();
}
static bool remote_ready(void)
{
    char address[16];
    return web_server_is_running() && remote_address(address, sizeof(address));
}
void app_main(void)
{
    /* PA must be low before network/NVS work, as on JC4880. */
    ESP_ERROR_CHECK(bsp_audio_force_safe_boot_state());
    esp_err_t rc = board_ethernet_start();
    if (rc != ESP_OK) ESP_LOGE("jc1060", "Ethernet unavailable: %s", esp_err_to_name(rc));
    const p4_app_network_provider_t remote = {
        .required = true, .start = remote_start, .ready = remote_ready,
    };
    pajoniiir_common_app_main_with_network(&remote);
#if CONFIG_PAJONIIIR_DJ_LINK_SERVICE
    rc = dj_link_service_init(board_ethernet_netif);
    if (rc != ESP_OK) ESP_LOGE("jc1060", "DJ Link service unavailable: %s", esp_err_to_name(rc));
#endif
}
