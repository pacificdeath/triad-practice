#include "./raylib/include/raylib.h"

#include "math.h"

#include <stdint.h>
typedef uint8_t uint8;

#define BUTTON_FRACT_HEIGHT 10
#define BUTTON_FRACT_WIDTH 7

#define NOTE_A 0
#define NOTE_A_SHARP 1
#define NOTE_B 2
#define NOTE_C 3
#define NOTE_C_SHARP 4
#define NOTE_D 5
#define NOTE_D_SHARP 6
#define NOTE_E 7
#define NOTE_F 8
#define NOTE_F_SHARP 9
#define NOTE_G 10
#define NOTE_G_SHARP 11
#define NOTE_COUNT 12

#define BG_COLOR ((Color){32,32,32,255})
#define ACTIVE_COLOR ((Color){0,128,0,255})
#define INACTIVE_COLOR ((Color){128,0,0,255})
#define SLIDER_FILLED_COLOR ((Color){0,128,0,255})
#define SLIDER_UNFILLED_COLOR ((Color){0,64,0,255})
#define TEXT_COLOR ((Color){255,255,255,255})

typedef enum Mode {
    MODE_MANUAL,
    MODE_AUTOMATIC,
} Mode;

typedef enum Flags {
    FLAG_NONE = 0,
    FLAG_AUDIO = 1 << 0,
    FLAG_MAJOR = 1 << 1,
    FLAG_MINOR = 1 << 2,
    FLAG_DIM = 1 << 3,
    FLAG_NATURAL = 1 << 4,
    FLAG_SHARP = 1 << 5,
    FLAG_FLAT = 1 << 6,
} Flags;

typedef uint8 chordstr[8];

typedef struct Chord {
    uint8 root;
    uint8 third;
    uint8 fifth;
    chordstr text;
    Color text_color;
} Chord;

typedef struct Area {
    int x;
    int y;
    int width;
    int height;
} Area;

typedef uint8 chord_map[NOTE_COUNT];

typedef struct State {
    AudioStream audio_stream;
    Font font;
    int font_spacing;
    int screen_width;
    int screen_height;
    Mode mode;
    float mode_auto_interval;
    float mode_auto_timer;
    Flags flags;
    chord_map chord_map;
    Chord upcoming_chord;
} State;

static Chord global_chord;
static float volume_fade = 0.0f;
static float volume_manual = 0.5f;

bool has_flag(State *state, int flag) {
    return (state->flags & flag) == flag;
}

bool in_area(Area a, Vector2 v) {
    return (
        v.x > a.x &&
        v.y > a.y &&
        v.x < a.x + a.width &&
        v.y < a.y + a.height
    );
}

void update_chord_map(State *state) {
    bool has_maj = has_flag(state, FLAG_MAJOR);
    bool has_min = has_flag(state, FLAG_MINOR);
    bool has_dim = has_flag(state, FLAG_DIM);
    int types = has_maj + has_min + has_dim;
    int i = 0;
    switch (types) {
    case 1: {
        int type;
        if (has_maj) type = FLAG_MAJOR;
        else if (has_min) type = FLAG_MINOR;
        else type = FLAG_DIM;
        for (; i < NOTE_COUNT; i++) state->chord_map[i] = type;
    } break;
    case 2: {
        int type_a;
        int type_b;
        if (has_maj) {
            type_a = FLAG_MAJOR;
            type_b = has_min ? FLAG_MINOR : FLAG_DIM;
        } else {
            type_a = FLAG_MINOR;
            type_b = FLAG_DIM;
        }
        int half = (NOTE_COUNT / 2);
        for (; i < half; i++) state->chord_map[i] = type_a;
        for (; i < NOTE_COUNT; i++) state->chord_map[i] = type_b;
    } break;
    case 3: {
        int third = (NOTE_COUNT / 3);
        for (; i < third; i++) state->chord_map[i] = FLAG_MAJOR;
        for (; i < third * 2; i++) state->chord_map[i] = FLAG_MINOR;
        for (; i < NOTE_COUNT; i++) state->chord_map[i] = FLAG_DIM;
    } break;
    default: {
        for (; i < NOTE_COUNT; i++) state->chord_map[i] = FLAG_NONE;
    } break;
    }
}

