/* SPDX-License-Identifier: Apache-2.0 */
#include "usb_host_topology.h"

#include <string.h>

void usb_host_topology_init(usb_host_topology_t *topology)
{
    if (topology) {
        memset(topology, 0, sizeof(*topology));
    }
}

bool usb_host_topology_observe(usb_host_topology_t *topology,
                               uint8_t address,
                               bool direct_root_child,
                               uint8_t root_port_index)
{
    if (!topology || address == 0u ||
        address >= USB_HOST_TOPOLOGY_ADDRESS_COUNT || root_port_index >= 32u) {
        return false;
    }
    (void)usb_host_topology_remove(topology, address);
    topology->entries[address] = (usb_host_topology_entry_t) {
        .observed = true,
        .present = true,
        .direct_root_child = direct_root_child,
        .root_port_index = root_port_index,
    };
    return true;
}

bool usb_host_topology_remove(usb_host_topology_t *topology, uint8_t address)
{
    if (!topology || address == 0u ||
        address >= USB_HOST_TOPOLOGY_ADDRESS_COUNT) {
        return false;
    }
    uintptr_t retired[USB_HOST_TOPOLOGY_ADDRESS_COUNT];
    unsigned count = 0u;
    if (topology->entries[address].device_token)
        retired[count++] = topology->entries[address].device_token;
    topology->entries[address] = (usb_host_topology_entry_t){.observed = true};
    /* Retire descendants before a parent address/handle can be reused. */
    for (unsigned cursor = 0u; cursor < count; ++cursor) {
        for (unsigned i = 1u; i < USB_HOST_TOPOLOGY_ADDRESS_COUNT; ++i) {
            usb_host_topology_entry_t *child = &topology->entries[i];
            if (!child->present || child->parent_token != retired[cursor]) continue;
            if (child->device_token && count < USB_HOST_TOPOLOGY_ADDRESS_COUNT)
                retired[count++] = child->device_token;
            *child = (usb_host_topology_entry_t){.observed = true};
        }
    }
    return true;
}

bool usb_host_topology_observe_parent(usb_host_topology_t *topology,
                                      uint8_t address, uintptr_t device_token,
                                      uintptr_t parent_token, uint8_t parent_port)
{
    if (!topology || !device_token || !address ||
        address >= USB_HOST_TOPOLOGY_ADDRESS_COUNT ||
        device_token == parent_token || (!parent_token && parent_port >= 32u))
        return false;
    (void)usb_host_topology_remove(topology, address);
    topology->entries[address] = (usb_host_topology_entry_t){
        .observed = true, .present = true, .direct_root_child = !parent_token,
        .root_port_index = parent_token ? UINT8_MAX : parent_port,
        .device_token = device_token, .parent_token = parent_token,
    };
    return true;
}

bool usb_host_topology_matches_root(const usb_host_topology_t *topology,
                                    uint8_t address,
                                    uint8_t root_port_index,
                                    bool require_direct_root,
                                    bool *known_out)
{
    if (known_out) {
        *known_out = false;
    }
    if (!topology || address == 0u ||
        address >= USB_HOST_TOPOLOGY_ADDRESS_COUNT) {
        return false;
    }
    const usb_host_topology_entry_t *entry = &topology->entries[address];
    if (!entry->observed || !entry->present) {
        return false;
    }
    const bool direct = entry->direct_root_child;
    /* USB permits bounded hub depth. Also bounds malformed/cyclic metadata. */
    for (unsigned depth = 0u; depth < 8u && entry->parent_token; ++depth) {
        const usb_host_topology_entry_t *parent = NULL;
        for (unsigned i = 1u; i < USB_HOST_TOPOLOGY_ADDRESS_COUNT; ++i) {
            const usb_host_topology_entry_t *candidate = &topology->entries[i];
            if (candidate->present && candidate->device_token == entry->parent_token) {
                if (parent) return false; /* ambiguous reused handle */
                parent = candidate;
            }
        }
        if (!parent) return false;
        entry = parent;
    }
    if (entry->parent_token || entry->root_port_index >= 32u) return false;
    if (known_out) *known_out = true;
    return entry->root_port_index == root_port_index &&
           (!require_direct_root || direct);
}
