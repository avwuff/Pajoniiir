/* Focused AudioStreaming fixture reconstructed from the Numark DJ2GO2 Touch
 * lsusb summary retained in donor 86d6b412's profile README: FS UAC1, four
 * PCM16 channels, synchronous OUT, 384-byte maximum packet, 44100 Hz only.
 * This is a minimal parser/stream contract, not a raw device/MIDI capture. */
static const uint8_t numark_touch_fs_as_config[] = {
    9,2,52,0,1,1,0,0x80,50,
    9,4,1,1,1,1,2,0,0,
    7,0x24,1,1,1,1,0,
    11,0x24,2,1,4,2,16,1,0x44,0xac,0,
    9,5,1,13,0x80,1,1,0,0,
    7,0x25,1,0,0,0,0,
};
