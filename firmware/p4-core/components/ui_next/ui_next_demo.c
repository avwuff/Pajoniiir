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
    m->lift_row = -1;
    m->ins_row = -1;
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

void ui_next_demo_source(ui_next_source_model_t *m, const ui_next_env_t *env2)
{
    static const struct { const char *n, *st; uint32_t c; ui_next_src_icon_t i; bool ok; uint8_t pct; } SRC[UI_NEXT_SOURCES] = {
        {"USB 1", "CONNECTED", 0xFFC15A, UI_NEXT_SRC_USB, true, 14},
        {"USB 2", "EMPTY", 0x3FD6C0, UI_NEXT_SRC_USB, false, 0},
        {"SD CARD", "CONNECTED", 0xB07CFF, UI_NEXT_SRC_SD, true, 52},
        {"NETWORK", "LINKED", 0x4DABF7, UI_NEXT_SRC_NET, true, 0},
        {"PHONE", "NOT PAIRED", 0xFF6B6B, UI_NEXT_SRC_PHONE, false, 0},
    };
    static const char *const V[6] = {"2 PLAYERS", "ETH", "192.168.1.21", "PAJONIIIR", "ON", "PLAYER 2"};
    for (int i = 0; i < UI_NEXT_SOURCES; i++) {
        ui_next_source_t *s = &m->source[i];
        snprintf(s->name, sizeof s->name, "%s", SRC[i].n);
        snprintf(s->status, sizeof s->status, "%s", SRC[i].st);
        s->color = SRC[i].c;
        s->icon = SRC[i].i;
        s->ready = SRC[i].ok;
        s->used_pct = SRC[i].pct;
    }
    m->selected = 3;
    snprintf(m->summary, sizeof m->summary, "2 DRIVES READY");
    snprintf(m->panel_title, sizeof m->panel_title, "NETWORK");
    snprintf(m->panel_state, sizeof m->panel_state, "DJ LINK ON");
    for (int i = 0; i < 6; i++) snprintf(m->value[i], sizeof m->value[i], "%s", V[i]);
    m->player = 1;
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env2 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}

void ui_next_demo_browse_edit(ui_next_browse_model_t *m, const ui_next_env_t *env4)
{
    ui_next_demo_browse(m, env4);
    snprintf(m->count, sizeof m->count, "96 TRACKS");
    snprintf(m->match, sizeof m->match, "EDIT: DRAG A TRACK TO A NEW POSITION");
    m->open_row = -1;
    m->lift_row = 5; /* track 6 is under the finger */
    m->ins_row = 2;  /* and would land above track 3 */
}

void ui_next_demo_fx(ui_next_fx_model_t *m, const ui_next_env_t *env2)
{
    static const struct { const char *n, *t; bool on; uint8_t a; } SL[UI_NEXT_FX_SLOTS] = {
        {"ECHO", "40 %", true, 40}, {"REVERB", "25 %", false, 25}, {"FILTER", "HP 65 %", true, 65},
    };
    for (int i = 0; i < UI_NEXT_FX_SLOTS; i++) {
        snprintf(m->slot[i].name, sizeof m->slot[i].name, "%s", SL[i].n);
        snprintf(m->slot[i].text, sizeof m->slot[i].text, "%s", SL[i].t);
        m->slot[i].on = SL[i].on;
        m->slot[i].amount = SL[i].a;
    }
    m->xy_x = 0.64f;
    m->xy_y = 0.36f;
    snprintf(m->label_x, sizeof m->label_x, "ECHO WET");
    snprintf(m->label_y, sizeof m->label_y, "FILTER");
    snprintf(m->panel_title, sizeof m->panel_title, "ECHO  /  DECK A");
    m->time_beats = 1;
    m->wet = 40;
    m->feedback = 55;
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env2 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}

