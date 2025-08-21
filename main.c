#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "raylib/include/raylib.h"

#define WINDOW_NAME "Triad Practice"

#define BG_COLOR                            ((Color){32,32,32,255})
#define BG_COLOR_THING                      ((Color){0,128,255,255})

#define TEXT_COLOR_THING                    ((Color){255,255,255,255})

#define COLOR_FILLED                        ((Color){0,128,255,255})
#define COLOR_UNFILLED                      ((Color){0,64,128,255})
#define COLOR_FILLED_PROGRESS_BAR           ((Color){0,255,0,255})
#define COLOR_UNFILLED_PROGRESS_BAR         ((Color){0,128,0,255})
#define COLOR_CURSOR_CURRENT                ((Color){0,255,0,255})
#define COLOR_CURSOR_SELECTION              ((Color){64,64,64,255})

#define RECTANGLE_LINES_COLOR ((Color){0,0,0,255})

#define TIME_PER_CHORD_MAX_MULTIPLIER 2.0f
#define VIBES_PER_CHORD_MIN 1
#define VIBES_PER_CHORD_MAX 16

#define CHAR_SINGLE_8TH_NOTE    0x266A  // ♪
#define CHAR_BEAMED_8TH_NOTES   0x266B  // ♫
#define CHAR_SHARP              0x266F  // ♯
#define CHAR_FLAT               0x266D  // ♭
#define CHAR_NATURAL            0x266E  // ♮
#define CHAR_DIMINISHED         0x00B0  // °
#define CHAR_PLAY               0x25b6  // ▶

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

#define SELECTABLES_BOX_WIDTH_MULTIPLIER 0.4f
#define SELECTABLE_ITEM_BG_COLOR_ODD ((Color){48,48,48,255})
#define SELECTABLE_ITEM_BG_COLOR_EVEN ((Color){64,64,64,255})
#define SELECTABLE_ITEM_BG_COLOR_SELECTED ((Color){0,0,255,255})

#ifdef DEBUG
    #include <stdio.h>
    __attribute__((unused))
    static void are_you_a_horrible_person(bool condition, char *condition_string, char *file_name, int line_number) {
        if (!(condition)) {
            printf("You are a horrible person\n");
            printf(" -> ");
            printf("%s", file_name);
            printf(":");
            printf("%i\n", line_number);
            printf(" -> (");
            printf("%s", condition_string);
            printf(")\n");
            exit(1);
        }
    }
    #define ASSERT(condition) do { are_you_a_horrible_person(condition, #condition, __FILE__, __LINE__); } while (0)
#else
    #define ASSERT(condition)
#endif

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define SCALE_DEGREE_COUNT 7
typedef uint8 Scale[SCALE_DEGREE_COUNT];

#define SEQUENCER_AMOUNT 4
#define SEQUENCER_ROW 8
#define SEQUENCER_ELEMENTS (SEQUENCER_AMOUNT * SEQUENCER_ROW)
typedef uint8 Sequencers[SEQUENCER_ELEMENTS];

#define CHORD_NAME_CAPACITY 16
typedef struct Chord {
    uint8 root;
    uint8 third;
    uint8 fifth;
    uint8 type;
    char symbol[CHORD_NAME_CAPACITY];
    char roman[CHORD_NAME_CAPACITY];
} Chord;

#define MAX_SELECTABLES 16
typedef struct Selectables {
    uint8 type;
    Rectangle rectangle;
    char items[MAX_SELECTABLES][32];
    uint8 item_count;
    uint8 *reference;
} Selectables;

typedef struct State {
    uint8 state;
    uint8 scale_type;
    uint8 scale_root;
    uint8 vibe;
    uint8 vibes_per_chord;
    AudioStream audio_stream;
    Font font;
    int font_spacing;
    float time_per_chord;
    float chord_timer;
    int flags;
    Scale scale;
    Sequencers sequencer;
    bool sequencer_states[SEQUENCER_AMOUNT];
    uint8 sequencer_reps[SEQUENCER_AMOUNT];
    int chord_idx;
    float volume_fade;
    float volume_manual;
    Selectables selectables;
    Vector2 mouse_position;
} State;

