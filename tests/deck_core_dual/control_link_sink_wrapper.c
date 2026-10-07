/* Exercise the real semantic input adapter while keeping this suite's LED fake. */
#define control_link_send_led host_unused_control_link_send_led
#define control_link_send_led_deck host_unused_control_link_send_led_deck
#include "../../firmware/p4-core/components/control_link/control_link_local.c"
