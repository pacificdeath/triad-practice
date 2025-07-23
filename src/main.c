#include "../raylib/include/raylib.h"
#include "engine.h"

#include "math.h"

#include <stdint.h>
#include <stdio.h>

#define VERTICAL_POSITION_OF_VOLUME_SLIDER 0
#define VERTICAL_POSITION_OF_INTERVAL_SLIDER 1
#define VERTICAL_POSITION_OF_SCALE_BUTTONS 2
#define VERTICAL_POSITION_OF_VIBE_BUTTONS 3
#define VERTICAL_POSITION_OF_PROGRESS_BAR 4
#define VERTICAL_POSITION_OF_SEQUENCER 5

#define DLL_EXPORT __attribute__((visibility("default")))

#define FONT_SIZE 80
#define RECTANGLE_LINES_SIZE 5

#define BUTTON_FRACT_HEIGHT (SCALE_NOTE_COUNT + 2)

#define BG_COLOR ((Color){32,32,32,255})

#define INACTIVE_BG_COLOR ((Color){0,64,128,255})
#define INACTIVE_TEXT_COLOR ((Color){128,128,128,255})

#define ACTIVE_BG_COLOR ((Color){0,128,255,255})
#define ACTIVE_TEXT_COLOR ((Color){255,255,255,255})

#define SLIDER_FILLED_COLOR ((Color){0,128,255,255})
#define SLIDER_UNFILLED_COLOR ((Color){0,64,128,255})

#define CHAR_SHARP             0x266F  // ♯
#define CHAR_FLAT              0x266D  // ♭
#define CHAR_NATURAL           0x266E  // ♮
#define CHAR_DIMINISHED        0x00B0  // °
#define CHAR_HALF_DIMINISHED   0x00F8  // ø
#define CHAR_NOTE              0x266A  // ♪

typedef enum Vibe {
    VIBE_BASIC,
    VIBE_POLKA,
    VIBE_SWING,
    VIBE_WALTZ,
    VIBE_TOTAL,
} Vibe;

static Raylib *raylib;
static State *state;

bool in_rectangle(Rectangle rec, Vector2 v) {
    return (
        v.x > rec.x &&
        v.y > rec.y &&
        v.x < rec.x + rec.width &&
        v.y < rec.y + rec.height
    );
}

void draw_text_in_rectangle(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x + rec.width / 2, rec.y + rec.height / 2 };
    Vector2 dimensions = raylib->measure_text_ex(state->font, text, FONT_SIZE, state->font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    raylib->draw_text_pro(state->font, text, position, origin, 0, FONT_SIZE, state->font_spacing, color);
}

Chord get_sequencer_chord(int idx) {
    Chord chord;

    int degree = state->sequencer[idx];

    chord.root = (state->scale[degree % 7] + state->scale_root) % NOTE_COUNT;
    chord.third = (state->scale[(degree + 2) % 7] + state->scale_root) % NOTE_COUNT;
    chord.fifth = (state->scale[(degree + 4) % 7] + state->scale_root) % NOTE_COUNT;

    int non_inversed_third = chord.third < chord.root ? chord.third + 12 : chord.third;
    int non_inversed_fifth = chord.fifth < chord.root ? chord.fifth + 12 : chord.fifth;

    int third_interval = non_inversed_third - chord.root;
    int fifth_interval = non_inversed_fifth - chord.root;

    switch (third_interval) {
        case 3:
            switch (fifth_interval) {
                case 7:
                    chord.type = CHORD_TYPE_MINOR;
                    break;
                case 6:
                    chord.type = CHORD_TYPE_DIMINISHED;
                    break;
                default:
                    printf("BAD CODE!\n");
                    break;
            }
            break;
        case 4:
            switch (fifth_interval) {
                case 7:
                    chord.type = CHORD_TYPE_MAJOR;
                    break;
                case 8:
                    chord.type = CHORD_TYPE_AUGMENTED;
                    break;
                default:
                    printf("BAD CODE!\n");
                    break;
            }
            break;
            break;
        default:
            printf("BAD CODE!\n");
            break;
    }

    return chord;
}

