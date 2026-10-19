#include "usb_host_topology.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned checks;
#define CHECK(x) do { checks++; if (!(x)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); \
} } while (0)

int main(void)
{
    usb_host_topology_t topology;
    usb_host_topology_init(&topology);

    bool known = true;
    CHECK(!usb_host_topology_matches_root(&topology, 4u, 0u, true, &known));
    CHECK(!known);
    CHECK(!usb_host_topology_observe(&topology, 0u, true, 0u));
    CHECK(usb_host_topology_observe(&topology, 4u, true, 0u));
    CHECK(usb_host_topology_matches_root(&topology, 4u, 0u, true, &known));
    CHECK(known);
    CHECK(!usb_host_topology_matches_root(&topology, 4u, 1u, true, &known));
    CHECK(known);

    CHECK(usb_host_topology_observe(&topology, 7u, false, 1u));
    CHECK(usb_host_topology_matches_root(&topology, 7u, 1u, false, &known));
    CHECK(!usb_host_topology_matches_root(&topology, 7u, 1u, true, &known));
    CHECK(known);

    CHECK(usb_host_topology_remove(&topology, 4u));
    CHECK(!usb_host_topology_matches_root(&topology, 4u, 0u, true, &known));
    CHECK(!known);
    CHECK(usb_host_topology_observe(&topology, 4u, true, 1u));
    CHECK(usb_host_topology_matches_root(&topology, 4u, 1u, true, &known));

    usb_host_topology_init(&topology);
    /* Child first: its downstream port 1 is NOT a root index. */
    CHECK(usb_host_topology_observe_parent(&topology, 6u, 600u, 500u, 1u));
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 1u, false, &known));
    CHECK(!known);
    CHECK(usb_host_topology_observe_parent(&topology, 5u, 500u, 0u, 0u));
    CHECK(usb_host_topology_matches_root(&topology, 6u, 0u, false, &known));
    CHECK(known);
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 0u, true, &known));
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 1u, false, &known));
    CHECK(known);
    /* Nested ancestry and delayed parent publication. */
    CHECK(usb_host_topology_observe_parent(&topology, 8u, 800u, 700u, 4u));
    CHECK(!usb_host_topology_matches_root(&topology, 8u, 0u, false, &known));
    CHECK(!known);
    CHECK(usb_host_topology_observe_parent(&topology, 7u, 700u, 500u, 3u));
    CHECK(usb_host_topology_matches_root(&topology, 8u, 0u, false, &known));
    CHECK(usb_host_topology_remove(&topology, 5u));
    CHECK(!usb_host_topology_matches_root(&topology, 8u, 0u, false, &known));
    CHECK(!known);
    /* Reused hub address/handle cannot revive a stale child. */
    CHECK(usb_host_topology_observe_parent(&topology, 5u, 500u, 0u, 1u));
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 1u, false, &known));
    CHECK(!known);
    CHECK(usb_host_topology_observe_parent(&topology, 6u, 601u, 500u, 2u));
    CHECK(usb_host_topology_matches_root(&topology, 6u, 1u, false, &known));
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 0u, false, &known));
    /* Cycles, ambiguous tokens and invalid roots never authorize media. */
    CHECK(usb_host_topology_observe_parent(&topology, 9u, 900u, 901u, 1u));
    CHECK(usb_host_topology_observe_parent(&topology, 10u, 901u, 900u, 2u));
    CHECK(!usb_host_topology_matches_root(&topology, 9u, 0u, false, &known));
    CHECK(!known);
    CHECK(!usb_host_topology_observe_parent(&topology, 11u, 1u, 1u, 0u));
    CHECK(!usb_host_topology_observe_parent(&topology, 11u, 0u, 0u, 0u));
    CHECK(!usb_host_topology_observe_parent(&topology, 11u, 1u, 0u, 32u));
    CHECK(usb_host_topology_observe_parent(&topology, 12u, 500u, 0u, 0u));
    CHECK(!usb_host_topology_matches_root(&topology, 6u, 1u, false, &known));
    CHECK(!known);

    printf("USB host topology: %u checks passed\n", checks);
    return 0;
}
