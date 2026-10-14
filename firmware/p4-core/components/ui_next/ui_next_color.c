#include "ui_next_color.h"

bool ui_next_env_from_pwv4(const uint8_t *p, uint32_t len, ui_next_env_t *out)
{
    const uint32_t n = p ? len / UI_NEXT_PWV4_ENTRY : 0u;
    if (!out || n == 0u) return false;

    uint32_t peak = 1u;
    for (uint32_t i = 0; i < n; i++)
        if (p[i * UI_NEXT_PWV4_ENTRY] > peak) peak = p[i * UI_NEXT_PWV4_ENTRY];

    for (uint32_t c = 0; c < UI_NEXT_ENV_COLUMNS; c++) {
        uint32_t from = (uint32_t)((uint64_t)c * n / UI_NEXT_ENV_COLUMNS);
        uint32_t to = (uint32_t)((uint64_t)(c + 1u) * n / UI_NEXT_ENV_COLUMNS);
        if (to <= from) to = from + 1u;
        if (to > n) to = n;
        uint32_t h = 0, b[3] = {0, 0, 0};
        for (uint32_t i = from; i < to; i++) {
            const uint8_t *e = &p[i * UI_NEXT_PWV4_ENTRY];
            if (e[0] > h) h = e[0];
            for (int k = 0; k < 3; k++)
                if (e[3 + k] > b[k]) b[k] = e[3 + k];
        }
        uint32_t top = b[0] > b[1] ? b[0] : b[1];
        if (b[2] > top) top = b[2];
        const float hn = (float)h / (float)peak;
        out->lo[c] = top ? hn * (float)b[0] / (float)top : 0.0f;
        out->mi[c] = top ? hn * (float)b[1] / (float)top : 0.0f;
        out->hi[c] = top ? hn * (float)b[2] / (float)top : 0.0f;
    }
    return true;
}
