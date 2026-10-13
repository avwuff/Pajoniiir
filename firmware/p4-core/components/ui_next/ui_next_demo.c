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

void ui_next_demo_boot(ui_next_boot_model_t *m)
{
    m->progress01 = 0.62f;
    snprintf(m->status, sizeof m->status, "Reading library: 96 tracks");
    snprintf(m->device_name, sizeof m->device_name, "PLACEHOLDER NAME");
    snprintf(m->footer, sizeof m->footer, "v0.1.0  /  CODE APACHE-2.0  /  DESIGN CC BY 4.0");
}

/* The 15 rows and the tag set of the Browse board. */
static const ui_next_track_t DEMO_TRACKS[UI_NEXT_BROWSE_TRACKS] = {
    {"Sample Track 01", "Sample Artist 01", "Deep House", "122.0", "9A", "05:12", UI_NEXT_KEY_FIT, {0, 0, 0}, 1, 0x7B3232, 0x3A3418, 0},
    {"Sample Track 02", "Sample Artist 02", "Tech House", "122.0", "8A", "06:40", UI_NEXT_KEY_FIT, {0, 2, 0}, 2, 0x7B6B32, 0x253A18, 1},
    {"Sample Track 03", "Sample Artist 03", "Techno", "123.0", "10A", "04:58", UI_NEXT_KEY_FIT, {0, 3, 0}, 2, 0x527B32, 0x183A25, 2},
    {"Sample Track 04", "Sample Artist 04", "Deep House", "123.0", "11A", "07:05", UI_NEXT_KEY_NEAR, {2, 0, 0}, 1, 0x327B4C, 0x18343A, 3},
    {"Sample Track 05", "Sample Artist 05", "Melodic", "124.0", "4A", "05:31", UI_NEXT_KEY_FAR, {0, 3, 0}, 2, 0x32717B, 0x18193A, 0},
    {"Sample Track 06", "Sample Artist 06", "Tech House", "124.0", "9B", "06:02", UI_NEXT_KEY_FIT, {2, 1, 0}, 2, 0x32387B, 0x31183A, 1},
    {"Sample Track 07", "Sample Artist 07", "Techno", "124.0", "3B", "05:47", UI_NEXT_KEY_NONE, {0, 0, 0}, 1, 0x65327B, 0x3A1828, 2},
    {"Sample Track 08", "Sample Artist 08", "House", "124.0", "10A", "04:21", UI_NEXT_KEY_FIT, {0, 2, 0}, 2, 0x7B3258, 0x3A2318, 3},
    {"Sample Track 09", "Sample Artist 09", "Deep House", "125.0", "5A", "06:18", UI_NEXT_KEY_NONE, {2, 3, 0}, 2, 0x7B4632, 0x373A18, 0},
    {"Sample Track 10", "Sample Artist 01", "Melodic", "125.0", "11A", "05:03", UI_NEXT_KEY_NEAR, {0, 0, 0}, 1, 0x777B32, 0x1C3A18, 1},
    {"Sample Track 11", "Sample Artist 02", "House", "126.0", "8A", "06:55", UI_NEXT_KEY_FIT, {2, 4, 0}, 2, 0x3E7B32, 0x183A2F, 2},
    {"Sample Track 12", "Sample Artist 03", "Techno", "126.0", "4A", "05:40", UI_NEXT_KEY_FAR, {0, 3, 0}, 2, 0x327B5F, 0x182B3A, 3},
    {"Sample Track 13", "Sample Artist 04", "Tech House", "127.0", "2B", "06:11", UI_NEXT_KEY_NONE, {0, 2, 0}, 2, 0x325E7B, 0x20183A, 0},
    {"Sample Track 14", "Sample Artist 05", "House", "127.0", "9A", "05:26", UI_NEXT_KEY_FIT, {2, 0, 0}, 1, 0x40327B, 0x3A1839, 1},
    {"Sample Track 15", "Sample Artist 06", "Deep House", "128.0", "7A", "07:32", UI_NEXT_KEY_NONE, {0, 0, 0}, 1, 0x79327B, 0x3A181F, 2},
};