enum {
    STATE_MAIN,
    STATE_SELECT,
};

#define SEQUENCER_ROWS (SEQUENCER_AMOUNT * 3)
enum {
    VERTICAL_POSITION_OF_AUDIO_CONTROLS = 0,
    VERTICAL_POSITION_OF_SEQUENCER = 1,
    VERTICAL_POSITION_OF_PROGRESS_BAR = VERTICAL_POSITION_OF_SEQUENCER + SEQUENCER_ROWS,
    VERTICAL_POSITION_OF_SCALE_BUTTONS,
    VERTICAL_POSITION_OF_TIMING_CONTROLS,
    VERTICAL_POSITION_COUNT,
};

enum {
    FLAG_PLAYING = (1 << 0),
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
    VIBE_POLKA,
    VIBE_SWING,
    VIBE_WALTZ,
    VIBE_CHORD,
    VIBE_ROOT,
    VIBE_THIRD,
    VIBE_FIFTH,
    VIBE_COUNT,
};

enum {
    SELECTABLE_TYPE_SCALE_DEGREE,
    SELECTABLE_TYPE_SCALE_ROOT,
    SELECTABLE_TYPE_SCALE_TYPE,
    SELECTABLE_TYPE_VIBE,
    SELECTABLE_TYPE_VIBES_PER_CHORD,
};

enum {
    SEQUENCER_STATE_1X,
    SEQUENCER_STATE_2X,
    SEQUENCER_STATE_3X,
    SEQUENCER_STATE_4X,
    SEQUENCER_STATE_OFF,
};

enum {
    SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL,
    SEQUENCER_ELEMENT_SECTION_BUTTON,
    SEQUENCER_ELEMENT_SECTION_CURSOR,
};

static State *state;

inline static float size_multiplier() {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    return (width < height) ? width : height;
}

inline static float font_size() {
    return 0.04f * size_multiplier();
}

inline static int rectangle_lines_size() {
    return 0.004f * size_multiplier();
}

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
    DrawRectangleLinesEx(rec, rectangle_lines_size(), RECTANGLE_LINES_COLOR);
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
        case VIBE_POLKA: return "Polka";
        case VIBE_SWING: return "Swing";
        case VIBE_WALTZ: return "Waltz";
        case VIBE_CHORD: return "Chord";
        case VIBE_ROOT: return "Root";
        case VIBE_THIRD: return "Third";
        case VIBE_FIFTH: return "Fifth";
    }
    ASSERT(false);
    return NULL;
}

void draw_text_in_rectangle(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x + rec.width / 2, rec.y + rec.height / 2 };
    Vector2 dimensions = MeasureTextEx(state->font, text, font_size(), state->font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, color);
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
        case SCALE_DEGREE_I: TextCopy(chord.roman, "I"); break;
        case SCALE_DEGREE_II: TextCopy(chord.roman, "II"); break;
        case SCALE_DEGREE_III: TextCopy(chord.roman, "III"); break;
        case SCALE_DEGREE_IV: TextCopy(chord.roman, "IV"); break;
        case SCALE_DEGREE_V: TextCopy(chord.roman, "V"); break;
        case SCALE_DEGREE_VI: TextCopy(chord.roman, "VI"); break;
        case SCALE_DEGREE_VII: TextCopy(chord.roman, "VII"); break;
    }

    switch (third_interval) {
        case INTERVAL_MAJOR_THIRD:
            switch (fifth_interval) {
                case INTERVAL_FIFTH:
                    chord.type = CHORD_TYPE_MAJOR;
                    break;
                case INTERVAL_AUGMENTED_FIFTH:
                    chord.type = CHORD_TYPE_AUGMENTED;
                    TextCopy(chord.roman, TextFormat("%s+", chord.roman));
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
                    TextCopy(chord.roman, TextToLower(chord.roman));
                    break;
                case INTERVAL_FLAT_FIFTH:
                    chord.type = CHORD_TYPE_DIMINISHED;
                    TextCopy(chord.roman, TextToLower(chord.roman));
                    TextCopy(chord.roman, TextFormat("%s°", chord.roman));
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
        case NOTE_A: TextCopy(chord.symbol, "A"); break;
        case NOTE_B: TextCopy(chord.symbol, "B"); break;
        case NOTE_C: TextCopy(chord.symbol, "C"); break;
        case NOTE_D: TextCopy(chord.symbol, "D"); break;
        case NOTE_E: TextCopy(chord.symbol, "E"); break;
        case NOTE_F: TextCopy(chord.symbol, "F"); break;
        case NOTE_G: TextCopy(chord.symbol, "G"); break;
        default: ASSERT(false);
    }

    int accidentals = (chord.root - natural_chord_root + NOTE_COUNT) % NOTE_COUNT;
    if (accidentals > NOTE_COUNT / 2) {
        accidentals -= NOTE_COUNT;
    }
    if (accidentals != 0) {
        char * accidental_text;
        switch (accidentals) {
            case -2: accidental_text = "♭♭"; break;
            case -1: accidental_text = "♭"; break;
            case 1: accidental_text = "♯"; break;
            case 2: accidental_text = "♯♯"; break;
            default:
                ASSERT(false);
        }

        TextCopy(chord.symbol, TextFormat("%s%s", chord.symbol, accidental_text));
    }

    if (chord.type != CHORD_TYPE_MAJOR) {
        char *extension;
        switch (chord.type) {
            case CHORD_TYPE_MINOR: extension = "m"; break;
            case CHORD_TYPE_DIMINISHED: extension = "°"; break;
            case CHORD_TYPE_AUGMENTED: extension = "+"; break;
            default: ASSERT(false);
        }

        TextCopy(chord.symbol, TextFormat("%s%s", chord.symbol, extension));
    }

    return chord;
}

