#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "../raylib/include/raylib.h"

#define WINDOW_NAME "Triad Practice"

#define TP_BG                               ((Color){0x28, 0x18, 0x10, 0xff})
#define TP_BG2                              ((Color){0x18, 0x08, 0x00, 0xff})
#define TP_FG                               ((Color){0xd0, 0xc0, 0x90, 0xff})
#define TP_RED                              ((Color){0x40, 0x00, 0x00, 0xff})
#define TP_YELLOW                           ((Color){0x40, 0x40, 0x00, 0xff})
#define TP_ORANGE                           ((Color){0x40, 0x20, 0x00, 0xff})
#define TP_GREEN                            ((Color){0x00, 0x40, 0x00, 0xff})

#define COLOR_FILLED                        TP_GREEN
#define COLOR_UNFILLED                      TP_BG2
#define COLOR_CURSOR_CURRENT                ((Color){0,255,0,255})
#define COLOR_CURSOR_SELECTION              ((Color){64,64,64,255})

#define VIBES_PER_CHORD_MIN 1
#define VIBES_PER_CHORD_MAX 16

#define CHAR_SINGLE_8TH_NOTE    0x266A  // ♪
#define CHAR_BEAMED_8TH_NOTES   0x266B  // ♫
#define CHAR_SHARP              0x266F  // ♯
#define CHAR_FLAT               0x266D  // ♭
#define CHAR_DIMINISHED         0x00B0  // °

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

#define CMD_MAX_TEXT 64

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
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
    float min_time_per_chord;
    float max_time_per_chord;
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
    char cmd_buffer[CMD_MAX_TEXT];
    int cmd_cursor;
} State;

enum {
    STATE_MAIN,
    STATE_SELECT,
    STATE_SAVE_FILE,
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
    SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL,
    SEQUENCER_ELEMENT_SECTION_BUTTON,
    SEQUENCER_ELEMENT_SECTION_CURSOR,

    SEQUENCER_STATE_SECTION_RESET = SEQUENCER_ELEMENT_SECTION_BUTTON,
    SEQUENCER_STATE_SECTION_ENABLE = SEQUENCER_ELEMENT_SECTION_CURSOR,
};

enum {
    CONTROLS_SCALE,
    CONTROLS_ROOT_NOTE,
    CONTROLS_VIBE,
    CONTROLS_VIBES_PER_CHORD,
    CONTROLS_ACCIDENTAL,
    CONTROLS_VOLUME,
    CONTROLS_INTERVAL,
    CONTROLS_COUNT,
    CONTROLS_COLUMN_COUNT = (CONTROLS_COUNT / 2) + 1,
};

#define SEQUENCER_ROWS (SEQUENCER_AMOUNT * 3)
enum {
    VERTICAL_POSITION_OF_FILE_CONTROLS = 0,
    VERTICAL_POSITION_OF_SEQUENCER = 1,
    VERTICAL_POSITION_OF_PLAY_BUTTON = VERTICAL_POSITION_OF_SEQUENCER + SEQUENCER_ROWS,
    VERTICAL_POSITION_OF_CONTROLS,
    VERTICAL_POSITION_OF_CMD = VERTICAL_POSITION_OF_CONTROLS + (CONTROLS_COLUMN_COUNT),
    VERTICAL_POSITION_COUNT,
};