void str_copy(char *dst, char *src) {
    int i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

float get_button_height() {
    return raylib->get_screen_height() / BUTTON_FRACT_HEIGHT;
}

Rectangle get_volume_slider_rectangle() {
    float button_height = get_button_height();
    return (Rectangle) {
        .x = 0,
        .y = button_height * VERTICAL_POSITION_OF_VOLUME_SLIDER,
        .width = raylib->get_screen_width(),
        .height = button_height,
    };
}

Rectangle get_interval_slider_rectangle() {
    float button_height = get_button_height();
    return (Rectangle) {
        .x = 0,
        .y = button_height * VERTICAL_POSITION_OF_INTERVAL_SLIDER,
        .width = raylib->get_screen_width(),
        .height = button_height,
    };
}

Rectangle get_scale_root_button_rectangle() {
    float button_width = raylib->get_screen_width() / 8;
    float button_height = get_button_height();
    return (Rectangle) {
        .x = 0,
        .y = button_height * VERTICAL_POSITION_OF_SCALE_BUTTONS,
        .width = button_width * 2,
        .height = button_height,
    };
}

Rectangle get_scale_button_rectangle() {
    Rectangle scale_root_rec = get_scale_root_button_rectangle();
    return (Rectangle) {
        .x = scale_root_rec.x + scale_root_rec.width,
        .y = scale_root_rec.y,
        .width = raylib->get_screen_width() - scale_root_rec.width,
        .height = scale_root_rec.height,
    };
}

Rectangle get_vibe_button_rectangle(int vibe) {
    float button_width = raylib->get_screen_width() / VIBE_TOTAL;
    float button_height = get_button_height();
    return (Rectangle) {
        .x = button_width * vibe,
        .y = button_height * VERTICAL_POSITION_OF_VIBE_BUTTONS,
        .width = button_width,
        .height = button_height,
    };
}

Rectangle get_progress_bar_rectangle() {
    float button_height = get_button_height();
    return (Rectangle) {
        .x = 0,
        .y = button_height * VERTICAL_POSITION_OF_PROGRESS_BAR,
        .width = raylib->get_screen_width(),
        .height = button_height,
    };
}

Rectangle get_sequencer_rectangle() {
    float button_height = get_button_height();
    return (Rectangle) {
        .x = 0,
        .y = button_height * VERTICAL_POSITION_OF_SEQUENCER,
        .width = raylib->get_screen_width(),
        .height = button_height,
    };
}

void draw_scale_buttons() {
    Rectangle scale_root_rec = get_scale_root_button_rectangle();

    char scale_root_text[4];
    switch (state->scale_root) {
        case NOTE_A: str_copy(scale_root_text, "A"); break;
        case NOTE_A_SHARP: str_copy(scale_root_text, "A♯"); break;
        case NOTE_B: str_copy(scale_root_text, "B"); break;
        case NOTE_C: str_copy(scale_root_text, "C"); break;
        case NOTE_C_SHARP: str_copy(scale_root_text, "C♯"); break;
        case NOTE_D: str_copy(scale_root_text, "D"); break;
        case NOTE_D_SHARP: str_copy(scale_root_text, "D♯"); break;
        case NOTE_E: str_copy(scale_root_text, "E"); break;
        case NOTE_F: str_copy(scale_root_text, "F"); break;
        case NOTE_F_SHARP: str_copy(scale_root_text, "F♯"); break;
        case NOTE_G: str_copy(scale_root_text, "G"); break;
        case NOTE_G_SHARP: str_copy(scale_root_text, "G♯"); break;
    }

    raylib->draw_rectangle_rec(scale_root_rec, ACTIVE_BG_COLOR);
    raylib->draw_rectangle_lines_ex(scale_root_rec, RECTANGLE_LINES_SIZE, ACTIVE_TEXT_COLOR);
    draw_text_in_rectangle(scale_root_rec, scale_root_text, ACTIVE_TEXT_COLOR);

    Rectangle scale_rec = get_scale_button_rectangle();

    char scale_text[16];
    switch (state->scale_type) {
        case SCALE_TYPE_MAJOR: str_copy(scale_text, "Major"); break;
        case SCALE_TYPE_DORIAN: str_copy(scale_text, "Dorian"); break;
        case SCALE_TYPE_PHRYGIAN: str_copy(scale_text, "Phrygian"); break;
        case SCALE_TYPE_LYDIAN: str_copy(scale_text, "Lydian"); break;
        case SCALE_TYPE_MIXOLYDIAN: str_copy(scale_text, "Mixolydian"); break;
        case SCALE_TYPE_MINOR: str_copy(scale_text, "Minor"); break;
        case SCALE_TYPE_LOCRIAN: str_copy(scale_text, "Locrian"); break;
        case SCALE_TYPE_HARMONIC_MINOR: str_copy(scale_text, "Harmonic Minor"); break;
        case SCALE_TYPE_MELODIC_MINOR: str_copy(scale_text, "Melodic Minor"); break;
    }

    raylib->draw_rectangle_rec(scale_rec, ACTIVE_BG_COLOR);
    raylib->draw_rectangle_lines_ex(scale_rec, RECTANGLE_LINES_SIZE, ACTIVE_TEXT_COLOR);
    draw_text_in_rectangle(scale_rec, scale_text, ACTIVE_TEXT_COLOR);
}

void draw_volume_slider() {
    Rectangle rec = get_volume_slider_rectangle();
    float fill = rec.width * state->volume_manual;
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, SLIDER_FILLED_COLOR);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.x + rec.width - fill, rec.height, SLIDER_UNFILLED_COLOR);
    raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, ACTIVE_TEXT_COLOR);
    const char *text = raylib->text_format("Volume: %.2f%%", state->volume_manual * 100);
    draw_text_in_rectangle(rec, text, ACTIVE_TEXT_COLOR);
}