inline static float get_thing_height() {
    return GetScreenHeight() / VERTICAL_POSITION_COUNT;
}

inline static float get_selectable_item_height() {
    return get_thing_height() / 2;
}

Rectangle get_volume_slider_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_AUDIO_CONTROLS,
        .width = GetScreenWidth(),
        .height = height,
    };
}

Rectangle get_vibe_button_rectangle() {
    float width = GetScreenWidth() / 4;
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
        .width = GetScreenWidth() - vibe_button_rec.width - vibes_per_chord_rec.width,
        .height = vibe_button_rec.height,
    };
}

Rectangle get_scale_root_button_rectangle() {
    float width = GetScreenWidth() / 4;
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
        .width = GetScreenWidth() - root_rec.width - accidental_rec.width,
        .height = root_rec.height,
    };
}

Rectangle get_play_pause_button_rectangle() {
    float width = GetScreenWidth() / 4;
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_PROGRESS_BAR,
        .width = width,
        .height = height,
    };
}

Rectangle get_progress_bar_rectangle() {
    Rectangle rec = get_play_pause_button_rectangle();
    rec.x += rec.width;
    rec.width = GetScreenWidth() - rec.x;
    return rec;
}

inline static float get_sequencer_element_width() {
    return (float)(GetScreenWidth()) / ((float)SEQUENCER_ROW + 1.0f);
}

Rectangle get_sequencer_rectangle() {
    float element_width = get_sequencer_element_width();
    float height = get_thing_height();
    Rectangle rec;
    rec.x = element_width;
    rec.width = element_width * SEQUENCER_ROW;
    rec.y = height * VERTICAL_POSITION_OF_SEQUENCER;
    rec.height = height * SEQUENCER_ROWS;
    return rec;
}

Rectangle get_sequencer_state_section_rectangle() {
    Rectangle rec = get_sequencer_rectangle();
    rec.width /= SEQUENCER_ROW;
    rec.x -= rec.width;
    return rec;
}

Rectangle get_sequencer_state_rectangle(int idx) {
    Rectangle rec = get_sequencer_state_section_rectangle();
    rec.height /= SEQUENCER_AMOUNT;
    rec.y += (rec.height * idx);
    return rec;
}

Rectangle get_sequencer_element_rectangle(int idx) {
    Rectangle sequencer_rec = get_sequencer_rectangle();
    Rectangle element_rec;
    element_rec.width = sequencer_rec.width / SEQUENCER_ROW;
    element_rec.height = sequencer_rec.height / SEQUENCER_AMOUNT;
    element_rec.x = sequencer_rec.x + (element_rec.width * (idx % SEQUENCER_ROW));
    element_rec.y = sequencer_rec.y + (element_rec.height * (idx / SEQUENCER_ROW));
    return element_rec;
}