void ui_next_demo_browse(ui_next_browse_model_t *m, const ui_next_env_t *env4)
{
    static const struct { const char *name; uint32_t color; bool on; } TAGS[UI_NEXT_BROWSE_TAGS] = {
        {"WARM-UP", 0xB07CFF, true}, {"PEAK", 0xFF6B6B, false}, {"VOCAL", 0xFF8AD0, true},
        {"DARK", 0x6FB1FF, false},   {"CLOSING", 0x5FD38D, false},
    };
    snprintf(m->crumb, sizeof m->crumb, "Playlists / Warm-up");
    snprintf(m->count, sizeof m->count, "38 OF 96");
    snprintf(m->match, sizeof m->match, "MATCH TO DECK B 9A");
    m->category = 5; /* PLAYLIST */
    m->tag_badge = 2;
    for (int i = 0; i < UI_NEXT_BROWSE_TAGS; i++) {
        snprintf(m->tags[i].name, sizeof m->tags[i].name, "%s", TAGS[i].name);
        m->tags[i].color = TAGS[i].color;
        m->tags[i].on = TAGS[i].on;
    }
    for (int i = 0; i < UI_NEXT_BROWSE_TRACKS; i++) m->track[i] = DEMO_TRACKS[i];
    m->open_row = 2;
    for (int i = 0; i < 4; i++) m->preview_env[i] = env4 + i;
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env4 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}

void ui_next_demo_search(ui_next_search_model_t *m, const ui_next_env_t *env2)
{
    static const struct { const char *pre, *hit, *post, *artist, *bpm, *key; uint32_t a0, a1; } R[UI_NEXT_SEARCH_RESULTS] = {
        {"", "\xC3\x89t\xC3\xA9", " Indien", "Sample Artist 01", "124", "8A", 0xB07CFF, 0x3FD6C0},
        {"Nuit d'", "\xC3\xA9t\xC3\xA9", "", "Sample Artist 04", "122", "5A", 0xFFC15A, 0xFF6B6B},
        {"F", "\xC3\xAAte", " Foraine", "Sample Artist 02", "126", "8A", 0x4DABF7, 0xB07CFF},
        {"Po", "\xC3\xA8te", " Urbain", "Sample Artist 07", "128", "9A", 0x3FD6C0, 0xF4F1E8},
        {"Beaut\xC3\xA9 Secr", "\xC3\xA8te", "", "Sample Artist 03", "120", "4A", 0xB07CFF, 0x3FD6C0},
        {"Temp", "\xC3\xAAte", " d'Hiver", "Sample Artist 09", "118", "3B", 0xFFC15A, 0xFF6B6B},
        {"Compl", "ete", " Control", "Sample Artist 05", "130", "10A", 0x4DABF7, 0xB07CFF},
        {"Ar", "\xC3\xAAte", " Blanche", "Sample Artist 06", "123", "9B", 0x3FD6C0, 0xF4F1E8},
    };
    snprintf(m->query, sizeof m->query, "ete");
    snprintf(m->status, sizeof m->status, "ACCENTS IGNORED / 8 RESULTS");
    m->n = UI_NEXT_SEARCH_RESULTS;
    m->selected = 1;
    for (int i = 0; i < UI_NEXT_SEARCH_RESULTS; i++) {
        ui_next_search_result_t *r = &m->result[i];
        snprintf(r->pre, sizeof r->pre, "%s", R[i].pre);
        snprintf(r->hit, sizeof r->hit, "%s", R[i].hit);
        snprintf(r->post, sizeof r->post, "%s", R[i].post);
        snprintf(r->artist, sizeof r->artist, "%s", R[i].artist);
        snprintf(r->bpm, sizeof r->bpm, "%s", R[i].bpm);
        snprintf(r->key, sizeof r->key, "%s", R[i].key);
        r->art0 = R[i].a0;
        r->art1 = R[i].a1;
    }
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env2 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}

