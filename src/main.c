#include "../raylib/include/raylib.h"
#include "engine.h"
#include "math.h"

#define DLL_EXPORT __attribute__((visibility("default")))

#define BG_COLOR                            ((Color){32,32,32,255})
#define BG_COLOR_INACTIVE                   ((Color){0,64,128,255})
#define BG_COLOR_ACTIVE                     ((Color){0,128,255,255})
#define BG_COLOR_MUTE                       ((Color){0,128,0,255})
#define BG_COLOR_UNMUTE                     ((Color){128,0,0,255})

#define TEXT_COLOR_INACTIVE                 ((Color){128,128,128,255})
#define TEXT_COLOR_ACTIVE                   ((Color){255,255,255,255})
#define TEXT_COLOR_MUTE                     ((Color){0,255,0,255})
#define TEXT_COLOR_UNMUTE                   ((Color){255,0,0,255})

#define COLOR_FILLED_ACTIVE                 ((Color){0,128,255,255})
#define COLOR_UNFILLED_ACTIVE               ((Color){0,64,128,255})
#define COLOR_FILLED_INACTIVE               ((Color){0,64,128,255})
#define COLOR_UNFILLED_INACTIVE             ((Color){0,32,64,255})
#define COLOR_FILLED_PROGRESS_BAR           ((Color){0,255,0,255})
#define COLOR_UNFILLED_PROGRESS_BAR         ((Color){0,128,0,255})

#define FONT_SIZE 50

#define RECTANGLE_LINES_SIZE 5
#define RECTANGLE_LINES_COLOR ((Color){0,0,0,255})

#define TIME_PER_CHORD_MAX_MULTIPLIER 2.0f
#define VIBES_PER_CHORD_MIN 1
#define VIBES_PER_CHORD_MAX 16

#define CHAR_SHARP              0x266F  // ♯
#define CHAR_DOUBLE_SHARP       0x1D12A // 𝄪
#define CHAR_FLAT               0x266D  // ♭
#define CHAR_DOUBLE_FLAT        0x1D12B // 𝄫
#define CHAR_NATURAL            0x266E  // ♮
#define CHAR_DIMINISHED         0x00B0  // °
#define CHAR_HALF_DIMINISHED    0x00F8  // ø
#define CHAR_NOTE               0x266A  // ♪

#define INTERVAL_MAJOR_THIRD 4
#define INTERVAL_MINOR_THIRD 3
#define INTERVAL_FIFTH 7
#define INTERVAL_FLAT_FIFTH 6
#define INTERVAL_AUGMENTED_FIFTH 8

#define SCALE_MAJOR             ((Scale){0, 2, 4, 5, 7, 9, 11})
#define SCALE_DORIAN            ((Scale){0, 2, 3, 5, 7, 9, 10})
#define SCALE_PHRYGIAN          ((Scale){0, 1, 3, 5, 7, 8, 10})
#define SCALE_LYDIAN            ((Scale){0, 2, 4, 6, 7, 9, 11})
#define SCALE_MIXOLYDIAN        ((Scale){0, 2, 4, 5, 7, 9, 10})
#define SCALE_MINOR             ((Scale){0, 2, 3, 5, 7, 8, 10})
#define SCALE_LOCRIAN           ((Scale){0, 1, 3, 5, 6, 8, 10})
#define SCALE_HARMONIC_MINOR    ((Scale){0, 2, 3, 5, 7, 8, 11})
#define SCALE_MELODIC_MINOR     ((Scale){0, 2, 3, 5, 7, 9, 11})

#define SELECTABLES_BOX_WIDTH_MULTIPLIER 0.5f
#define SELECTABLE_ITEM_BG_COLOR_ODD ((Color){48,48,48,255})
#define SELECTABLE_ITEM_BG_COLOR_EVEN ((Color){64,64,64,255})
#define SELECTABLE_ITEM_BG_COLOR_SELECTED ((Color){0,0,255,255})

enum {
    STATE_MAIN,
    STATE_SELECT,
};

enum {
    VERTICAL_POSITION_OF_AUDIO_CONTROLS = 0,
    VERTICAL_POSITION_OF_SEQUENCER = 1, // needs 3 slots
    VERTICAL_POSITION_OF_SEQUENCER_B = 4, // needs 3 slots
    VERTICAL_POSITION_OF_PROGRESS_BAR = 7,
    VERTICAL_POSITION_OF_SCALE_BUTTONS = 8,
    VERTICAL_POSITION_OF_TIMING_CONTROLS = 9,
    VERTICAL_POSITION_COUNT,
};

enum {
    FLAG_AUDIO = (1 << 0),
    FLAG_FLATS = (1 << 1),
};

enum {
    SCALE_DEGREE_I,
    SCALE_DEGREE_II,
    SCALE_DEGREE_III,
    SCALE_DEGREE_IV,
    SCALE_DEGREE_V,
    SCALE_DEGREE_VI,
    SCALE_DEGREE_VII,
    SCALE_DEGREE_NONE,
};

enum {
    NOTE_A,
    NOTE_A_SHARP,
    NOTE_B_FLAT = NOTE_A_SHARP,
    NOTE_B,
    NOTE_C,
    NOTE_C_SHARP,
    NOTE_D_FLAT = NOTE_C_SHARP,
    NOTE_D,
    NOTE_D_SHARP,
    NOTE_E_FLAT = NOTE_D_SHARP,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP,
    NOTE_G_FLAT = NOTE_F_SHARP,
    NOTE_G,
    NOTE_G_SHARP,
    NOTE_A_FLAT = NOTE_G_SHARP,
    NOTE_COUNT,
};

