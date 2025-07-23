#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include "../raylib/include/raylib.h"

typedef uint8_t uint8;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef struct Raylib {
    bool (*is_window_resized)();
    int (*get_screen_width)(void);
    int (*get_screen_height)(void);

    float (*get_frame_time)(void);
    int (*get_random_value)(int min, int max);
    const char *(*text_format)(const char *text, ...);
    const char *(*text_to_lower)(const char *text);

    bool (*is_key_pressed)(int key);
    bool (*is_mouse_button_pressed)(int button);
    bool (*is_mouse_button_down)(int button);
    Vector2 (*get_mouse_position)(void);

    void (*init_audio_device)(void);
    void (*close_audio_device)(void);
    AudioStream (*load_audio_stream)(uint32 sample_rate, uint32 sample_size, uint32 channels);
    void (*play_audio_stream)(AudioStream stream);
    void (*stop_audio_stream)(AudioStream stream);
    bool (*is_audio_stream_playing)(AudioStream stream);
    void (*unload_audio_stream)(AudioStream stream);
    void (*set_audio_stream_callback)(AudioStream stream, AudioCallback callback);
    void (*set_audio_stream_buffer_size_default)(int size);

    Font (*load_font_ex)(const char *file_name, int font_size, int *codepoints, int codepoint_count);
    Vector2 (*measure_text_ex)(Font font, const char *text, float font_size, float spacing);
    void (*draw_text_pro)(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint);

    void (*clear_background)(Color color);
    void (*draw_rectangle)(int pos_x, int pos_y, int width, int height, Color color);
    void (*draw_rectangle_rec)(Rectangle rec, Color color);
    void (*draw_rectangle_lines_ex)(Rectangle rec, float line_thick, Color color);
    void (*draw_triangle)(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
} Raylib;

enum {
    NOTE_A,
    NOTE_A_SHARP,
    NOTE_B,
    NOTE_C,
    NOTE_C_SHARP,
    NOTE_D,
    NOTE_D_SHARP,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP,
    NOTE_G ,
    NOTE_G_SHARP ,
    NOTE_COUNT,
};

enum {
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

#define SCALE_NOTE_COUNT 7
typedef uint8 Scale[SCALE_NOTE_COUNT];
#define SCALE_MAJOR             ((Scale){0, 2, 4, 5, 7, 9, 11})
#define SCALE_DORIAN            ((Scale){0, 2, 3, 5, 7, 9, 10})
#define SCALE_PHRYGIAN          ((Scale){0, 1, 3, 5, 7, 8, 10})
#define SCALE_LYDIAN            ((Scale){0, 2, 4, 6, 7, 9, 11})
#define SCALE_MIXOLYDIAN        ((Scale){0, 2, 4, 5, 7, 9, 10})
#define SCALE_MINOR             ((Scale){0, 2, 3, 5, 7, 8, 10})
#define SCALE_LOCRIAN           ((Scale){0, 1, 3, 5, 6, 8, 10})
#define SCALE_HARMONIC_MINOR    ((Scale){0, 2, 3, 5, 7, 8, 11})
#define SCALE_MELODIC_MINOR     ((Scale){0, 2, 3, 5, 7, 9, 11})

#define SEQUENCER_AMOUNT 8
typedef uint8 Sequencer[SEQUENCER_AMOUNT];

typedef uint8 chordstr[8];
typedef struct Chord {
    uint8 root;
    uint8 third;
    uint8 fifth;
    chordstr text;
    uint8 type;
} Chord;

typedef struct State {
    AudioStream audio_stream;
    Font font;
    int font_spacing;
    int vibe;
    float chord_interval;
    float chord_timer;
    uint64 flags;
    Scale scale;
    uint8 scale_type;
    uint8 scale_root;
    Sequencer sequencer;
    int current_chord_idx;
    Chord current_chord;
    float volume_fade;
    float volume_manual;
} State;

#endif