Rectangle get_sequencer_element_section_rectangle(Rectangle element_rec, int section) {
    element_rec.height /= 3;
    switch (section) {
        case SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL:
            break;
        case SEQUENCER_ELEMENT_SECTION_BUTTON:
            element_rec.y += element_rec.height;
            break;
        case SEQUENCER_ELEMENT_SECTION_CURSOR:
            element_rec.y += (element_rec.height * 2);
            break;
    }

    return element_rec;
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
                const char *text = TextFormat("%s (%s)", chord.roman, chord.symbol);
                TextCopy(state->selectables.items[i], text);
            }
            TextCopy(state->selectables.items[SCALE_DEGREE_COUNT], "off");
            break;
        case SELECTABLE_TYPE_SCALE_ROOT:
            state->selectables.item_count = NOTE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_note_name(i);
                TextCopy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_SCALE_TYPE:
            state->selectables.item_count = SCALE_TYPE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_scale_name(i);
                TextCopy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_VIBE:
            state->selectables.item_count = VIBE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                TextCopy(state->selectables.items[i], get_vibe_name(i));
            }
            break;
        case SELECTABLE_TYPE_VIBES_PER_CHORD:
            state->selectables.item_count = 5;
            TextCopy(state->selectables.items[0], "1/chord");
            TextCopy(state->selectables.items[1], "2/chord");
            TextCopy(state->selectables.items[2], "4/chord");
            TextCopy(state->selectables.items[3], "8/chord");
            TextCopy(state->selectables.items[4], "16/chord");
            break;
    }

    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

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
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
}

void draw_scale_accidental_button() {
    Rectangle rec = get_scale_accidental_button_rectangle();

    const char *text = has_flag(FLAG_FLATS) ? "Flats" : "Sharps";

    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
}

void draw_scale_button() {
    Rectangle rec = get_scale_button_rectangle();
    const char *text = get_scale_name(state->scale_type);
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
}

void draw_volume_slider() {
    Rectangle rec = get_volume_slider_rectangle();
    float fill = rec.width * state->volume_manual;
    Color filled_color;
    Color unfilled_color;
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED);
    DrawRectangle(rec.x + fill, rec.y, rec.x + rec.width - fill, rec.height, COLOR_UNFILLED);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("Volume: %.2f%%", state->volume_manual * 100), TEXT_COLOR_THING);
}

void draw_vibes_per_chord_button() {
    Rectangle rec = get_vibes_per_chord_rectangle();
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("%d/chord", state->vibes_per_chord), TEXT_COLOR_THING);
}

void draw_interval_slider() {
    Rectangle rec = get_interval_slider_rectangle();
    float fill = rec.width * ((float)state->time_per_chord / get_time_per_chord_max());
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED);
    DrawRectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("Interval: %.1f sec", state->time_per_chord), TEXT_COLOR_THING);
}

bool is_sequencer_active() {
    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        if (!state->sequencer_states[i]) {
            continue;
        }

        for (int j = 0; j < SEQUENCER_ROW; j++) {
            if (state->sequencer[(i * SEQUENCER_ROW) + j] != SCALE_DEGREE_NONE) {
                return true;
            }
        }
    }
    return false;
}

