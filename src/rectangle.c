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

