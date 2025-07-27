#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include "../raylib/include/raylib.h"

#ifdef DEBUG
    #include <stdio.h>
    #include <stdlib.h>
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
    int (*text_copy)(char *dst, const char *src);

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

#define SCALE_DEGREE_COUNT 7
typedef uint8 Scale[SCALE_DEGREE_COUNT];

#define SEQUENCER_AMOUNT 16
typedef uint8 Sequencer[SEQUENCER_AMOUNT];

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
    Sequencer sequencer;
    int chord_idx;
    float volume_fade;
    float volume_manual;
    Selectables selectables;
} State;

#endif