enum {
    CHORD_TYPE_NONE,
    CHORD_TYPE_MAJOR,
    CHORD_TYPE_MINOR,
    CHORD_TYPE_DIMINISHED,
    CHORD_TYPE_AUGMENTED,
};

enum {
    SCALE_TYPE_MAJOR,
    SCALE_TYPE_DORIAN,
    SCALE_TYPE_PHRYGIAN,
    SCALE_TYPE_LYDIAN,
    SCALE_TYPE_MIXOLYDIAN,
    SCALE_TYPE_MINOR,
    SCALE_TYPE_LOCRIAN,
    SCALE_TYPE_HARMONIC_MINOR,
    SCALE_TYPE_MELODIC_MINOR,
    SCALE_TYPE_COUNT,
};

enum {
    VIBE_BASIC,
    VIBE_POLKA,
    VIBE_SWING,
    VIBE_WALTZ,
    VIBE_COUNT,
};

enum {
    SELECTABLE_TYPE_SCALE_DEGREE,
    SELECTABLE_TYPE_SCALE_ROOT,
    SELECTABLE_TYPE_SCALE_TYPE,
    SELECTABLE_TYPE_VIBE,
    SELECTABLE_TYPE_VIBES_PER_CHORD,
};

static Raylib *raylib;
static State *state;

inline static bool has_flag(int flag) {
    return (state->flags & flag) == flag;
}

inline static void toggle_flag(int flag) {
    state->flags ^= flag;
}

bool in_rectangle(Rectangle rec, Vector2 v) {
    return (
        v.x > rec.x &&
        v.y > rec.y &&
        v.x < rec.x + rec.width &&
        v.y < rec.y + rec.height
    );
}

inline static void draw_rectangle_lines(Rectangle rec) {
    raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, RECTANGLE_LINES_COLOR);
}

const char *get_note_name(int note) {
    if (has_flag(FLAG_FLATS)) {
        switch (note) {
            case NOTE_A: return "A";
            case NOTE_B_FLAT: return "B♭";
            case NOTE_B: return "B";
            case NOTE_C: return "C";
            case NOTE_D_FLAT: return "D♭";
            case NOTE_D: return "D";
            case NOTE_E_FLAT: return "E♭";
            case NOTE_E: return "E";
            case NOTE_F: return "F";
            case NOTE_G_FLAT: return "G♭";
            case NOTE_G: return "G";
            case NOTE_A_FLAT: return "A♭";
        }
    } else {
        switch (note) {
            case NOTE_A: return "A";
            case NOTE_A_SHARP: return "A♯";
            case NOTE_B: return "B";
            case NOTE_C: return "C";
            case NOTE_C_SHARP: return "C♯";
            case NOTE_D: return "D";
            case NOTE_D_SHARP: return "D♯";
            case NOTE_E: return "E";
            case NOTE_F: return "F";
            case NOTE_F_SHARP: return "F♯";
            case NOTE_G: return "G";
            case NOTE_G_SHARP: return "G♯";
        }
    }
    ASSERT(false);
    return NULL;
}

const char *get_scale_name(int scale_type) {
    switch (scale_type) {
        case SCALE_TYPE_MAJOR: return "Major";
        case SCALE_TYPE_DORIAN: return "Dorian";
        case SCALE_TYPE_PHRYGIAN: return "Phrygian";
        case SCALE_TYPE_LYDIAN: return "Lydian";
        case SCALE_TYPE_MIXOLYDIAN: return "Mixolydian";
        case SCALE_TYPE_MINOR: return "Minor";
        case SCALE_TYPE_LOCRIAN: return "Locrian";
        case SCALE_TYPE_HARMONIC_MINOR: return "Harmonic Minor";
        case SCALE_TYPE_MELODIC_MINOR: return "Melodic Minor";
    }
    ASSERT(false);
    return NULL;
}

const char *get_vibe_name(int vibe) {
    switch (vibe) {
        case VIBE_BASIC: return "Basic";
        case VIBE_POLKA: return "Polka";
        case VIBE_SWING: return "Swing";
        case VIBE_WALTZ: return "Waltz";
    }
    ASSERT(false);
    return NULL;
}

void draw_text_in_rectangle(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x + rec.width / 2, rec.y + rec.height / 2 };
    Vector2 dimensions = raylib->measure_text_ex(state->font, text, FONT_SIZE, state->font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    raylib->draw_text_pro(state->font, text, position, origin, 0, FONT_SIZE, state->font_spacing, color);
}

int truncate_accidentals(int note) {
    if (has_flag(FLAG_FLATS)) {
        switch (note) {
            case NOTE_A_FLAT: case NOTE_A:  return NOTE_A;
            case NOTE_B_FLAT: case NOTE_B:  return NOTE_B;
            case NOTE_C:                    return NOTE_C;
            case NOTE_D_FLAT: case NOTE_D:  return NOTE_D;
            case NOTE_E_FLAT: case NOTE_E:  return NOTE_E;
            case NOTE_F:                    return NOTE_F;
            case NOTE_G_FLAT: case NOTE_G:  return NOTE_G;
            default: ASSERT(false);
        }
    }
    switch (note) {
        case NOTE_A: case NOTE_A_SHARP: return NOTE_A;
        case NOTE_B:                    return NOTE_B;
        case NOTE_C: case NOTE_C_SHARP: return NOTE_C;
        case NOTE_D: case NOTE_D_SHARP: return NOTE_D;
        case NOTE_E:                    return NOTE_E;
        case NOTE_F: case NOTE_F_SHARP: return NOTE_F;
        case NOTE_G: case NOTE_G_SHARP: return NOTE_G;
        default: ASSERT(false);
    }
    return 0;
}

