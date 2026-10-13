#include <stdio.h>

#include "ui_next.h"

/* Demo data for the boards. Everything here is a CORE-GAP stub: the real
 * values will come from the audio engine and the analysis. */
void ui_next_demo_perform(ui_next_perform_model_t *m, const ui_next_env_t *ea,
                          const ui_next_env_t *eb, uint32_t t, ui_next_class_t cls)
{
    static const uint16_t WIN[] = {144, 180, 216};
    const uint32_t shift = t % 180u;
    const uint32_t sb = (uint32_t)(shift * 0.96f);
    const uint32_t mcols = WIN[cls];
    const uint32_t sa = 800u;
    uint32_t sbase = (uint32_t)(312 - (int)mcols / 2);
    while ((sbase - sa) % UI_NEXT_BEAT_COLUMNS != 0u) sbase++;
    const uint32_t a = sa + shift, b = sbase + sb;

    m->deck[0].env = ea;
    m->deck[1].env = eb;
    m->deck[0].col = a + mcols / 2u;
    m->deck[1].col = b + mcols / 2u;
    snprintf(m->deck[0].title, sizeof m->deck[0].title, "Sample Track A");
    snprintf(m->deck[1].title, sizeof m->deck[1].title, "Sample Track B");
    snprintf(m->deck[0].bpm, sizeof m->deck[0].bpm, "124.0");
    snprintf(m->deck[1].bpm, sizeof m->deck[1].bpm, "119.0");
    snprintf(m->deck[0].key, sizeof m->deck[0].key, "8A");
    snprintf(m->deck[1].key, sizeof m->deck[1].key, "9A");
    snprintf(m->deck[0].badge, sizeof m->deck[0].badge, "MASTER");
    snprintf(m->deck[1].badge, sizeof m->deck[1].badge, "SYNC OFF");
    snprintf(m->deck[0].time, sizeof m->deck[0].time, "-03:44");
    snprintf(m->deck[1].time, sizeof m->deck[1].time, "-01:56");
    m->deck[0].badge_on = true;
    m->deck[1].badge_on = false;
    m->deck[0].key_fit = false;
    m->deck[1].key_fit = ui_next_key_fit(8, 'A', 9, 'A');
    m->deck[0].pos01 = 0.667f + (float)shift / 12.0f / 100.0f;
    m->deck[1].pos01 = 0.26f + (float)sb / 12.0f / 100.0f;

    for (int d = 0; d < 2; d++) {
        const ui_next_env_t *e = d ? eb : ea;
        int32_t ix = (int32_t)m->deck[d].col;
        float lev = .55f * ui_next_env_smooth(e->lo, UI_NEXT_ENV_COLUMNS, ix, 1) +
                    .45f * ui_next_env_smooth(e->mi, UI_NEXT_ENV_COLUMNS, ix, 1) +
                    .25f * ui_next_env_smooth(e->hi, UI_NEXT_ENV_COLUMNS, ix, 1);
        if (lev > 1.0f) lev = 1.0f;
        float pk = 0.0f;
        for (int32_t i = ix - 14; i <= ix; i++) {
            float l = ui_next_env_smooth(e->lo, UI_NEXT_ENV_COLUMNS, i, 1) * .55f +
                      ui_next_env_smooth(e->mi, UI_NEXT_ENV_COLUMNS, i, 1) * .45f;
            if (l > pk) pk = l;
        }
        m->deck[d].vu_l = lev;
        m->deck[d].vu_r = lev * .95f;
        m->deck[d].peak_l = pk > .99f ? .99f : pk;
        m->deck[d].peak_r = m->deck[d].peak_l * .95f;
    }

    ui_next_link_compute(ea, a, eb, b, mcols, &m->link);
    m->link.key_fit = m->deck[1].key_fit;
    m->tempo_pct = -5.0f;
    snprintf(m->key_change, sizeof m->key_change, "8A>9A");
    m->layers = cls == UI_NEXT_CLASS_S ? "LAYERS 1-4" : cls == UI_NEXT_CLASS_M ? "LAYERS 1-6" : "LAYERS 1-7";
}
