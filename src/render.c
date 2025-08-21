inline static void draw_rectangle_lines(Rectangle rec) {
    DrawRectangleLinesEx(rec, rectangle_lines_size(), RECTANGLE_LINES_COLOR);
}

void draw_text_in_rectangle(Rectangle rec, const char *text, Color color) {
    Vector2 position = { rec.x + rec.width / 2, rec.y + rec.height / 2 };
    Vector2 dimensions = MeasureTextEx(state->font, text, font_size(), state->font_spacing);
    Vector2 origin = { dimensions.x / 2, dimensions.y / 2 };
    DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, color);
}

void draw_volume_slider() {
    Rectangle rec = get_volume_slider_rectangle();
    float fill = rec.width * state->volume_manual;
    Color filled_color;
    Color unfilled_color;
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED);
    DrawRectangle(rec.x + fill, rec.y, rec.x + rec.width - fill, rec.height, COLOR_UNFILLED);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("Volume: %.2f%%", state->volume_manual * 100), TEXT_COLOR_THING);
}

void draw_vibes_per_chord_button() {
    Rectangle rec = get_vibes_per_chord_rectangle();
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("%d/chord", state->vibes_per_chord), TEXT_COLOR_THING);
}

void draw_interval_slider() {
    Rectangle rec = get_interval_slider_rectangle();
    float fill = rec.width * ((float)state->time_per_chord / get_time_per_chord_max());
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED);
    DrawRectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, TextFormat("Interval: %.1f sec", state->time_per_chord), TEXT_COLOR_THING);
}

void draw_vibe_button() {
    Rectangle rec = get_vibe_button_rectangle();
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, get_vibe_name(state->vibe), TEXT_COLOR_THING);
}

void draw_play_pause_button() {
    Rectangle rec = get_play_pause_button_rectangle();
    if (has_flag(FLAG_PLAYING)) {
        DrawRectangleRec(rec, COLOR_FILLED_PROGRESS_BAR);
        draw_text_in_rectangle(rec, "PLAYING", TEXT_COLOR_THING);
    } else {
        DrawRectangleRec(rec, COLOR_UNFILLED_PROGRESS_BAR);
        draw_text_in_rectangle(rec, "PAUSED", TEXT_COLOR_THING);
    }
    draw_rectangle_lines(rec);
}

void draw_progress_bar() {
    Rectangle rec = get_progress_bar_rectangle();
    float fill = rec.width * (state->chord_timer / state->time_per_chord);
    DrawRectangle(rec.x, rec.y, fill, rec.height, COLOR_FILLED_PROGRESS_BAR);
    DrawRectangle(rec.x + fill, rec.y, rec.width - fill, rec.height, COLOR_UNFILLED_PROGRESS_BAR);
    draw_rectangle_lines(rec);
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
        if (in_rectangle(rec, state->mouse_position)) {
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
        DrawTextPro(state->font, text, position, origin, 0, font_size(), state->font_spacing, TEXT_COLOR_THING);
    }
}

void draw_scale_root_button() {
    Rectangle rec = get_scale_root_button_rectangle();
    const char *text = get_note_name(state->scale_root);
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
}

void draw_scale_accidental_button() {
    Rectangle rec = get_scale_accidental_button_rectangle();

    const char *text = has_flag(FLAG_FLATS) ? "Flats" : "Sharps";

    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
}

void draw_scale_button() {
    Rectangle rec = get_scale_button_rectangle();
    const char *text = get_scale_name(state->scale_type);
    DrawRectangleRec(rec, BG_COLOR_THING);
    draw_rectangle_lines(rec);
    draw_text_in_rectangle(rec, text, TEXT_COLOR_THING);
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

        draw_rectangle_lines(sequencer_row_rec);

        const char *state_text;
        Color state_text_color;
        if (state->sequencer_states[i]) {
            state_text = "ON";
            state_text_color = (Color){0,255,0,255};
        } else {
            state_text = "OFF";
            state_text_color = (Color){255,0,0,255};
        }

        draw_rectangle_lines(state_rec);
        draw_text_in_rectangle(state_rec, state_text, state_text_color);

        for (int j = 0; j < SEQUENCER_ROW; j++) {
            int element_idx = (i * SEQUENCER_ROW) + j;

            Rectangle element_rec = get_sequencer_element_rectangle(element_idx);

            Chord chord = get_sequencer_chord(state->sequencer[element_idx]);
            bool enabled = chord.type != CHORD_TYPE_NONE;

            Rectangle chord_symbol_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CHORD_SYMBOL);
            const char *chord_symbol = enabled ? chord.symbol : "---";
            draw_text_in_rectangle(chord_symbol_rec, chord_symbol, TEXT_COLOR_THING);

            Rectangle button_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_BUTTON);
            const char *button_text = enabled ? chord.roman : "off";
            Color button_text_color = (Color){64,64,64,255};
            if (state->sequencer_states[i]) {
                switch (state->sequencer[element_idx]) {
                    case SCALE_DEGREE_I: button_text_color = (Color){255, 0, 0, 255}; break;
                    case SCALE_DEGREE_II: button_text_color = (Color){255, 128, 0, 255}; break;
                    case SCALE_DEGREE_III: button_text_color = (Color){255, 255, 0, 255}; break;
                    case SCALE_DEGREE_IV: button_text_color = (Color){0, 255, 0, 255}; break;
                    case SCALE_DEGREE_V: button_text_color = (Color){0, 128, 255, 255}; break;
                    case SCALE_DEGREE_VI: button_text_color = (Color){128, 0, 255, 255}; break;
                    case SCALE_DEGREE_VII: button_text_color = (Color){255, 0, 255, 255}; break;
                }
            }
            Color button_bg_color = (Color){
                button_text_color.r / 4,
                button_text_color.g / 4,
                button_text_color.b / 4,
                255
            };
            DrawRectangleRec(button_rec, button_bg_color);
            draw_rectangle_lines(button_rec);
            draw_text_in_rectangle(button_rec, button_text, button_text_color);

            Rectangle cursor_rec = get_sequencer_element_section_rectangle(element_rec, SEQUENCER_ELEMENT_SECTION_CURSOR);
            if (element_idx == state->chord_idx) {
                Rectangle outline = button_rec;
                outline.x += rectangle_lines_size();
                outline.y += rectangle_lines_size();
                outline.width -= rectangle_lines_size() * 2;
                outline.height -= rectangle_lines_size() * 2;

                DrawRectangleLinesEx(outline, rectangle_lines_size(), button_text_color);
                draw_text_in_rectangle(cursor_rec, "▶", button_text_color);
            }
        }
    }
}