inline static float get_time_per_chord_max() {
    return TIME_PER_CHORD_MAX_MULTIPLIER * state->vibes_per_chord;
}

Chord get_sequencer_chord(int degree) {
    Chord chord = {0};

    if (degree == SCALE_DEGREE_NONE) {
        chord.type = CHORD_TYPE_NONE;
        return chord;
    }

    chord.root = (state->scale[degree % SCALE_DEGREE_COUNT] + state->scale_root) % NOTE_COUNT;
    chord.third = (state->scale[(degree + 2) % SCALE_DEGREE_COUNT] + state->scale_root) % NOTE_COUNT;
    chord.fifth = (state->scale[(degree + 4) % SCALE_DEGREE_COUNT] + state->scale_root) % NOTE_COUNT;

    int non_inversed_third = chord.third < chord.root ? chord.third + NOTE_COUNT : chord.third;
    int non_inversed_fifth = chord.fifth < chord.root ? chord.fifth + NOTE_COUNT : chord.fifth;

    int third_interval = non_inversed_third - chord.root;
    int fifth_interval = non_inversed_fifth - chord.root;

    switch (degree) {
        case SCALE_DEGREE_I: raylib->text_copy(chord.roman, "I"); break;
        case SCALE_DEGREE_II: raylib->text_copy(chord.roman, "II"); break;
        case SCALE_DEGREE_III: raylib->text_copy(chord.roman, "III"); break;
        case SCALE_DEGREE_IV: raylib->text_copy(chord.roman, "IV"); break;
        case SCALE_DEGREE_V: raylib->text_copy(chord.roman, "V"); break;
        case SCALE_DEGREE_VI: raylib->text_copy(chord.roman, "VI"); break;
        case SCALE_DEGREE_VII: raylib->text_copy(chord.roman, "VII"); break;
    }

    switch (third_interval) {
        case INTERVAL_MAJOR_THIRD:
            switch (fifth_interval) {
                case INTERVAL_FIFTH:
                    chord.type = CHORD_TYPE_MAJOR;
                    break;
                case INTERVAL_AUGMENTED_FIFTH:
                    chord.type = CHORD_TYPE_AUGMENTED;
                    raylib->text_copy(chord.roman, raylib->text_format("%s+", chord.roman));
                    break;
                default:
                    ASSERT(false);
            }
            break;
            break;
        case INTERVAL_MINOR_THIRD:
            switch (fifth_interval) {
                case INTERVAL_FIFTH:
                    chord.type = CHORD_TYPE_MINOR;
                    raylib->text_copy(chord.roman, raylib->text_to_lower(chord.roman));
                    break;
                case INTERVAL_FLAT_FIFTH:
                    chord.type = CHORD_TYPE_DIMINISHED;
                    raylib->text_copy(chord.roman, raylib->text_to_lower(chord.roman));
                    raylib->text_copy(chord.roman, raylib->text_format("%s°", chord.roman));
                    break;
                default:
                    ASSERT(false);
            }
            break;
        default:
            ASSERT(false);
    }

    int natural_scale_root = truncate_accidentals(state->scale_root);

    int natural_chord_root = natural_scale_root;
    for (int i = 0; i < degree; i++) {
        int old = natural_chord_root;
        int new = natural_chord_root;
        while (new == old) {
            natural_chord_root = (natural_chord_root + 1) % NOTE_COUNT;
            new = truncate_accidentals(natural_chord_root);
        }
        natural_chord_root = new;
    }

    switch (natural_chord_root) {
        case NOTE_A: raylib->text_copy(chord.symbol, "A"); break;
        case NOTE_B: raylib->text_copy(chord.symbol, "B"); break;
        case NOTE_C: raylib->text_copy(chord.symbol, "C"); break;
        case NOTE_D: raylib->text_copy(chord.symbol, "D"); break;
        case NOTE_E: raylib->text_copy(chord.symbol, "E"); break;
        case NOTE_F: raylib->text_copy(chord.symbol, "F"); break;
        case NOTE_G: raylib->text_copy(chord.symbol, "G"); break;
        default: ASSERT(false);
    }

    int accidentals = (chord.root - natural_chord_root + NOTE_COUNT) % NOTE_COUNT;
    if (accidentals > NOTE_COUNT / 2) {
        accidentals -= NOTE_COUNT;
    }
    if (accidentals != 0) {
        char * accidental_text;
        switch (accidentals) {
            case -2: accidental_text = "𝄫"; break;
            case -1: accidental_text = "♭"; break;
            case 1: accidental_text = "♯"; break;
            case 2: accidental_text = "𝄪"; break;
            default:
                ASSERT(false);
        }

        raylib->text_copy(chord.symbol, raylib->text_format("%s%s", chord.symbol, accidental_text));
    }

    if (chord.type != CHORD_TYPE_MAJOR) {
        char *extension;
        switch (chord.type) {
            case CHORD_TYPE_MINOR: extension = "m"; break;
            case CHORD_TYPE_DIMINISHED: extension = "°"; break;
            case CHORD_TYPE_AUGMENTED: extension = "+"; break;
            default: ASSERT(false);
        }

        raylib->text_copy(chord.symbol, raylib->text_format("%s%s", chord.symbol, extension));
    }

    return chord;
}