void draw_sequencer() {
    Rectangle sequencer_rec = get_sequencer_rectangle();

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        Rectangle state_rec = get_sequencer_state_rectangle(i);
        Rectangle sequencer_row_rec;
        sequencer_row_rec.x = sequencer_rec.x;
        sequencer_row_rec.y = state_rec.y;
        sequencer_row_rec.width = sequencer_rec.width;
        sequencer_row_rec.height = state_rec.height;

        draw_rectangle_lines(sequencer_row_rec);

        const char *state_text;
        Color state_text_color;
        if (state->sequencer_states[i]) {
            state_text = "ON";
            state_text_color = (Color){0,255,0,255};
        } else {
            state_text = "OFF";
            state_text_color = (Color){255,0,0,255};
        }

        draw_rectangle_lines(state_rec);
        draw_text_in_rectangle(state_rec, state_text, state_text_color);

        for (int j = 0; j < SEQUENCER_ROW; j++) {
            int element_idx = (i * SEQUENCER_ROW) + j;

            Rectangle element_rec = get_sequencer_element_rectangle(element_idx);

            Chord chord = get_sequencer_chord(state->sequencer[element_idx]);
            bool enabled = chord.type != CHORD_TYPE_NONE;

            Rectangle chord_symbol_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL);
            const char *chord_symbol = enabled ? chord.symbol : "---";
            draw_text_in_rectangle(chord_symbol_rec, chord_symbol, TEXT_COLOR_THING);

            Rectangle button_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_BUTTON);
            const char *button_text = enabled ? chord.roman : "off";
            Color button_text_color = (Color){64,64,64,255};
            if (state->sequencer_states[i]) {
                switch (state->sequencer[element_idx]) {
                    case SCALE_DEGREE_I: button_text_color = (Color){255, 0, 0, 255}; break;
                    case SCALE_DEGREE_II: button_text_color = (Color){255, 128, 0, 255}; break;
                    case SCALE_DEGREE_III: button_text_color = (Color){255, 255, 0, 255}; break;
                    case SCALE_DEGREE_IV: button_text_color = (Color){0, 255, 0, 255}; break;
                    case SCALE_DEGREE_V: button_text_color = (Color){0, 128, 255, 255}; break;
                    case SCALE_DEGREE_VI: button_text_color = (Color){128, 0, 255, 255}; break;
                    case SCALE_DEGREE_VII: button_text_color = (Color){255, 0, 255, 255}; break;
                }
            }
            Color button_bg_color = (Color){
                button_text_color.r / 4,
                button_text_color.g / 4,
                button_text_color.b / 4,
                255
            };
            DrawRectangleRec(button_rec, button_bg_color);
            draw_rectangle_lines(button_rec);
            draw_text_in_rectangle(button_rec, button_text, button_text_color);

            Rectangle cursor_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CURSOR);
            if (element_idx == state->chord_idx) {
                Rectangle outline = button_rec;
                outline.x += rectangle_lines_size();
                outline.y += rectangle_lines_size();
                outline.width -= rectangle_lines_size() * 2;
                outline.height -= rectangle_lines_size() * 2;

                DrawRectangleLinesEx(outline, rectangle_lines_size(), button_text_color);
                draw_text_in_rectangle(cursor_rec, "▶", button_text_color);
            }
        }
    }
}

void draw_vibe_button() {
    Rectangle rec = get_vibe_button_rectangle();
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, get_vibe_name(state->vibe), TEXT_COLOR_THING);
}

void draw_play_pause_button() {
    Rectangle rec = get_play_pause_button_rectangle();
    if (has_flag(FLAG_PLAYING)) {
        DrawRectangleRec(rec, COLOR_FILLED_PROGRESS_BAR);
        draw_text_in_rectangle(rec, "PLAYING", TEXT_COLOR_THING);
    } else {
        DrawRectangleRec(rec, COLOR_UNFILLED_PROGRESS_BAR);
        draw_text_in_rectangle(rec, "PAUSED", TEXT_COLOR_THING);
    }
    draw_rectangle_lines(rec);
}

void draw_progress_bar() {
    Rectangle rec = get_progress_bar_rectangle();
    float fill = rec.width * (state->chord_timer / state->time_per_chord);
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED_PROGRESS_BAR);
    DrawRectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED_PROGRESS_BAR);
    draw_rectangle_lines(rec);
}