void update_upcoming_chord(State *state) {
    int i = 0;

    {
        uint8 note_map[11];
        uint8 note_map_count = 0;
        switch (state->flags & (FLAG_NATURAL|FLAG_SHARP|FLAG_FLAT)) {
        case FLAG_NONE:
        case FLAG_NATURAL:
            note_map[note_map_count++] = NOTE_A;
            note_map[note_map_count++] = NOTE_B;
            note_map[note_map_count++] = NOTE_C;
            note_map[note_map_count++] = NOTE_D;
            note_map[note_map_count++] = NOTE_E;
            note_map[note_map_count++] = NOTE_F;
            note_map[note_map_count++] = NOTE_G;
            break;
        case FLAG_SHARP:
        case FLAG_FLAT:
        case (FLAG_SHARP | FLAG_FLAT):
            note_map[note_map_count++] = NOTE_A_SHARP;
            note_map[note_map_count++] = NOTE_C_SHARP;
            note_map[note_map_count++] = NOTE_D_SHARP;
            note_map[note_map_count++] = NOTE_F_SHARP;
            note_map[note_map_count++] = NOTE_G_SHARP;
            break;
        default:
            for (; note_map_count <= NOTE_G_SHARP; note_map_count++) {
                note_map[note_map_count] = note_map_count;
            }
            break;
        }
        int prev_root = state->upcoming_chord.root;
        int new_root = prev_root;

        while (prev_root == new_root) {
            new_root = note_map[GetRandomValue(0, note_map_count - 1)];
        }
        state->upcoming_chord.root = new_root;

        char natural_note = 0;
        char sharp_note = 0;
        char flat_note = 0;
    
        switch (new_root) {
        case 0:     natural_note = 'A'; break;
        case 1:         sharp_note = 'A'; flat_note = 'B'; break;
        case 2:     natural_note = 'B'; break;
        case 3:     natural_note = 'C'; break;
        case 4:         sharp_note = 'C'; flat_note = 'D'; break;
        case 5:     natural_note = 'D'; break;
        case 6:         sharp_note = 'D'; flat_note = 'E'; break;
        case 7:     natural_note = 'E'; break;
        case 8:     natural_note = 'F'; break;
        case 9:         sharp_note = 'F'; flat_note = 'G'; break;
        case 10:    natural_note = 'G'; break;
        case 11:        sharp_note = 'G'; flat_note = 'B'; break;
        default: break;
        }

        if (has_flag(state, FLAG_SHARP) && sharp_note) {
            if (has_flag(state, FLAG_FLAT) && flat_note) {
                if (GetRandomValue(0, 1) == 0) {
                    state->upcoming_chord.text[i++] = sharp_note;
                    state->upcoming_chord.text[i++] = '#';
                } else {
                    state->upcoming_chord.text[i++] = flat_note;
                    state->upcoming_chord.text[i++] = 'b';
                }
            } else {
                state->upcoming_chord.text[i++] = sharp_note;
                state->upcoming_chord.text[i++] = '#';
            }
        } else if (has_flag(state, FLAG_FLAT) && flat_note) {
            state->upcoming_chord.text[i++] = flat_note;
            state->upcoming_chord.text[i++] = 'b';
        } else {
            state->upcoming_chord.text[i++] = natural_note;
        }
    }

    switch (state->chord_map[GetRandomValue(0, NOTE_COUNT - 1)]) {
    default:
    case FLAG_MAJOR: {
        state->upcoming_chord.third = (state->upcoming_chord.root + 4) % NOTE_COUNT;
        state->upcoming_chord.fifth = (state->upcoming_chord.root + 7) % NOTE_COUNT;
    } break;
    case FLAG_MINOR: {
        state->upcoming_chord.third = (state->upcoming_chord.root + 3) % NOTE_COUNT;
        state->upcoming_chord.fifth = (state->upcoming_chord.root + 7) % NOTE_COUNT;
        state->upcoming_chord.text[i++] = 'm';
    } break;
    case FLAG_DIM: {
        state->upcoming_chord.third = (state->upcoming_chord.root + 3) % NOTE_COUNT;
        state->upcoming_chord.fifth = (state->upcoming_chord.root + 6) % NOTE_COUNT;
        state->upcoming_chord.text[i++] = 'd';
        state->upcoming_chord.text[i++] = 'i';
        state->upcoming_chord.text[i++] = 'm';
    } break;
    }
    state->upcoming_chord.text[i] = '\0';

    int random_color_channel = GetRandomValue(1, 3);
    state->upcoming_chord.text_color = (Color) {
        random_color_channel == 1 ? 255 : GetRandomValue(0, 255),
        random_color_channel == 2 ? 255 : GetRandomValue(0, 255),
        random_color_channel == 3 ? 255 : GetRandomValue(0, 255),
        255
    };
}