inline static float get_thing_height() {
    return raylib->get_screen_height() / VERTICAL_POSITION_COUNT;
}

inline static float get_selectable_item_height() {
    return get_thing_height() / 2;
}

inline static float get_sequencer_width() {
    return raylib->get_screen_width() / SEQUENCER_AMOUNT * 2;
}

inline static float get_sequencer_height() {
    return get_thing_height() * 3;
}

Rectangle get_audio_button_rectangle() {
    float width = raylib->get_screen_width() / 4;
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_AUDIO_CONTROLS,
        .width = width,
        .height = height,
    };
}

Rectangle get_volume_slider_rectangle() {
    Rectangle rec = get_audio_button_rectangle();
    return (Rectangle) {
        .x = rec.x + rec.width,
        .y = rec.y,
        .width = raylib->get_screen_width() - rec.width,
        .height = rec.height,
    };
}

Rectangle get_vibe_button_rectangle() {
    float width = raylib->get_screen_width() / 4;
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_TIMING_CONTROLS,
        .width = width,
        .height = height,
    };
}

Rectangle get_vibes_per_chord_rectangle() {
    Rectangle rec = get_vibe_button_rectangle();
    return (Rectangle) {
        .x = rec.x + rec.width,
        .y = rec.y,
        .width = rec.width,
        .height = rec.height,
    };
}

Rectangle get_interval_slider_rectangle() {
    Rectangle vibe_button_rec = get_vibe_button_rectangle();
    Rectangle vibes_per_chord_rec = get_vibes_per_chord_rectangle();
    return (Rectangle) {
        .x = vibes_per_chord_rec.x + vibes_per_chord_rec.width,
        .y = vibe_button_rec.y,
        .width = raylib->get_screen_width() - vibe_button_rec.width - vibes_per_chord_rec.width,
        .height = vibe_button_rec.height,
    };
}

Rectangle get_scale_root_button_rectangle() {
    float width = raylib->get_screen_width() / 4;
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_SCALE_BUTTONS,
        .width = width,
        .height = height,
    };
}

Rectangle get_scale_accidental_button_rectangle() {
    Rectangle rec = get_scale_root_button_rectangle();
    return (Rectangle) {
        .x = rec.x + rec.width,
        .y = rec.y,
        .width = rec.width,
        .height = rec.height,
    };
}

Rectangle get_scale_button_rectangle() {
    Rectangle root_rec = get_scale_root_button_rectangle();
    Rectangle accidental_rec = get_scale_accidental_button_rectangle();
    return (Rectangle) {
        .x = accidental_rec.x + accidental_rec.width,
        .y = root_rec.y,
        .width = raylib->get_screen_width() - root_rec.width - accidental_rec.width,
        .height = root_rec.height,
    };
}

Rectangle get_progress_bar_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_PROGRESS_BAR,
        .width = raylib->get_screen_width(),
        .height = height,
    };
}

Rectangle get_sequencer_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_SEQUENCER,
        .width = raylib->get_screen_width(),
        .height = get_sequencer_height(),
    };
}

Rectangle get_sequencer_b_rectangle() {
    Rectangle rec = get_sequencer_rectangle();
    rec.y = get_thing_height() * VERTICAL_POSITION_OF_SEQUENCER_B;
    return rec;
}

void prepare_select_state(int selectable_type, Vector2 position, uint8 *reference) {
    state->selectables.type = selectable_type;
    state->selectables.reference = reference;
    switch (selectable_type) {
        case SELECTABLE_TYPE_SCALE_DEGREE:
            Chord chord;
            state->selectables.item_count = SCALE_DEGREE_COUNT + 1;
            for (int i = 0; i < SCALE_DEGREE_COUNT; i++) {
                Chord chord = get_sequencer_chord(i);
                const char *text = raylib->text_format("%s (%s)", chord.roman, chord.symbol);
                raylib->text_copy(state->selectables.items[i], text);
            }
            raylib->text_copy(state->selectables.items[SCALE_DEGREE_COUNT], "off");
            break;
        case SELECTABLE_TYPE_SCALE_ROOT:
            state->selectables.item_count = NOTE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_note_name(i);
                raylib->text_copy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_SCALE_TYPE:
            state->selectables.item_count = SCALE_TYPE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_scale_name(i);
                raylib->text_copy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_VIBE:
            state->selectables.item_count = VIBE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_vibe_name(i);
                raylib->text_copy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_VIBES_PER_CHORD:
            state->selectables.item_count = 5;
            raylib->text_copy(state->selectables.items[0], "1/chord");
            raylib->text_copy(state->selectables.items[1], "2/chord");
            raylib->text_copy(state->selectables.items[2], "4/chord");
            raylib->text_copy(state->selectables.items[3], "8/chord");
            raylib->text_copy(state->selectables.items[4], "16/chord");
            break;
    }

    float screen_width = raylib->get_screen_width();
    float screen_height = raylib->get_screen_height();

    state->selectables.rectangle.width = screen_width * SELECTABLES_BOX_WIDTH_MULTIPLIER;

    if (position.x < screen_width / 2) {
        state->selectables.rectangle.x = position.x;
    } else {
        state->selectables.rectangle.x = position.x - state->selectables.rectangle.width;
    }

    state->selectables.rectangle.height = state->selectables.item_count * get_selectable_item_height();

    if (position.y < screen_height / 2) {
        state->selectables.rectangle.y = position.y;
    } else {
        state->selectables.rectangle.y = position.y - state->selectables.rectangle.height;
    }

    state->state = STATE_SELECT;
}

