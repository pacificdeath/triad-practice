inline static float get_time_per_chord_max() {
    return TIME_PER_CHORD_MAX_MULTIPLIER * state->vibes_per_chord;
}

float note_to_freq(uint8 note, int octave) {
    int semitone_index = note + (octave - 4) * 12;
    return 440.0f * powf(2.0f, semitone_index / 12.0f);
}

int truncate_note_accidentals(int note) {
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

    int natural_scale_root = truncate_note_accidentals(state->scale_root);

    int natural_chord_root = natural_scale_root;
    for (int i = 0; i < degree; i++) {
        int old = natural_chord_root;
        int new = natural_chord_root;
        while (new == old) {
            natural_chord_root = (natural_chord_root + 1) % NOTE_COUNT;
            new = truncate_note_accidentals(natural_chord_root);
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

inline static float trapezoid_ramp(float a, float b, float x) {
    if (x <= a || x >= b) return 0.0f;

    float len = b - a;
    float t = (x - a) / len;
    float ramp = 0.1f;

    if (t < ramp) {
        return t / ramp;
    } else if (t > 1.0f - ramp) {
        return (1.0f - t) / ramp;
    } else {
        return 1.0f;
    }
}

typedef enum ChordBits {
    BIT_NONE = 0,
    BIT_1 = 1 << 0,
    BIT_3 = 1 << 1,
    BIT_5 = 1 << 2,
    BIT_5_LOW = 1 << 3,
    BIT_ALL = ~0,
} ChordBits;

typedef struct VibeStep {
    float start;
    float end;
    ChordBits bits;
} VibeStep;

#define FREQ_COUNT 4
void chord_synthesizer(void *buffer, unsigned int frames) {
    if (!has_flag(FLAG_PLAYING) || !is_sequencer_active()) {
        return;
    }

    float sample_rate = 44100.0f;

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

    float lfo_depth = 5.0f;
    float lfo_rate = 6.0f;
    static float lfo_phase = 0.0f;

    float cutoff_freq = 500.0f * state->volume_fade;
    float alpha = 1.0f / (1.0f + (sample_rate / cutoff_freq));
    static float prev_output = 0.0f;

    float incr[FREQ_COUNT] = {0};
    static float phase[FREQ_COUNT] = {0};
    short *d = (int16 *)buffer;

    float range = state->time_per_chord / state->vibes_per_chord;

    int active_tri = 0;
    float trapezoid_ramp_multiplier = 0.0f;

    int step_count = 0;
    VibeStep steps[16];
    float fract = -1.0f;

    switch (state->vibe) {
        case VIBE_POLKA: {
            fract = range / 8.0f;
            steps[step_count++] = (VibeStep){ 0.0f, 2.0f, BIT_1 };
            steps[step_count++] = (VibeStep){ 2.0f, 3.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 3.0f, 4.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 4.0f, 6.0f, BIT_5_LOW };
            steps[step_count++] = (VibeStep){ 6.0f, 7.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 7.0f, 8.0f, BIT_NONE };
            break;
        }

        case VIBE_SWING: {
            fract = range / 6.0f;
            steps[step_count++] = (VibeStep){ 0.0f, 1.0f, BIT_1 };
            steps[step_count++] = (VibeStep){ 1.0f, 2.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 2.0f, 3.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 3.0f, 4.0f, BIT_5_LOW };
            steps[step_count++] = (VibeStep){ 4.0f, 5.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 5.0f, 6.0f, BIT_3|BIT_5 };
            break;
        }

        case VIBE_WALTZ: {
            fract = range / 12.0f;
            steps[step_count++] = (VibeStep){ 0.0f, 2.0f, BIT_1 };
            steps[step_count++] = (VibeStep){ 2.0f, 3.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 3.0f, 4.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 4.0f, 5.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 5.0f, 6.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 6.0f, 8.0f, BIT_5_LOW };
            steps[step_count++] = (VibeStep){ 8.0f, 9.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 9.0f, 10.0f, BIT_NONE };
            steps[step_count++] = (VibeStep){ 10.0f, 11.0f, BIT_3|BIT_5 };
            steps[step_count++] = (VibeStep){ 11.0f, 12.0f, BIT_NONE };
            break;
        }

        case VIBE_CHORD: {
            fract = range;
            steps[step_count++] = (VibeStep){ 0.0f, 1.0f, BIT_ALL };
            break;
        }

        case VIBE_ROOT: {
            fract = range;
            steps[step_count++] = (VibeStep){ 0.0f, 1.0f, BIT_1 };
            break;
        }

        case VIBE_THIRD: {
            fract = range;
            steps[step_count++] = (VibeStep){ 0.0f, 1.0f, BIT_3 };
            break;
        }

        case VIBE_FIFTH: {
            fract = range;
            steps[step_count++] = (VibeStep){ 0.0f, 1.0f, BIT_5 };
            break;
        }
    }

    ASSERT(step_count > 0);
    ASSERT(fract >= 0.0f);

    for (uint32 i = 0; i < frames; i++) {
        float local_time = fmodf(state->chord_timer, range);

        for (size_t i = 0; i < step_count; i++) {
            VibeStep *s = &steps[i];
            float start = s->start * fract;
            float end = s->end * fract;
            if (local_time >= start && local_time < end) {
                active_tri = s->bits;
                trapezoid_ramp_multiplier = trapezoid_ramp(start, end, local_time);
                break;
            }
        }

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

        d[i] = (short)(32000.0f * filtered_sample)
            * state->volume_fade
            * state->volume_manual
            * trapezoid_ramp_multiplier;

        for (int j = 0; j < FREQ_COUNT; j++) {
            phase[j] += incr[j];
            if (phase[j] > 1.0f) phase[j] -= 1.0f;
        }

        lfo_phase += lfo_rate / sample_rate;
        if (lfo_phase > 1.0f) lfo_phase -= 1.0f;

        state->chord_timer += 1.0 / sample_rate;
        if (state->chord_timer > state->time_per_chord) {
            progress();
        }
    }
}