void toggle_flag(State *state, int flag) {
    if (has_flag(state, flag)) {
        state->flags &= ~flag;
    } else {
        state->flags |= flag;
    }
}

void str_copy(char *dst, char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int get_button_width(State *state) {
    return state->screen_width / BUTTON_FRACT_WIDTH;
}

int get_button_height(State* state) {
    return state->screen_height / BUTTON_FRACT_HEIGHT;
}

Area get_interval_slider_area(State *state) {
    int padding = state->screen_height / 30;
    int button_height = get_button_height(state);
    int width = get_button_width(state) - (padding * 2);
    return (Area) {
        .x = state->screen_width - padding - width,
        .y = button_height + padding,
        .width = width,
        .height = state->screen_height - (2 * button_height) - (padding * 2),
    };
}

Area get_volume_slider_area(State *state) {
    int padding = state->screen_height / 30;
    int button_height = get_button_height(state);
    return (Area) {
        .x = padding,
        .y = button_height + padding,
        .width = get_button_width(state) - (padding * 2),
        .height = state->screen_height - (2 * button_height) - (padding * 2),
    };
}

Area get_manual_next_chord_button(State *state) {
    int padding = state->screen_height / 30;
    int button_width = get_button_width(state) * 2;
    int button_height = get_button_height(state);
    return (Area) {
        .x = (state->screen_width / 2) - (button_width / 2),
        .y = state->screen_height - (button_height * 2) - padding,
        .width = button_width,
        .height = button_height
    };
}

void draw_volume_slider(State *state) {
    Area a = get_volume_slider_area(state);
    int fill = a.height * volume_manual;
    DrawRectangle(a.x, a.y + a.height - fill, a.width, fill, SLIDER_FILLED_COLOR);
    DrawRectangle(a.x, a.y, a.width, a.height - fill, SLIDER_UNFILLED_COLOR);

    {
        const int font_size = state->font.baseSize;
        const int font_spacing = 2;
        const char *text = TextFormat("Volume: %i%%", (int)(volume_manual * 100));
        Vector2 position = { a.x + (a.width / 2), a.y + (a.height / 2) };
        Vector2 dimensions = MeasureTextEx(state->font, text, font_size, font_spacing);
        Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
        DrawTextPro(state->font, text, position, origin, 270, font_size, font_spacing, TEXT_COLOR);
    }
}

void draw_manual_next_chord_button(State *state) {
    Area a = get_manual_next_chord_button(state);
    DrawRectangle(a.x, a.y, a.width, a.height, INACTIVE_COLOR);

    {
        const int font_size = state->font.baseSize;
        const int font_spacing = 2;
        const char *text = TextFormat("Next: %s", state->upcoming_chord.text);
        Vector2 position = { a.x + (a.width / 2), a.y + (a.height / 2) };
        Vector2 dimensions = MeasureTextEx(state->font, text, font_size, font_spacing);
        Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
        DrawTextPro(state->font, text, position, origin, 0, font_size, font_spacing, TEXT_COLOR);
    }
}

void draw_auto_seconds_slider(State *state) {
    Area a = get_interval_slider_area(state);
    int fill = a.height * ((float)state->mode_auto_interval / 10.0f);
    DrawRectangle(a.x, a.y + a.height - fill, a.width, fill, SLIDER_FILLED_COLOR);
    DrawRectangle(a.x, a.y, a.width, a.height - fill, SLIDER_UNFILLED_COLOR);

    {
        const int font_size = state->font.baseSize;
        const int font_spacing = 2;
        const char *text = TextFormat("Interval: %.1f sec", state->mode_auto_interval);
        Vector2 position = { a.x + (a.width / 2), a.y + (a.height / 2) };
        Vector2 dimensions = MeasureTextEx(state->font, text, font_size, font_spacing);
        Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
        DrawTextPro(state->font, text, position, origin, 270, font_size, font_spacing, TEXT_COLOR);
    }
}

void draw_top_buttons(State *state, int idx, Flags flag) {
    int width = get_button_width(state);
    int height = get_button_height(state);
    int x = width * idx;
    int y = 0;

    Color color = has_flag(state, flag)
        ? ACTIVE_COLOR
        : INACTIVE_COLOR;

    DrawRectangle(x, y, width, height, color);

    Vector2 position = { x + (width / 2), y + (height / 2) };

    char text[8];
    switch (flag) {
    case FLAG_AUDIO: str_copy(text, "Audio"); break;
    case FLAG_MAJOR: str_copy(text, "Major"); break;
    case FLAG_MINOR: str_copy(text, "Minor"); break;
    case FLAG_DIM: str_copy(text, "Dim"); break;
    case FLAG_NATURAL: str_copy(text, "Natural"); break;
    case FLAG_SHARP: str_copy(text, "Sharp"); break;
    case FLAG_FLAT: str_copy(text, "Flat"); break;
    }

    const int font_size = state->font.baseSize;
    const int font_spacing = 2;

    Vector2 dimensions = MeasureTextEx(state->font, text, font_size, font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size, font_spacing, TEXT_COLOR);
}

void draw_mode(State *state) {
    char text[32];
    int mode_height = state->screen_height / 10;
    int y = state->screen_height - mode_height;
    Color mode_color = ACTIVE_COLOR;
    switch (state->mode) {
    case MODE_MANUAL: {
        str_copy(text, "Manual");
        DrawRectangle(0, y, state->screen_width, mode_height, mode_color);
    } break;
    case MODE_AUTOMATIC: {
        str_copy(text, "Automatic");
        int fill = state->screen_width * (state->mode_auto_timer / state->mode_auto_interval);
        Color unfilled_color = {0,64,0,255};
        DrawRectangle(0, y, fill, mode_height, mode_color);
        DrawRectangle(fill, y, state->screen_width - fill, mode_height, unfilled_color);
        draw_auto_seconds_slider(state);
    } break;
    }

    const int font_size = state->font.baseSize;
    const int font_spacing = 2;

    Vector2 position = { state->screen_width / 2, y + (mode_height / 2) };
    Vector2 dimensions = MeasureTextEx(state->font, text, font_size, font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size, font_spacing, TEXT_COLOR);
}

float note_to_freq(uint8 note) {
    return 440.0f * powf(2.0f, (float)note / (float)NOTE_COUNT);
}

void chord_synthesizer(void *buffer, unsigned int frames) {
    float freq1 = note_to_freq(global_chord.root);
    float freq2 = note_to_freq(global_chord.third);
    float freq3 = note_to_freq(global_chord.fifth);

    float sample_rate = 44100.0f;

    float lfo_depth = 3.0f;
    float lfo_rate = 4.0f;
    static float lfo_phase = 0.0f;

    float cutoff_freq = 1000.0f * volume_fade;
    float alpha = 1.0f / (1.0f + (sample_rate / cutoff_freq));
    static float prev_output = 0.0f;

    float incr1, incr2, incr3;
    static float phase1 = 0.0f, phase2 = 0.0f, phase3 = 0.0f;
    short *d = (short *)buffer;

    for (unsigned int i = 0; i < frames; i++) {
        float vibrato = sinf(2 * PI * lfo_phase) * lfo_depth;

        incr1 = (freq1 + vibrato) / sample_rate;
        incr2 = (freq2 + vibrato) / sample_rate;
        incr3 = (freq3 + vibrato) / sample_rate;

        float tri1 = (phase1 < 0.5f) ? (4.0f * phase1 - 1.0f) : (3.0f - 4.0f * phase1);
        float tri2 = (phase2 < 0.5f) ? (4.0f * phase2 - 1.0f) : (3.0f - 4.0f * phase2);
        float tri3 = (phase3 < 0.5f) ? (4.0f * phase3 - 1.0f) : (3.0f - 4.0f * phase3);

        float sample = (tri1 + tri2 + tri3) / 3.0f;

        float filtered_sample = alpha * sample + (1.0f - alpha) * prev_output;
        prev_output = filtered_sample;

        d[i] = (short)(32000.0f * filtered_sample) * volume_fade * volume_manual;

        phase1 += incr1;
        phase2 += incr2;
        phase3 += incr3;

        if (phase1 > 1.0f) phase1 -= 1.0f;
        if (phase2 > 1.0f) phase2 -= 1.0f;
        if (phase3 > 1.0f) phase3 -= 1.0f;

        lfo_phase += lfo_rate / sample_rate;
        if (lfo_phase > 1.0f) lfo_phase -= 1.0f;
    }
}

void input(State *state) {
    int button_idx = -1;
    bool is_mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (is_mouse_pressed) {
        Vector2 position = GetMousePosition();
        if (position.y < state->screen_height / BUTTON_FRACT_HEIGHT) {
            int w = state->screen_width / BUTTON_FRACT_WIDTH;
            button_idx = position.x / w;
        }
    }

    if (has_flag(state, FLAG_AUDIO)) {
        if (volume_fade < 1.0f) {
            volume_fade += GetFrameTime() * 10.0f;
            if (volume_fade > 1.0f) {
                volume_fade = 1.0f;
            }
        }
        Area a = get_volume_slider_area(state);
        Vector2 mouse_position = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && in_area(a, mouse_position)) {
            volume_manual = ((a.height - mouse_position.y) + a.y) / a.height;
        }
    } else {
        if (volume_fade > 0.0f) {
            volume_fade -= GetFrameTime() * 10.0f;
            if (volume_fade < 0.0f) {
                volume_fade = 0.0f;
                StopAudioStream(state->audio_stream);
            }
        }
    }

    switch (button_idx) {
    case 0: {
        toggle_flag(state, FLAG_AUDIO);
        if (has_flag(state, FLAG_AUDIO) && !IsAudioStreamPlaying(state->audio_stream)) {
            PlayAudioStream(state->audio_stream);
        }
    } break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6: {
        toggle_flag(state, (1 << button_idx));
        update_chord_map(state);
        update_upcoming_chord(state);
    } break;
    default: break;
    }
    if ( is_mouse_pressed &&
            GetMousePosition().y > (state->screen_height - get_button_height(state))
    ) {
        if (state->mode == MODE_MANUAL) {
            state->mode = MODE_AUTOMATIC;
            state->mode_auto_timer = 0.0f;
        } else {
            state->mode = MODE_MANUAL;
        }
    }

    bool space = IsKeyPressed(KEY_SPACE);
    bool next_button_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && in_area(get_manual_next_chord_button(state), GetMousePosition());
    if (space || next_button_pressed) {
        global_chord = state->upcoming_chord;
        update_upcoming_chord(state);
    }

    if (state->mode == MODE_AUTOMATIC) {
        state->mode_auto_timer += GetFrameTime();
        if (state->mode_auto_timer > state->mode_auto_interval) {
            global_chord = state->upcoming_chord;
            update_upcoming_chord(state);
            state->mode_auto_timer = 0.0f;
        }
        Area a = get_interval_slider_area(state);
        Vector2 mouse_position = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && in_area(a, mouse_position)) {
            state->mode_auto_interval = (((a.height - mouse_position.y) + a.y) / a.height) * 10.0f;
            if (state->mode_auto_interval <= 0.5f) {
                state->mode_auto_interval = 0.5f;
            }
        }
    }
}