void draw_scale_root_button() {
    Rectangle rec = get_scale_root_button_rectangle();
    const char *text = get_note_name(state->scale_root);
    raylib->draw_rectangle_rec(rec, BG_COLOR_ACTIVE);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

void draw_scale_accidental_button() {
    Rectangle rec = get_scale_accidental_button_rectangle();

    const char *text = has_flag(FLAG_FLATS) ? "Flats" : "Sharps";

    raylib->draw_rectangle_rec(rec, BG_COLOR_ACTIVE);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

void draw_scale_button() {
    Rectangle rec = get_scale_button_rectangle();
    const char *text = get_scale_name(state->scale_type);
    raylib->draw_rectangle_rec(rec, BG_COLOR_ACTIVE);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

void draw_audio_button() {
    Rectangle rec = get_audio_button_rectangle();
    const char *text;
    Color bg_color;
    Color text_color;
    if (has_flag(FLAG_AUDIO)) {
        text = "Audio";
        bg_color = BG_COLOR_MUTE;
        text_color = TEXT_COLOR_MUTE;
    } else {
        text = "Muted";
        bg_color = BG_COLOR_UNMUTE;
        text_color = TEXT_COLOR_UNMUTE;
    }
    raylib->draw_rectangle_rec(rec, bg_color);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, text_color);
}

void draw_volume_slider() {
    Rectangle rec = get_volume_slider_rectangle();
    float fill = rec.width * state->volume_manual;
    Color filled_color;
    Color unfilled_color;
    Color text_color;
    if (has_flag(FLAG_AUDIO)) {
        filled_color = COLOR_FILLED_ACTIVE;
        unfilled_color = COLOR_UNFILLED_ACTIVE;
        text_color = TEXT_COLOR_ACTIVE;
    } else {
        filled_color = COLOR_FILLED_INACTIVE;
        unfilled_color = COLOR_UNFILLED_INACTIVE;
        text_color = TEXT_COLOR_INACTIVE;
    }
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, filled_color);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.x + rec.width - fill, rec.height, unfilled_color);
    draw_rectangle_lines(rec);
    const char *text = raylib->text_format("Volume: %.2f%%", state->volume_manual * 100);
    draw_text_in_rectangle(rec, text, text_color);
}

void draw_vibes_per_chord_button() {
    Rectangle rec = get_vibes_per_chord_rectangle();
    raylib->draw_rectangle_rec(rec, BG_COLOR_ACTIVE);
    draw_rectangle_lines(rec);
    const char *text = raylib->text_format("%d/chord", state->vibes_per_chord);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

void draw_interval_slider() {
    Rectangle rec = get_interval_slider_rectangle();
    float fill = rec.width * ((float)state->time_per_chord / get_time_per_chord_max());
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED_ACTIVE);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED_ACTIVE);
    draw_rectangle_lines(rec);
    const char *text = raylib->text_format("Interval: %.1f sec", state->time_per_chord);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

bool is_sequencer_active() {
    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        if (state->sequencer[i] != SCALE_DEGREE_NONE) {
            return true;
        }
    }
    return false;
}

void draw_sequencer() {
    Rectangle sequencer_rec = get_sequencer_rectangle();
    float width = get_sequencer_width();

    Rectangle chord_rec = sequencer_rec;
    chord_rec.width = width;
    chord_rec.height /= 3;

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        Chord chord = get_sequencer_chord(state->sequencer[i]);
        bool enabled = chord.type != CHORD_TYPE_NONE;

        chord_rec.x = width * i;
        if (i > (SEQUENCER_AMOUNT / 2) - 1) {
            chord_rec.x -= sequencer_rec.width;
            chord_rec.y = get_thing_height() * VERTICAL_POSITION_OF_SEQUENCER_B;
        }

        const char *chord_name = enabled ? chord.symbol : "---";
        draw_text_in_rectangle(chord_rec, chord_name, TEXT_COLOR_ACTIVE);

        Rectangle scale_degree_rec = chord_rec;
        scale_degree_rec.y += chord_rec.height;
        const char *scale_degree_text = enabled ? chord.roman : "off";
        Color text_color = TEXT_COLOR_INACTIVE;

        switch (state->sequencer[i]) {
            case SCALE_DEGREE_I: text_color = (Color){255, 0, 0, 255}; break;
            case SCALE_DEGREE_II: text_color = (Color){255, 128, 0, 255}; break;
            case SCALE_DEGREE_III: text_color = (Color){255, 255, 0, 255}; break;
            case SCALE_DEGREE_IV: text_color = (Color){0, 255, 0, 255}; break;
            case SCALE_DEGREE_V: text_color = (Color){0, 128, 255, 255}; break;
            case SCALE_DEGREE_VI: text_color = (Color){128, 0, 255, 255}; break;
            case SCALE_DEGREE_VII: text_color = (Color){255, 0, 255, 255}; break;
        }

        Color bg_color = (Color){
            text_color.r / 4,
            text_color.g / 4,
            text_color.b / 4,
            255
        };

        raylib->draw_rectangle_rec(scale_degree_rec, bg_color);
        draw_rectangle_lines(scale_degree_rec);

        if (i == state->chord_idx) {
            Rectangle outline = scale_degree_rec;
            outline.x += RECTANGLE_LINES_SIZE;
            outline.y += RECTANGLE_LINES_SIZE;
            outline.width -= RECTANGLE_LINES_SIZE * 2;
            outline.height -= RECTANGLE_LINES_SIZE * 2;

            raylib->draw_rectangle_lines_ex(outline, RECTANGLE_LINES_SIZE, text_color);

            Vector2 v1 = { scale_degree_rec.x + (width * 0.5f), scale_degree_rec.y + (scale_degree_rec.height * 1.25f) };
            Vector2 v2 = { scale_degree_rec.x + (width * 0.25f), scale_degree_rec.y + (scale_degree_rec.height * 1.75f) };
            Vector2 v3 = { scale_degree_rec.x + (width * 0.75f), v2.y };
            raylib->draw_triangle(v1, v2, v3, text_color);
        }

        draw_text_in_rectangle(scale_degree_rec, scale_degree_text, text_color);
    }
}

