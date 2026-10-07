#include <stdio.h>
#include <stdlib.h>
#include "controller_profile_runtime.h"
#include "control_link.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    CHECK(file != NULL);
    unsigned char blob[16384];
    size_t length = fread(blob, 1, sizeof(blob), file);
    CHECK(!ferror(file) && feof(file));
    fclose(file);
    controller_profile_runtime_init();
    CHECK(!controller_profile_runtime_activate(blob, length, 0x17cc, 0x9999));
    CHECK(controller_profile_runtime_activate(blob, length, 0x17cc, 0x1710));
    for (unsigned deck = 0; deck < 2; ++deck) {
        uint8_t status = (uint8_t)(0x91 + 2 * deck), type, id;
        int16_t value;
        CHECK(controller_profile_runtime_map(status, 0x13, 0x7f, &type, &id, &value));
        CHECK(type == CTRL_TYPE_BUTTON && id == (deck ? CTRL_ID_LOAD_DECK2 : CTRL_ID_LOAD_DECK1) && value == 1);
        CHECK(controller_profile_runtime_map(status, 0x13, 0, &type, &id, &value) && value == 0);
        CHECK(controller_profile_runtime_map(status, 0x18, 0x7f, &type, &id, &value));
        CHECK(type == CTRL_TYPE_BUTTON && id == CTRL_ID_BROWSE_PRESS && value == 1);
        CHECK(controller_profile_runtime_map(status, 0x18, 0, &type, &id, &value) && value == 0);
        CHECK(!controller_profile_runtime_map(status, 0x29, 0x7f, &type, &id, &value));
        CHECK(!controller_profile_runtime_map(status, 0x29, 0, &type, &id, &value));
        CHECK(controller_profile_runtime_map(status + 0x20, 0x19, 0x41, &type, &id, &value));
        CHECK(type == CTRL_TYPE_ENCODER && id == CTRL_ID_BROWSE_DELTA && value == 1);
        CHECK(controller_profile_runtime_map(status + 0x20, 0x19, 0x3f, &type, &id, &value));
        CHECK(type == CTRL_TYPE_ENCODER && id == CTRL_ID_BROWSE_DELTA && value == -1);
        CHECK(controller_profile_runtime_map(status, 1, 0x7f, &type, &id, &value));
        CHECK(type == CTRL_TYPE_BUTTON && id == (deck ? CTRL_ID_DECK2_PLAY : CTRL_ID_DECK1_PLAY) && value == 1);
    }
    puts("S2 compiled-profile navigation: PASS (both decks, edges, direction, contact ignored, VID/PID)");
    return 0;
}