void draw_selectables() {
    float item_height = get_selectable_item_height();
    for (int i = 0; i < state->selectables.item_count; i++) {
        Rectangle rec;
        rec.x = state->selectables.rectangle.x;
        rec.y = state->selectables.rectangle.y + (item_height * i);
        rec.width = state->selectables.rectangle.width;
        rec.height = item_height;
        Color bg_color;
        if (in_rectangle(rec, state->mouse_position)) {
            bg_color = SELECTABLE_ITEM_BG_COLOR_SELECTED;
        } else if (i % 2 == 0) {
            bg_color = SELECTABLE_ITEM_BG_COLOR_EVEN;
        } else {
            bg_color = SELECTABLE_ITEM_BG_COLOR_ODD;
        }
        const char *text = state->selectables.items[i];
        DrawRectangleRec(rec, bg_color);
        Vector2 position = { rec.x + item_height / 4, rec.y + rec.height / 2 };
        Vector2 dimensions = MeasureTextEx(state->font, text, font_size(), state->font_spacing);
        Vector2 origin = { 0, dimensions.y / 2 };
        DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, TEXT_COLOR_THING);
    }
}

float note_to_freq(uint8 note, int octave) {
    int semitone_index = note + (octave - 4) * 12;
    return 440.0f * powf(2.0f, semitone_index / 12.0f);
}

#define FREQ_COUNT 4
void chord_synthesizer(void *buffer, unsigned int frames) {
    if (!has_flag(FLAG_PLAYING) || !is_sequencer_active()) {
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

        switch (state->vibe) {
            case VIBE_POLKA:
            case VIBE_SWING:
            case VIBE_WALTZ:
            case VIBE_CHORD: {
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
            } break;
            case VIBE_ROOT:
            case VIBE_THIRD:
            case VIBE_FIFTH: {
                freq[0] = note_to_freq(r, 4);
                freq[1] = note_to_freq(t, 4);
                freq[2] = note_to_freq(f, 4);
            } break;
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

        case VIBE_CHORD: {
            active_tri = 15;
            break;
        }

        case VIBE_ROOT: {
            active_tri = 1;
            break;
        }

        case VIBE_THIRD: {
            active_tri = 2;
            break;
        }

        case VIBE_FIFTH: {
            active_tri = 4;
            break;
        }
    }

    for (unsigned int i = 0; i < frames; i++) {
        float vibrato = sinf(2 * PI * lfo_phase) * lfo_depth;

        float tri[4] = {0};

        for (int j = 0; j < FREQ_COUNT; j++) {
            float vibrated_freq = freq[j] * (1.0f + vibrato / freq[j]);
            incr[j] = vibrated_freq / sample_rate;
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
        int sequencer_idx;
        do {
            state->chord_idx = (state->chord_idx + 1) % SEQUENCER_ELEMENTS;
            sequencer_idx = state->chord_idx / SEQUENCER_ROW;
            chord = get_sequencer_chord(state->sequencer[state->chord_idx]);
        } while (!state->sequencer_states[sequencer_idx] || chord.type == CHORD_TYPE_NONE);
    } else {
        chord.type = CHORD_TYPE_NONE;
    }
}

void init() {
#ifndef DEBUG
    SetTraceLogLevel(LOG_WARNING);
#endif
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, WINDOW_NAME);
    SetTargetFPS(60);

    state = (State *)calloc(1, sizeof(State));
    state->state = STATE_MAIN;
    state->vibe = VIBE_POLKA;
    state->vibes_per_chord = 4;
    state->time_per_chord = get_time_per_chord_max() / 2;
    state->volume_manual = 0.5f;
    state->scale_root = NOTE_C;
    state->scale_type = SCALE_TYPE_MAJOR;

    refresh_scale();

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        state->sequencer_states[i] = false;
        for (int j = 0; j < SEQUENCER_ROW; j++) {
            state->sequencer[(i * SEQUENCER_ROW) + j] = SCALE_DEGREE_NONE;
        }
    }

    int asciiStart = 32;
    int asciiEnd = 126;
    int asciiCount = asciiEnd - asciiStart + 1;

    int extraSymbols[] = {
        CHAR_SINGLE_8TH_NOTE,
        CHAR_BEAMED_8TH_NOTES,
        CHAR_SHARP,
        CHAR_FLAT,
        CHAR_NATURAL,
        CHAR_DIMINISHED,
        CHAR_PLAY,
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

    state->font = LoadFontEx("DejaVuSans.ttf", 200, allChars, totalCount);
    state->font_spacing = 2;

    state->chord_idx = 0;

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);
    state->audio_stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(state->audio_stream, chord_synthesizer);
    PlayAudioStream(state->audio_stream);
}

