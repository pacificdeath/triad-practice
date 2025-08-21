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

