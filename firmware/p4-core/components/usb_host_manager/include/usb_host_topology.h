/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USB_HOST_TOPOLOGY_ADDRESS_COUNT 128u

typedef struct {
    bool observed;
    bool present;
    bool direct_root_child;
    uint8_t root_port_index;
    uintptr_t device_token;
    uintptr_t parent_token;
} usb_host_topology_entry_t;

typedef struct {
    usb_host_topology_entry_t entries[USB_HOST_TOPOLOGY_ADDRESS_COUNT];
} usb_host_topology_t;

void usb_host_topology_init(usb_host_topology_t *topology);
bool usb_host_topology_observe(usb_host_topology_t *topology,
                               uint8_t address,
                               bool direct_root_child,
                               uint8_t root_port_index);
bool usb_host_topology_remove(usb_host_topology_t *topology, uint8_t address);
/* Opaque handle tokens are compared only, never dereferenced. Parent publication
 * may follow a child's; unresolved ancestry remains fail-closed until known. */
bool usb_host_topology_observe_parent(usb_host_topology_t *topology,
                                      uint8_t address, uintptr_t device_token,
                                      uintptr_t parent_token, uint8_t parent_port);
bool usb_host_topology_matches_root(const usb_host_topology_t *topology,
                                    uint8_t address,
                                    uint8_t root_port_index,
                                    bool require_direct_root,
                                    bool *known_out);

#ifdef __cplusplus
}
#endif