void update() {
    state->mouse_position = GetMousePosition();

    if (state->volume_fade < 1.0f) {
        state->volume_fade += GetFrameTime() * 10.0f;
        if (state->volume_fade > 1.0f) {
            state->volume_fade = 1.0f;
        }
    }

    {
        Rectangle rec = get_volume_slider_rectangle(state);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && in_rectangle(rec, state->mouse_position)) {
            state->volume_manual = (state->mouse_position.x - rec.x) / rec.width;
        }
    }

    if (has_flag(FLAG_PLAYING)) {
        state->chord_timer += GetFrameTime();
        if (state->chord_timer > state->time_per_chord) {
            progress();
        }
    }

    float vibes_per_chord = state->vibes_per_chord;
    {
        Rectangle rec = get_interval_slider_rectangle(state);
        if (IsMouseButtonDown(0) && in_rectangle(rec, state->mouse_position)) {
            state->time_per_chord = ((state->mouse_position.x - rec.x) / rec.width) * get_time_per_chord_max();
        }
    }

    if (IsMouseButtonPressed(0)) {
        switch (state->state) {
            case STATE_MAIN: {
                if (in_rectangle(get_sequencer_state_section_rectangle(), state->mouse_position)) {
                    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
                        if (in_rectangle(get_sequencer_state_rectangle(i), state->mouse_position)) {
                            state->sequencer_states[i] = !state->sequencer_states[i];
                            break;
                        }
                    }
                } else if (in_rectangle(get_sequencer_rectangle(), state->mouse_position)) {
                    for (int i = 0; i < SEQUENCER_ELEMENTS; i++) {
                        if (in_rectangle(get_sequencer_element_rectangle(i), state->mouse_position)) {
                            Rectangle rec = get_sequencer_element_rectangle(i);
                            if (in_rectangle(get_sequencer_element_section_rectangle(rec, SEQUENCER_ELEMENT_SECTION_BUTTON), state->mouse_position)) {
                                prepare_select_state(SELECTABLE_TYPE_SCALE_DEGREE, state->mouse_position, &(state->sequencer[i]));
                            } else if (in_rectangle(get_sequencer_element_section_rectangle(rec, SEQUENCER_ELEMENT_SECTION_CURSOR), state->mouse_position)) {
                                state->chord_idx = i;
                            }
                            break;
                        }
                    }
                } else if (in_rectangle(get_progress_bar_rectangle(), state->mouse_position)) {
                    progress();
                } else if (in_rectangle(get_vibe_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBE, state->mouse_position, &(state->vibe));
                } else if (in_rectangle(get_vibes_per_chord_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBES_PER_CHORD, state->mouse_position, &(state->vibes_per_chord));
                } else if (in_rectangle(get_scale_root_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_ROOT, state->mouse_position, &(state->scale_root));
                } else if (in_rectangle(get_scale_accidental_button_rectangle(), state->mouse_position)) {
                    toggle_flag(FLAG_FLATS);
                } else if (in_rectangle(get_scale_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_TYPE, state->mouse_position, &(state->scale_root));
                } else if (in_rectangle(get_play_pause_button_rectangle(), state->mouse_position)) {
                    toggle_flag(FLAG_PLAYING);
                    if (has_flag(FLAG_PLAYING)) {
                        if (state->sequencer[state->chord_idx] == SCALE_DEGREE_NONE) {
                            progress();
                        }
                    } else {
                        state->chord_timer = 0.0f;
                    }
                }
            } break;
            case STATE_SELECT: {
                if (in_rectangle(state->selectables.rectangle, state->mouse_position)) {
                    int item_idx = (state->mouse_position.y - state->selectables.rectangle.y) / get_selectable_item_height();
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

void render() {
    BeginDrawing();

    ClearBackground(BG_COLOR);

    draw_volume_slider();

    draw_sequencer();

    draw_play_pause_button();
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

    EndDrawing();
}

void cleanup() {
    UnloadAudioStream(state->audio_stream);
    CloseAudioDevice();
    free(state);
    CloseWindow();
}

int main() {

    init();

    while (!WindowShouldClose()) {

        update();

        render();
    }

    cleanup();
}

