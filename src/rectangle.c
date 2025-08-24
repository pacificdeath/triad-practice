Rectangle get_file_controls_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_FILE_CONTROLS,
        .width = GetScreenWidth(),
        .height = height,
    };
}

Rectangle get_load_file_rectangle() {
    Rectangle rec = get_file_controls_rectangle();
    rec.width /= 2;
    return rec;
}

Rectangle get_save_file_rectangle() {
    Rectangle rec = get_file_controls_rectangle();
    rec.x = rec.x + (rec.width / 2);
    rec.width /= 2;
    return rec;
}

Rectangle get_control_rectangle(int control) {
    float width = GetScreenWidth() / 2;
    float height = get_thing_height();
    int x_idx = control / CONTROLS_COLUMN_COUNT;
    int y_idx = control % CONTROLS_COLUMN_COUNT;
    int split_point = CONTROLS_COUNT / 2;
    return (Rectangle) {
        .x = width * x_idx,
        .y = (height * VERTICAL_POSITION_OF_CONTROLS) + (height * y_idx),
        .width = width,
        .height = height,
    };
}

Rectangle get_control_label_rectangle(int control) {
    Rectangle rec = get_control_rectangle(control);
    rec.width /= 2;
    return rec;
}

Rectangle get_control_value_rectangle(int control) {
    Rectangle rec = get_control_rectangle(control);
    rec.x = rec.x + (rec.width / 2);
    rec.width /= 2;
    return rec;
}

Rectangle get_play_button_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_PLAY_BUTTON,
        .width = GetScreenWidth(),
        .height = height,
    };
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

Rectangle get_sequencer_section_rectangle(Rectangle element_rec, int section) {
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

Rectangle get_cmd_rectangle() {
    float height = get_thing_height();
    return (Rectangle) {
        .x = 0,
        .y = height * VERTICAL_POSITION_OF_CMD,
        .width = GetScreenWidth(),
        .height = height,
    };
}

