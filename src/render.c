void draw_separator(Rectangle rec) {
    Vector2 start = { rec.x, rec.y + rec.height };
    Vector2 end = { rec.x + rec.width, start.y };
    DrawLineEx(start, end, size_multiplier() * 0.002f, TP_BG2);
}

void draw_text_in_rectangle(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x + rec.width / 2, rec.y + rec.height / 2 };
    Vector2 dimensions = MeasureTextEx(state->font, text, font_size(), state->font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, color);
}

void draw_text_in_rectangle_fixed_x(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x, rec.y + rec.height / 2 };
    Vector2 dimensions = MeasureTextEx(state->font, text, font_size(), state->font_spacing);
    Vector2 origin = { -(0.01f * size_multiplier()), dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, color);
}

void draw_load_file_button() {
    Rectangle rec = get_load_file_rectangle();
    DrawRectangleRec(rec, TP_GREEN);
    draw_text_in_rectangle(rec, "Load file", TP_FG);
}

void draw_save_file_button() {
    Rectangle rec = get_save_file_rectangle();
    DrawRectangleRec(rec, TP_RED);
    draw_text_in_rectangle(rec, "Save file", TP_FG);
}

void draw_play_control() {
    Rectangle rec = get_play_button_rectangle();
    if (has_flag(FLAG_PLAYING)) {
        DrawRectangleRec(rec, TP_GREEN);
        draw_text_in_rectangle(rec, "Playing", TP_FG);
    } else {
        DrawRectangleRec(rec, TP_RED);
        draw_text_in_rectangle(rec, "Paused", TP_FG);
    }
}

void draw_control(Rectangle rec, const char *text) {
    DrawRectangleRec(rec, TP_BG2);
    draw_text_in_rectangle_fixed_x(rec, text, TP_FG);
}

void draw_toggle_control(Rectangle rec, bool condition, const char *text_true, const char *text_false) {
    DrawRectangleRec(rec, TP_BG2);
    if (condition) {
        draw_text_in_rectangle_fixed_x(rec, text_true, TP_GREEN);
    } else {
        draw_text_in_rectangle_fixed_x(rec, text_false, TP_RED);
    }
}

void draw_slider_control(Rectangle rec, float min, float max, float value) {
    float range = max - min;
    float offset = min / range;
    float fill = ((value / range) - offset) * rec.width;
    DrawRectangle(rec.x, rec.y, fill, rec.height, TP_GREEN);
    DrawRectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, TP_BG2);
}