/* Running waveform of the Record board, 240 columns of half heights. */
static const uint8_t REC_LOW[UI_NEXT_REC_COLS] = {
    7, 7, 6, 26, 19, 15, 10, 9, 8, 7, 7, 6, 29, 18, 13, 11, 8, 8, 8, 6, 7, 27, 20, 15,
    11, 9, 7, 7, 7, 6, 25, 20, 14, 11, 9, 8, 7, 7, 6, 27, 20, 13, 11, 10, 8, 7, 7, 6,
    26, 18, 13, 11, 9, 8, 7, 7, 6, 26, 19, 14, 11, 10, 8, 7, 7, 7, 25, 20, 13, 11, 9, 8,
    7, 6, 6, 29, 18, 15, 11, 9, 8, 8, 7, 6, 27, 18, 15, 11, 9, 8, 7, 7, 7, 27, 20, 13,
    10, 9, 8, 7, 6, 6, 27, 18, 14, 11, 9, 7, 8, 7, 6, 28, 19, 14, 11, 10, 8, 7, 7, 6,
    93, 70, 53, 40, 33, 26, 26, 24, 24, 98, 70, 51, 41, 30, 27, 25, 23, 23, 98, 69, 50, 39, 31, 29,
    24, 26, 24, 98, 65, 53, 38, 33, 28, 27, 23, 23, 98, 64, 48, 38, 29, 30, 26, 24, 24, 98, 68, 52,
    36, 32, 26, 26, 24, 23, 98, 67, 48, 39, 30, 29, 26, 24, 22, 98, 64, 46, 40, 34, 29, 25, 22, 24,
    98, 72, 47, 41, 31, 27, 24, 25, 22, 95, 67, 52, 38, 30, 26, 25, 22, 21, 92, 73, 53, 37, 32, 30,
    26, 23, 24, 98, 70, 53, 41, 31, 29, 25, 25, 24, 94, 70, 53, 38, 32, 27, 27, 23, 22, 98, 72, 49,
};
static const uint8_t REC_MID[UI_NEXT_REC_COLS] = {
    5, 20, 26, 35, 28, 20, 26, 24, 23, 22, 22, 19, 23, 21, 23, 22, 22, 24, 26, 23, 20, 30, 26, 25,
    28, 25, 29, 23, 19, 22, 31, 21, 20, 27, 19, 29, 24, 23, 24, 33, 22, 28, 22, 24, 24, 28, 22, 25,
    26, 22, 20, 23, 27, 33, 24, 24, 22, 27, 22, 24, 25, 26, 32, 28, 24, 23, 28, 28, 25, 23, 27, 27,
    25, 25, 28, 31, 23, 23, 26, 19, 30, 22, 24, 19, 34, 31, 26, 22, 25, 26, 24, 22, 21, 28, 28, 25,
    21, 27, 23, 23, 26, 24, 31, 26, 27, 19, 23, 26, 21, 26, 20, 30, 28, 22, 21, 27, 26, 23, 18, 27,
    29, 17, 20, 14, 11, 26, 15, 18, 13, 47, 23, 14, 18, 17, 34, 17, 16, 11, 33, 25, 19, 13, 15, 27,
    18, 19, 15, 34, 22, 20, 11, 17, 27, 18, 15, 11, 35, 27, 16, 14, 11, 29, 16, 17, 11, 45, 21, 16,
    11, 13, 21, 19, 14, 13, 35, 17, 17, 14, 15, 33, 21, 16, 18, 29, 21, 18, 16, 16, 21, 21, 16, 11,
    34, 18, 18, 14, 14, 31, 22, 18, 15, 42, 17, 15, 17, 13, 33, 16, 18, 15, 39, 23, 13, 13, 13, 31,
    17, 17, 16, 32, 27, 14, 13, 14, 28, 17, 12, 12, 40, 23, 16, 14, 13, 25, 21, 14, 16, 35, 25, 17,
};
static const uint8_t REC_HIGH[UI_NEXT_REC_COLS] = {
    5, 2, 2, 1, 1, 1, 1, 1, 5, 5, 3, 3, 2, 1, 1, 1, 1, 7, 5, 3, 2, 1, 2, 1,
    1, 1, 7, 4, 3, 3, 1, 1, 2, 1, 2, 9, 4, 3, 2, 2, 2, 2, 1, 2, 5, 5, 3, 1,
    2, 2, 1, 1, 1, 5, 5, 3, 2, 1, 2, 2, 2, 2, 6, 3, 3, 3, 2, 2, 2, 2, 1, 6,
    5, 3, 2, 2, 1, 1, 1, 1, 8, 3, 3, 2, 1, 2, 1, 1, 1, 9, 4, 2, 2, 1, 1, 1,
    1, 1, 7, 3, 3, 2, 2, 1, 1, 1, 1, 5, 3, 3, 2, 2, 2, 2, 1, 2, 5, 3, 2, 1,
    5, 6, 5, 4, 4, 30, 15, 10, 10, 8, 7, 5, 6, 6, 27, 16, 8, 5, 7, 5, 4, 5, 7, 30,
    14, 8, 9, 4, 7, 4, 4, 4, 17, 16, 10, 7, 4, 6, 6, 6, 4, 25, 12, 8, 9, 7, 6, 5,
    5, 4, 21, 17, 12, 7, 5, 7, 6, 6, 4, 29, 19, 9, 5, 7, 4, 5, 6, 6, 33, 19, 8, 8,
    7, 7, 6, 6, 4, 25, 14, 12, 9, 6, 7, 4, 6, 5, 28, 10, 9, 6, 5, 5, 6, 5, 6, 26,
    15, 10, 8, 7, 6, 5, 5, 6, 29, 14, 12, 6, 8, 7, 5, 7, 4, 21, 13, 10, 6, 6, 6, 5,
};

void ui_next_demo_record(ui_next_record_model_t *m, const ui_next_env_t *env2)
{
    snprintf(m->timer, sizeof m->timer, "00:12:34");
    m->wave[0] = REC_LOW;
    m->wave[1] = REC_MID;
    m->wave[2] = REC_HIGH;
    m->head01 = 0.88f;
    m->level[0] = 0.68f; m->peak[0] = 0.69f;
    m->level[1] = 0.62f; m->peak[1] = 0.64f;
    snprintf(m->db[0], sizeof m->db[0], "-8 dB");
    snprintf(m->db[1], sizeof m->db[1], "-10 dB");
    snprintf(m->status, sizeof m->status, "RECORDING");
    snprintf(m->free_space, sizeof m->free_space, "FREE 38 GB");
    m->recording = true;
    static const char *const V[4] = {"MASTER", "WAV 24", "48 kHz", "124 MB"};
    for (int i = 0; i < 4; i++) snprintf(m->value[i], sizeof m->value[i], "%s", V[i]);
    static const char *const FN[3] = {"set_0005.wav", "set_0004.wav", "set_0003.wav"};
    static const char *const FS[3] = {"842 MB", "1.1 GB", "506 MB"};
    for (int i = 0; i < 3; i++) {
        snprintf(m->file[i].name, sizeof m->file[i].name, "%s", FN[i]);
        snprintf(m->file[i].size, sizeof m->file[i].size, "%s", FS[i]);
    }
    for (int d = 0; d < 2; d++) {
        m->deck[d].env = env2 + d;
        snprintf(m->deck[d].title, sizeof m->deck[d].title, d ? "Sample Track B" : "Sample Track A");
        snprintf(m->deck[d].time, sizeof m->deck[d].time, d ? "-01:56" : "-03:44");
        m->deck[d].pos01 = d ? 0.61f : 0.28f;
    }
}