void draw_interval_slider() {
    Rectangle rec = get_interval_slider_rectangle();
    float fill = rec.width * ((float)state->chord_interval / 10.0f);
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, SLIDER_FILLED_COLOR);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, SLIDER_UNFILLED_COLOR);
    raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, ACTIVE_TEXT_COLOR);
    const char *text = raylib->text_format("Interval: %.1f sec", state->chord_interval);
    draw_text_in_rectangle(rec, text, ACTIVE_TEXT_COLOR);
}

float get_sequencer_button_width() {
    return raylib->get_screen_width() / SEQUENCER_AMOUNT;
}

void draw_sequencer() {
    Rectangle rec = get_sequencer_rectangle();
    float w = get_sequencer_button_width();
    rec.width = w;

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        rec.x = w * i;

        raylib->draw_rectangle_rec(rec, BLACK);

        const char *text;
        Color text_color;

        switch (state->sequencer[i]) {
            case 0:
                text = "I";
                text_color = (Color){255, 51, 51, 255};
                break;
            case 1:
                text = "II";
                text_color = (Color){255, 153, 51, 255};
                break;
            case 2:
                text = "III";
                text_color = (Color){255, 255, 51, 255};
                break;
            case 3:
                text = "IV";
                text_color = (Color){51, 255, 51, 255};
                break;
            case 4:
                text = "V";
                text_color = (Color){51, 204, 255, 255};
                break;
            case 5:
                text = "VI";
                text_color = (Color){102, 102, 255, 255};
                break;
            case 6:
                text = "VII";
                text_color = (Color){204, 51, 255, 255};
                break;
        }

        Chord chord = get_sequencer_chord(i);

        switch (chord.type) {
            default:
            case CHORD_TYPE_MAJOR:
                // default text already written as if major
                break;
            case CHORD_TYPE_MINOR:
                text = raylib->text_to_lower(text);
                break;
            case CHORD_TYPE_DIMINISHED:
                text = raylib->text_to_lower(text);
                text = raylib->text_format("%s°", text);
                break;
            case CHORD_TYPE_AUGMENTED:
                text = raylib->text_format("%s+", text);
                break;
        }

        raylib->draw_rectangle_rec(rec, BLACK);
        raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, text_color);

        draw_text_in_rectangle(rec, text, text_color);

        if (i == state->current_chord_idx) {
            Vector2 v1 = { rec.x + (w * 0.5f), rec.y + (rec.height * 1.25f) };
            Vector2 v2 = { rec.x + (w * 0.25f), rec.y + (rec.height * 1.75f) };
            Vector2 v3 = { rec.x + (w * 0.75f), v2.y };
            raylib->draw_triangle(v1, v2, v3, text_color);
        }
    }
}

void draw_vibe_buttons() {
    char text[8];
    for (int i = 0; i < VIBE_TOTAL; i++) {
        switch (i) {
            case VIBE_BASIC: str_copy(text, "Basic"); break;
            case VIBE_POLKA: str_copy(text, "Polka"); break;
            case VIBE_SWING: str_copy(text, "Swing"); break;
            case VIBE_WALTZ: str_copy(text, "Waltz"); break;
        }

        Color bg_color;
        Color text_color;

        if (i == state->vibe) {
            bg_color = ACTIVE_BG_COLOR;
            text_color = ACTIVE_TEXT_COLOR;
        } else {
            bg_color = INACTIVE_BG_COLOR;
            text_color = INACTIVE_TEXT_COLOR;
        }

        Rectangle rec = get_vibe_button_rectangle(i);

        raylib->draw_rectangle_rec(rec, bg_color);
        raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, text_color);

        draw_text_in_rectangle(rec, text, text_color);
    }
}

void draw_progress_bar() {
    Rectangle rec = get_progress_bar_rectangle();
    float fill = rec.width * (state->chord_timer / state->chord_interval);
    raylib->draw_rectangle(rec.x, rec.y, fill, rec.height, SLIDER_FILLED_COLOR);
    raylib->draw_rectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, SLIDER_UNFILLED_COLOR);
    raylib->draw_rectangle_lines_ex(rec, RECTANGLE_LINES_SIZE, ACTIVE_TEXT_COLOR);
}

float note_to_freq(uint8 note, int octave) {
    int semitone_index = note + (octave - 4) * 12;
    return 440.0f * powf(2.0f, semitone_index / 12.0f);
}