void draw_controls() {
    for (int i = 0; i < CONTROLS_COUNT; i++) {
        Rectangle label_rec = get_control_label_rectangle(i);
        Rectangle value_rec = get_control_value_rectangle(i);

        const char *label_text;

        switch (i) {
            case CONTROLS_SCALE: {
                label_text = "Scale";
                draw_control(value_rec, get_scale_name(state->scale_type));
            } break;
            case CONTROLS_ROOT_NOTE: {
                label_text = "Root Note";
                draw_control(value_rec, get_note_name(state->scale_root));
            } break;
            case CONTROLS_VIBE: {
                label_text = "Vibe";
                draw_control(value_rec, get_vibe_name(state->vibe));
            } break;
            case CONTROLS_VIBES_PER_CHORD: {
                label_text = "Vibes/chord";
                draw_control(value_rec, TextFormat("%d", state->vibes_per_chord));
            } break;
            case CONTROLS_ACCIDENTAL: {
                label_text = "Accidentals";
                draw_toggle_control(value_rec, !has_flag(FLAG_FLATS), "Sharps(♯)", "Flats(♭)");
            } break;
            case CONTROLS_VOLUME: {
                label_text = "Volume";
                draw_slider_control(value_rec, 0.0f, 1.0f, state->volume_manual);
            } break;
            case CONTROLS_INTERVAL: {
                label_text = "Time/chord";
                draw_slider_control(value_rec, state->min_time_per_chord, state->max_time_per_chord, state->time_per_chord);
            } break;
        }

        DrawRectangleRec(label_rec, TP_BG);
        draw_text_in_rectangle_fixed_x(label_rec, label_text, TP_FG);

        draw_separator(label_rec);
        draw_separator(value_rec);
    }
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

        Rectangle reset_button_rec = get_sequencer_section_rectangle(state_rec, SEQUENCER_STATE_SECTION_RESET);
        draw_text_in_rectangle(reset_button_rec, "RESET", TP_FG);

        const char *state_text;
        Color state_bg;
        if (state->sequencer_states[i]) {
            state_text = "ON";
            state_bg = TP_GREEN;
        } else {
            state_text = "OFF";
            state_bg = TP_RED;
        }

        Rectangle state_button_rec = get_sequencer_section_rectangle(state_rec, SEQUENCER_STATE_SECTION_ENABLE);
        DrawRectangleRec(state_button_rec, state_bg);
        draw_text_in_rectangle(state_button_rec, state_text, TP_FG);

        Vector2 line_start = {state_button_rec.x + state_button_rec.width, state_button_rec.y + (state_button_rec.height / 2) };
        Vector2 line_end = {sequencer_row_rec.x + sequencer_row_rec.width, line_start.y };
        DrawLineEx(line_start, line_end, size_multiplier() * 0.004f, state_bg);

        for (int j = 0; j < SEQUENCER_ROW; j++) {
            int element_idx = (i * SEQUENCER_ROW) + j;

            Rectangle element_rec = get_sequencer_element_rectangle(element_idx);

            Chord chord = get_sequencer_chord(state->sequencer[element_idx]);
            bool is_enabled = chord.type != CHORD_TYPE_NONE;

            Rectangle chord_symbol_rec = get_sequencer_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL);
            const char *chord_symbol = is_enabled ? chord.symbol : "---";
            draw_text_in_rectangle(chord_symbol_rec, chord_symbol, TP_FG);

            Rectangle button_rec = get_sequencer_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_BUTTON);

            const char *button_text = is_enabled ? chord.roman : "off";
            Color button_bg = TP_BG2;
            if (is_enabled) {
                if (state->sequencer_states[i]) {
                    button_bg = TP_GREEN;
                } else if (is_enabled) {
                    button_bg = TP_RED;
                }
            }

            DrawRectangleRec(button_rec, button_bg);
            draw_text_in_rectangle(button_rec, button_text, TP_FG);
            Rectangle cursor_rec = get_sequencer_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CURSOR);

            if (element_idx == state->chord_idx) {
                float cursor_offset = ((state->chord_timer / state->time_per_chord) * button_rec.width);
                float cursor_size = cursor_rec.height / 4;

                Vector2 v1 = { cursor_rec.x + cursor_offset, cursor_rec.y };
                Vector2 v2 = { v1.x, v1.y + cursor_rec.height };
                Vector2 v3 = { v1.x + cursor_size, v1.y + cursor_rec.height / 2 };

                DrawTriangle(v1, v2, v3, state_bg);
            }
        }

        draw_separator(sequencer_row_rec);
    }
}

void draw_cmd() {
    Rectangle rec = get_cmd_rectangle();
    DrawRectangleRec(rec, TP_BG2);
    switch (state->state) {
        case STATE_MAIN: {
            draw_text_in_rectangle_fixed_x(
                rec,
                TextFormat(
                    "sequencer: %i:%i (%.2fs/%.2fs)",
                    1 + (state->chord_idx / SEQUENCER_ROW),
                    1 + (state->chord_idx % SEQUENCER_ROW),
                    state->chord_timer,
                    state->time_per_chord
                ),
                TP_FG
            );
        } break;
        case STATE_SAVE_FILE: {
            draw_text_in_rectangle_fixed_x(rec, TextFormat("save to file: \"%s\"", state->cmd_buffer), TP_FG);
        }
    }
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
        if (mouse_in_rectangle(rec)) {
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
        DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, TP_FG);
    }
}