void draw_vibe_button() {
    const char *text = get_vibe_name(state->vibe);
    Rectangle rec = get_vibe_button_rectangle();
    raylib->draw_rectangle_rec(rec, BG_COLOR_ACTIVE);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_ACTIVE);
}

void draw_progress_bar() {
    Rectangle rec = get_progress_bar_rectangle();
    float fill = rec.width * (state->chord_timer / state->time_per_chord);
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED_PROGRESS_BAR);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED_PROGRESS_BAR);
    draw_rectangle_lines(rec);
}

void draw_selectables() {
    float item_height = get_selectable_item_height();
    Vector2 mouse_position = raylib->get_mouse_position();
    for (int i = 0; i < state->selectables.item_count; i++) {
        Rectangle rec;
        rec.x = state->selectables.rectangle.x;
        rec.y = state->selectables.rectangle.y + (item_height * i);
        rec.width = state->selectables.rectangle.width;
        rec.height = item_height;
        Color bg_color;
        if (in_rectangle(rec, mouse_position)) {
            bg_color = SELECTABLE_ITEM_BG_COLOR_SELECTED;
        } else if (i % 2 == 0) {
            bg_color = SELECTABLE_ITEM_BG_COLOR_EVEN;
        } else {
            bg_color = SELECTABLE_ITEM_BG_COLOR_ODD;
        }
        const char *text = state->selectables.items[i];
        raylib->draw_rectangle_rec(rec, bg_color);
        Vector2 position = { rec.x + item_height / 4, rec.y + rec.height / 2 };
        Vector2 dimensions = raylib->measure_text_ex(state->font, text, FONT_SIZE, state->font_spacing);
        Vector2 origin = { 0, dimensions.y / 2 };
        raylib->draw_text_pro(state->font, text, position, origin, 0, FONT_SIZE, state->font_spacing, TEXT_COLOR_ACTIVE);
    }
}

float note_to_freq(uint8 note, int octave) {
    int semitone_index = note + (octave - 4) * 12;
    return 440.0f * powf(2.0f, semitone_index / 12.0f);
}