#define FREQ_COUNT 4
void chord_synthesizer(void *buffer, unsigned int frames) {
    float freq[FREQ_COUNT];
    {
        int r = state->current_chord.root;
        int t = state->current_chord.third;
        int f = state->current_chord.fifth;

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

    float vibes_per_chord;

    switch (state->vibe) {
        case VIBE_BASIC: vibes_per_chord = 1.0f; break;
        case VIBE_POLKA: vibes_per_chord = 4.0f; break;
        case VIBE_SWING: vibes_per_chord = 4.0f; break;
        case VIBE_WALTZ: vibes_per_chord = 2.0f; break;
    }

    float range = state->chord_interval / vibes_per_chord;
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
    for (int i = 0; i < SCALE_NOTE_COUNT; i++) {
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

DLL_EXPORT void init(Raylib *_raylib, State *_state) {
    raylib = _raylib;
    state = _state;

    state->chord_interval = 5.0f;
    state->volume_manual = 0.5f;
    state->scale_root = NOTE_C;
    state->scale_root = SCALE_TYPE_MAJOR;
    refresh_scale();

    state->sequencer[0] = 0;
    state->sequencer[1] = 0;
    state->sequencer[2] = 0;
    state->sequencer[3] = 0;
    state->sequencer[4] = 3;
    state->sequencer[5] = 3;
    state->sequencer[6] = 4;
    state->sequencer[7] = 4;

    raylib->init_audio_device();
    raylib->set_audio_stream_buffer_size_default(4096);
    state->audio_stream = raylib->load_audio_stream(44100, 16, 1);

    int asciiStart = 32;
    int asciiEnd = 126;
    int asciiCount = asciiEnd - asciiStart + 1;

    int extraSymbols[] = {
        CHAR_SHARP,
        CHAR_FLAT,
        CHAR_NATURAL,
        CHAR_DIMINISHED,
        CHAR_HALF_DIMINISHED
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

    state->current_chord_idx = 0;
    state->current_chord = get_sequencer_chord(state->current_chord_idx);
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
            state->volume_manual = (mouse_position.x + rec.x) / rec.width;
        }
    }

    state->chord_timer += raylib->get_frame_time();
    if (state->chord_timer > state->chord_interval) {
        state->current_chord_idx = (state->current_chord_idx + 1) % SEQUENCER_AMOUNT;
        state->current_chord = get_sequencer_chord(state->current_chord_idx);
        state->chord_timer = 0.0f;
    }

    {
        Rectangle rec = get_interval_slider_rectangle(state);
        if (raylib->is_mouse_button_down(0) && in_rectangle(rec, mouse_position)) {
            state->chord_interval = ((mouse_position.x + rec.x) / rec.width) * 10.0f;
            if (state->chord_interval <= 0.5f) {
                state->chord_interval = 0.5f;
            }
        }
    }

    if (raylib->is_mouse_button_pressed(0)) {
        for (int i = 0; i < VIBE_TOTAL; i++) {
            if (in_rectangle(get_vibe_button_rectangle(i), mouse_position)) {
                state->vibe = i;
                return;
            }
        }

        if (in_rectangle(get_sequencer_rectangle(), mouse_position)) {
            float w = get_sequencer_button_width();
            int idx = mouse_position.x / w;
            state->sequencer[idx] += 1;
            if (state->sequencer[idx] >= SCALE_NOTE_COUNT) {
                state->sequencer[idx] = 0;
            }
        }

        if (in_rectangle(get_scale_root_button_rectangle(), mouse_position)) {
            state->scale_root = (state->scale_root + 1) % NOTE_COUNT;
        }

        if (in_rectangle(get_scale_button_rectangle(), mouse_position)) {
            state->scale_type = (state->scale_type + 1) % SCALE_TYPE_COUNT;
            refresh_scale();
        }
    }
}

DLL_EXPORT void render(Raylib *raylib, State *state) {
    raylib->clear_background(BG_COLOR);

    float big_text_font_size = FONT_SIZE * 4.0f;
    float big_text_spacing = 2;
    Vector2 big_text_position = {
        raylib->get_screen_width() / 2,
        raylib->get_screen_height() / 2
    };
    Vector2 big_text_dimensions = raylib->measure_text_ex(state->font, state->current_chord.text, big_text_font_size, big_text_spacing);
    Vector2 main_text_origin = {
        (big_text_dimensions.x / 2),
        (big_text_dimensions.y / 2),
    };

    draw_volume_slider();
    draw_interval_slider();
    draw_scale_buttons();
    draw_vibe_buttons();
    draw_progress_bar();
    draw_sequencer();
}

DLL_EXPORT void cleanup(Raylib *raylib, State *state) {
    raylib->unload_audio_stream(state->audio_stream);
    raylib->close_audio_device();
}