int main(void) {
    State state = {0};
    state.flags |= FLAG_NONE;
    state.screen_width = 1200;
    state.screen_height = 800;
    state.mode_auto_interval = 5;

    InitWindow(state.screen_width, state.screen_height, "Death");
    SetTargetFPS(60);

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);
    state.audio_stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(state.audio_stream, chord_synthesizer);

    state.font = LoadFont("arial.ttf");
    state.font_spacing = 2;

    update_chord_map(&state);

    update_upcoming_chord(&state);
    global_chord = state.upcoming_chord;
    update_upcoming_chord(&state);

    while (!WindowShouldClose()) {
        input(&state);

        BeginDrawing();

        ClearBackground(BG_COLOR);

        int big_text_font_size = state.font.baseSize * 8.0f;
        int big_text_spacing = 2;
        Vector2 big_text_position = {
            state.screen_width / 2,
            state.screen_height / 2
        };
        Vector2 big_text_dimensions = MeasureTextEx(state.font, global_chord.text, big_text_font_size, big_text_spacing);
        Vector2 main_text_origin = {
            (big_text_dimensions.x / 2),
            (big_text_dimensions.y / 2),
        };
        DrawTextPro(
            state.font,
            global_chord.text,
            big_text_position,
            main_text_origin,
            0.0f,
            big_text_font_size,
            big_text_spacing,
            global_chord.text_color
        );

        draw_top_buttons(&state, 0, FLAG_AUDIO);
        draw_top_buttons(&state, 1, FLAG_MAJOR);
        draw_top_buttons(&state, 2, FLAG_MINOR);
        draw_top_buttons(&state, 3, FLAG_DIM);
        draw_top_buttons(&state, 4, FLAG_NATURAL);
        draw_top_buttons(&state, 5, FLAG_SHARP);
        draw_top_buttons(&state, 6, FLAG_FLAT);

        draw_manual_next_chord_button(&state);

        if (has_flag(&state, FLAG_AUDIO)) {
            draw_volume_slider(&state);
        }

        draw_mode(&state);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