void ui_next_demo_alerts(ui_next_alerts_model_t *m)
{
    static const struct { ui_next_toast_kind_t k; const char *t, *x; } T[UI_NEXT_TOASTS] = {
        {UI_NEXT_TOAST_DONE, "Recording saved", "set_0005.wav, 842 MB"},
        {UI_NEXT_TOAST_CARE, "Disk space low", "Less than 1 GB left on USB 2"},
        {UI_NEXT_TOAST_FAULT, "Controller lost", "Check the USB cable"},
    };
    m->n_toasts = UI_NEXT_TOASTS;
    for (int i = 0; i < UI_NEXT_TOASTS; i++) {
        m->toast[i].kind = T[i].k;
        snprintf(m->toast[i].title, sizeof m->toast[i].title, "%s", T[i].t);
        snprintf(m->toast[i].text, sizeof m->toast[i].text, "%s", T[i].x);
    }
    m->modal = true;
    snprintf(m->title, sizeof m->title, "USB 1 was removed");
    snprintf(m->body, sizeof m->body,
             "Deck A uses a track from this drive. The track stays in memory until it ends. "
             "Put the drive back to load more tracks.");
    snprintf(m->primary, sizeof m->primary, "KEEP PLAYING");
    snprintf(m->secondary, sizeof m->secondary, "OPEN SOURCE");
}

void ui_next_demo_trackinfo(ui_next_trackinfo_model_t *m, const ui_next_env_t *env3)
{
    static const char *const ST[6] = {"124.0", "A min", "06:40", "320 k", "12", "OK"};
    static const char *const CT[UI_NEXT_TI_CUES] = {"00:00", "00:32", "01:04", "01:36", "02:40", "03:12", "04:16", "05:20"};
    static const uint32_t CC[UI_NEXT_TI_CUES] = {0xFF6B6B, 0xFFA94D, 0xFFD43B, 0x69DB7C, 0x38D9A9, 0x4DABF7, 0x9775FA, 0xF783AC};
    static const struct { const char *n; uint8_t w; uint32_t c; } PH[UI_NEXT_TI_PHRASES] = {
        {"INTRO", 8, 0x4A5568}, {"BUILD", 18, 0x1F7A6F}, {"DROP", 24, 0x6B4AB8},
        {"BREAK", 10, 0x4A5568}, {"DROP", 30, 0x6B4AB8}, {"OUT", 10, 0x4A5568},
    };
    snprintf(m->title, sizeof m->title, "Sample Track 02");
    snprintf(m->artist, sizeof m->artist, "Sample Artist");
    snprintf(m->album, sizeof m->album, "Sample Album, 2026");
    snprintf(m->genre, sizeof m->genre, "House");
    snprintf(m->label, sizeof m->label, "Sample Label");
    m->art0 = 0x527B32;
    m->art1 = 0x183A25;
    for (int i = 0; i < 6; i++) snprintf(m->stat[i], sizeof m->stat[i], "%s", ST[i]);
    for (int i = 0; i < UI_NEXT_TI_CUES; i++) {
        snprintf(m->cue[i].time, sizeof m->cue[i].time, "%s", CT[i]);
        m->cue[i].color = CC[i];
    }
    for (int i = 0; i < UI_NEXT_TI_PHRASES; i++) {
        snprintf(m->phrase[i].name, sizeof m->phrase[i].name, "%s", PH[i].n);
        m->phrase[i].weight = PH[i].w;
        m->phrase[i].color = PH[i].c;
    }
    snprintf(m->preview, sizeof m->preview, "PREVIEW 02:44 OF 06:40");
    m->play01 = 0.41f;
    m->env = env3;
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env3 + 1 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}