#define FREQ_COUNT 4
void chord_synthesizer(void *buffer, unsigned int frames) {
    if (!has_flag(FLAG_AUDIO)) {
        return;
    }

    Chord chord = get_sequencer_chord(state->sequencer[state->chord_idx]);

    if (chord.type == CHORD_TYPE_NONE) {
        return;
    }

    float freq[FREQ_COUNT];
    {
        int r = chord.root;
        int t = chord.third;
        int f = chord.fifth;

        if (r < f) {
            freq[0] = note_to_freq(r, 4);
            freq[1] = note_to_freq(t, 4);
            freq[2] = note_to_freq(f, 4);
        } else if (t < r) {
            freq[0] = note_to_freq(t, 4);
            freq[1] = note_to_freq(f, 4);
            freq[2] = note_to_freq(r, 4);
        } else {
            freq[0] = note_to_freq(f, 4);
            freq[1] = note_to_freq(r, 4);
            freq[2] = note_to_freq(t, 4);
        }

        freq[3] = freq[2] / 2.0f;
    }

    float sample_rate = 44100.0f;

    float lfo_depth = 5.0f;
    float lfo_rate = 6.0f;
    static float lfo_phase = 0.0f;

    float cutoff_freq = 500.0f * state->volume_fade;
    float alpha = 1.0f / (1.0f + (sample_rate / cutoff_freq));
    static float prev_output = 0.0f;

    float incr[FREQ_COUNT] = {0};
    static float phase[FREQ_COUNT] = {0};
    short *d = (short *)buffer;

    float range = state->time_per_chord / state->vibes_per_chord;
    float position = state->chord_timer;
    while (position > range) position -= range;

    int active_tri = 0;

    switch (state->vibe) {
        case VIBE_BASIC: {
            active_tri = 15;
            break;
        }

        case VIBE_POLKA: {
            float fract = range / 8.0f;

            if (position < fract * 2.0f) {
                active_tri |= 1;
            } else if (position < fract * 3.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else if (position < fract * 4.0f) {
                // nothing
            } else if (position < fract * 6.0f) {
                active_tri |= 8;
            } else if (position < fract * 7.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else {
                // nothing
            }

            break;
        }

        case VIBE_SWING: {
            float fract = range / 6.0f;

            if (position < fract * 1.0f) {
                active_tri |= 1;
            } else if (position < fract * 2.0f) {
                // nothing
            } else if (position < fract * 3.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else if (position < fract * 4.0f) {
                active_tri |= 8;
            } else if (position < fract * 5.0f) {
                // nothing
            } else {
                active_tri |= 2;
                active_tri |= 4;
            }

            break;
        }

        case VIBE_WALTZ: {
            float fract = range / 12.0f;

            if (position < fract * 2.0f) {
                active_tri |= 1;
            } else if (position < fract * 3.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else if (position < fract * 4.0f) {
                // nothing
            } else if (position < fract * 5.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else if (position < fract * 6.0f) {
                // nothing
            } else if (position < fract * 8.0f) {
                active_tri |= 8;
            } else if (position < fract * 9.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else if (position < fract * 10.0f) {
                // nothing
            } else if (position < fract * 11.0f) {
                active_tri |= 2;
                active_tri |= 4;
            } else {
                // nothing
            }

            break;
        }
    }

    for (unsigned int i = 0; i < frames; i++) {
        float vibrato = sinf(2 * PI * lfo_phase) * lfo_depth;

        float tri[4] = {0};

        for (int j = 0; j < FREQ_COUNT; j++) {
            incr[j] = (freq[j] + vibrato) / sample_rate;
            tri[j] = (phase[j] < 0.5f) ? (4.0f * phase[j] - 1.0f) : (3.0f - 4.0f * phase[j]);
        }

        float sample = 0.0f;
        int divide = 0;
        for (int j = 0; j < FREQ_COUNT; j++) {
            if ((active_tri & (1 << j)) != 0) {
                sample += tri[j];
                divide++;
            }
        }
        if (divide > 0) {
            sample /= divide;
        }

        float filtered_sample = alpha * sample + (1.0f - alpha) * prev_output;
        prev_output = filtered_sample;

        d[i] = (short)(32000.0f * filtered_sample) * state->volume_fade * state->volume_manual;

        for (int j = 0; j < FREQ_COUNT; j++) {
            phase[j] += incr[j];
            if (phase[j] > 1.0f) phase[j] -= 1.0f;
        }

        lfo_phase += lfo_rate / sample_rate;
        if (lfo_phase > 1.0f) lfo_phase -= 1.0f;
    }
}

void generate_scale(Scale scale) {
    for (int i = 0; i < SCALE_DEGREE_COUNT; i++) {
        state->scale[i] = scale[i];
    }
}

void refresh_scale() {
    switch (state->scale_type) {
        case SCALE_TYPE_MAJOR: generate_scale(SCALE_MAJOR); break;
        case SCALE_TYPE_DORIAN: generate_scale(SCALE_DORIAN); break;
        case SCALE_TYPE_PHRYGIAN: generate_scale(SCALE_PHRYGIAN); break;
        case SCALE_TYPE_LYDIAN: generate_scale(SCALE_LYDIAN); break;
        case SCALE_TYPE_MIXOLYDIAN: generate_scale(SCALE_MIXOLYDIAN); break;
        case SCALE_TYPE_MINOR: generate_scale(SCALE_MINOR); break;
        case SCALE_TYPE_LOCRIAN: generate_scale(SCALE_LOCRIAN); break;
        case SCALE_TYPE_HARMONIC_MINOR: generate_scale(SCALE_HARMONIC_MINOR); break;
        case SCALE_TYPE_MELODIC_MINOR: generate_scale(SCALE_MELODIC_MINOR); break;
    }
}

void progress() {
    state->chord_timer = 0.0f;
    Chord chord;
    if (is_sequencer_active()) {
        do {
            state->chord_idx = (state->chord_idx + 1) % SEQUENCER_AMOUNT;
            chord = get_sequencer_chord(state->sequencer[state->chord_idx]);
        } while (chord.type == CHORD_TYPE_NONE);
    } else {
        chord.type = CHORD_TYPE_NONE;
    }
}

DLL_EXPORT void init(Raylib *_raylib, State *_state) {
    raylib = _raylib;
    state = _state;

    state->state = STATE_MAIN;
    state->vibe = VIBE_BASIC;
    state->vibes_per_chord = 4;
    state->flags = (FLAG_AUDIO);
    state->time_per_chord = get_time_per_chord_max() / 2;
    state->volume_manual = 0.5f;
    state->scale_root = NOTE_C;
    state->scale_type = SCALE_TYPE_MAJOR;
    refresh_scale();

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        state->sequencer[i] = SCALE_DEGREE_NONE;
    }

    raylib->init_audio_device();
    raylib->set_audio_stream_buffer_size_default(4096);
    state->audio_stream = raylib->load_audio_stream(44100, 16, 1);

    int asciiStart = 32;
    int asciiEnd = 126;
    int asciiCount = asciiEnd - asciiStart + 1;

    int extraSymbols[] = {
        CHAR_SHARP,
        CHAR_DOUBLE_SHARP,
        CHAR_FLAT,
        CHAR_DOUBLE_FLAT,
        CHAR_NATURAL,
        CHAR_DIMINISHED,
        CHAR_HALF_DIMINISHED,
    };

    int extraSymbolCount = sizeof(extraSymbols) / sizeof(extraSymbols[0]);
    int totalCount = asciiCount + extraSymbolCount;

    int allChars[totalCount];
    for (int i = 0; i < asciiCount; i++) {
        allChars[i] = asciiStart + i;
    }

    for (int i = 0; i < extraSymbolCount; i++) {
        allChars[asciiCount + i] = extraSymbols[i];
    }

    state->font = raylib->load_font_ex("NotoMusic-Regular.ttf", 200, allChars, totalCount);
    state->font_spacing = 2;

    state->chord_idx = 0;
}

DLL_EXPORT void on_load(Raylib *_raylib, State *_state) {
    raylib = _raylib;
    state = _state;
    raylib->set_audio_stream_callback(state->audio_stream, chord_synthesizer);
    raylib->play_audio_stream(state->audio_stream);
}

DLL_EXPORT void on_unload(Raylib *raylib, State *state) {
    raylib->set_audio_stream_callback(state->audio_stream, NULL);
}

DLL_EXPORT void update(Raylib *raylib, State *state) {
    Vector2 mouse_position = raylib->get_mouse_position();

    if (state->volume_fade < 1.0f) {
        state->volume_fade += raylib->get_frame_time() * 10.0f;
        if (state->volume_fade > 1.0f) {
            state->volume_fade = 1.0f;
        }
    }

    {
        Rectangle rec = get_volume_slider_rectangle(state);
        if (raylib->is_mouse_button_down(MOUSE_BUTTON_LEFT) && in_rectangle(rec, mouse_position)) {
            state->volume_manual = (mouse_position.x - rec.x) / rec.width;
        }
    }

    state->chord_timer += raylib->get_frame_time();
    if (state->chord_timer > state->time_per_chord) {
        progress();
    }

    float vibes_per_chord = state->vibes_per_chord;
    {
        Rectangle rec = get_interval_slider_rectangle(state);
        if (raylib->is_mouse_button_down(0) && in_rectangle(rec, mouse_position)) {
            state->time_per_chord = ((mouse_position.x - rec.x) / rec.width) * get_time_per_chord_max();
        }
    }

    if (raylib->is_mouse_button_pressed(0)) {
        switch (state->state) {
            case STATE_MAIN: {
                bool in_sequencer = in_rectangle(get_sequencer_rectangle(), mouse_position);
                bool in_sequencer_b = in_rectangle(get_sequencer_b_rectangle(), mouse_position);

                if (in_sequencer || in_sequencer_b) {
                    Rectangle rec = in_sequencer ? get_sequencer_rectangle() : get_sequencer_b_rectangle();
                    int x = mouse_position.x / get_sequencer_width();
                    int y = (mouse_position.y - rec.y) / (rec.height / 3);
                    if (in_sequencer_b) {
                        x += SEQUENCER_AMOUNT / 2;
                    }

                    switch (y) {
                        case 0: // chord symbol section
                            break;
                        case 1: // roman numeral section
                            prepare_select_state(SELECTABLE_TYPE_SCALE_DEGREE, mouse_position, &(state->sequencer[x]));
                            break;
                        case 2:// sequencer cursor section
                            state->chord_idx = x;
                            state->chord_timer = 0.0f;
                            break;
                        default:
                            ASSERT(false);
                            break;
                    }
                } else if (in_rectangle(get_audio_button_rectangle(), mouse_position)) {
                    toggle_flag(FLAG_AUDIO);
                } else if (in_rectangle(get_progress_bar_rectangle(), mouse_position)) {
                    progress();
                } else if (in_rectangle(get_vibe_button_rectangle(), mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBE, mouse_position, &(state->vibe));
                } else if (in_rectangle(get_vibes_per_chord_rectangle(), mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBES_PER_CHORD, mouse_position, &(state->vibes_per_chord));
                } else if (in_rectangle(get_scale_root_button_rectangle(), mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_ROOT, mouse_position, &(state->scale_root));
                } else if (in_rectangle(get_scale_accidental_button_rectangle(), mouse_position)) {
                    toggle_flag(FLAG_FLATS);
                } else if (in_rectangle(get_scale_button_rectangle(), mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_TYPE, mouse_position, &(state->scale_root));
                }
            } break;
            case STATE_SELECT: {
                if (in_rectangle(state->selectables.rectangle, mouse_position)) {
                    int item_idx = (mouse_position.y - state->selectables.rectangle.y) / get_selectable_item_height();
                    switch (state->selectables.type) {
                        default:
                            *(state->selectables.reference) = item_idx;
                            break;
                        case SELECTABLE_TYPE_SCALE_TYPE:
                            state->scale_type = item_idx;
                            refresh_scale();
                            break;
                        case SELECTABLE_TYPE_VIBES_PER_CHORD:
                            state->vibes_per_chord = pow(2, item_idx);
                            state->time_per_chord = get_time_per_chord_max() / 2;
                            break;
                    }
                }
                state->state = STATE_MAIN;
            } break;
        }
    }
}

DLL_EXPORT void render(Raylib *raylib, State *state) {
    raylib->clear_background(BG_COLOR);
    draw_audio_button();
    draw_volume_slider();

    draw_sequencer();

    draw_progress_bar();

    draw_scale_root_button();
    draw_scale_accidental_button();
    draw_scale_button();

    draw_vibe_button();
    draw_vibes_per_chord_button();
    draw_interval_slider();

    if (state->state == STATE_SELECT) {
        draw_selectables();
    }
}

DLL_EXPORT void cleanup(Raylib *raylib, State *state) {
    raylib->unload_audio_stream(state->audio_stream);
    raylib->close_audio_device();
}

