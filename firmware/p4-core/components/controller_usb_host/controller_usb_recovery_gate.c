/* SPDX-License-Identifier: Apache-2.0 */
#include "controller_usb_recovery_gate.h"

void controller_usb_recovery_gate_init(controller_usb_recovery_gate_t *gate)
{
    if (!gate) {
        return;
    }
    gate->pending = false;
    gate->fault_epochs = 0u;
    gate->in_pipe_retries = 0u;
}

void controller_usb_recovery_gate_reset_in_retries(controller_usb_recovery_gate_t *gate)
{
    if (gate) gate->in_pipe_retries = 0u;
}

bool controller_usb_recovery_gate_take_in_retry(controller_usb_recovery_gate_t *gate,
                                                bool high_speed, bool generic_error)
{
    if (!gate || gate->pending || !high_speed || !generic_error ||
        gate->in_pipe_retries >= 3u) return false;
    gate->in_pipe_retries++;
    return true;
}

bool controller_usb_recovery_gate_begin_fault(
    controller_usb_recovery_gate_t *gate)
{
    if (!gate || gate->pending) {
        return false;
    }
    gate->pending = true;
    gate->fault_epochs++;
    return true;
}

void controller_usb_recovery_gate_cancel(controller_usb_recovery_gate_t *gate)
{
    if (gate) {
        gate->pending = false;
    }
}

bool controller_usb_recovery_gate_pending(
    const controller_usb_recovery_gate_t *gate)
{
    return gate && gate->pending;
}

void controller_usb_recovery_gate_complete(
    controller_usb_recovery_gate_t *gate)
{
    if (gate) {
        gate->pending = false;
    }
}

uint32_t controller_usb_recovery_gate_fault_epochs(
    const controller_usb_recovery_gate_t *gate)
{
    return gate ? gate->fault_epochs : 0u;
}
