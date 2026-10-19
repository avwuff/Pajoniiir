/* Focused AS fixture derived from Kayrosen's DDJ-400 lsusb capture at
 * 00041a0d, docs/validation/jc1060/evidence/C-ddj400/ddj400_lsusb_v.txt.
 * Contains the two captured playback alternates, not the MIDI/control tree. */
static const uint8_t ddj400_hs_as_config[] = {
    9,2,95,0,1,1,0,0x80,250,
    9,4,1,1,1,1,2,0,4,
    7,0x24,1,1,1,1,0,
    11,0x24,2,1,4,2,16,1,0x44,0xac,0,
    9,5,1,13,0x40,2,4,0,0,
    7,0x25,1,1,0,0,0,
    9,4,1,2,1,1,2,0,4,
    7,0x24,1,1,1,1,0,
    11,0x24,2,1,4,3,24,1,0x44,0xac,0,
    9,5,1,13,0x40,2,4,0,0,
    7,0x25,1,1,0,0,0,
};
