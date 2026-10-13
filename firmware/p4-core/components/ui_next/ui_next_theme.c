#include "ui_next_theme.h"

static const ui_next_palette_t NIGHT = {
    .frame = 0x05070A, .panel = 0x161B22, .nav = 0x0F141B, .sel = 0x1B2430, .well = 0x0B0F14,
    .border = 0x2A313C, .border_hi = 0x3A4350, .chip = 0x222A35,
    .text = 0xE8ECF1, .muted = 0x9AA4B2, .on_accent = 0x05070A,
    .accent = 0xFFC15A, .accent_ink = 0xFFC15A,
    .ok = 0x5FD38D, .warn = 0xFFC15A, .bad = 0xFF6B6B,
    .wave_low = 0xB07CFF, .wave_mid = 0x3FD6C0, .wave_high = 0xF4F1E8,
};

static const ui_next_palette_t DAY = {
    .frame = 0xFFFFFF /* derived */, .panel = 0xFFFFFF, .nav = 0xF7F9FB, .sel = 0xDDE6F2, .well = 0xFAFBFC,
    .border = 0xCBD3DD /* derived */, .border_hi = 0x8A94A3, .chip = 0xE3E8EE,
    .text = 0x14181F, .muted = 0x5B6573, .on_accent = 0x14181F,
    .accent = 0xFFC15A, .accent_ink = 0xB26B00,
    .ok = 0x5FD38D, .warn = 0xFFC15A, .bad = 0xFF6B6B,
    .wave_low = 0x7B3FE4, .wave_mid = 0x0FA392, .wave_high = 0x2B3340,
};

const ui_next_palette_t *ui_next_palette(bool day)
{
    return day ? &DAY : &NIGHT;
}

ui_next_sev_t ui_next_severity(float err01)
{
    if (err01 < 0.15f) return UI_NEXT_SEV_OK;
    if (err01 < 0.5f) return UI_NEXT_SEV_WARN;
    return UI_NEXT_SEV_BAD;
}
