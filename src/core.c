void init() {
#ifndef DEBUG
    SetTraceLogLevel(LOG_WARNING);
#endif
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 800, WINDOW_NAME);
    SetTargetFPS(60);

    state = (State *)calloc(1, sizeof(State));
    state->state = STATE_MAIN;
    state->vibe = VIBE_POLKA;
    state->vibes_per_chord = 4;
    refresh_time_per_chord_range();
    state->time_per_chord = get_centralized_time_per_chord();
    state->volume_manual = 0.5f;
    state->scale_root = NOTE_C;
    state->scale_type = SCALE_TYPE_MAJOR;

    refresh_scale();

    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
        state->sequencer_states[i] = false;
        for (int j = 0; j < SEQUENCER_ROW; j++) {
            state->sequencer[(i * SEQUENCER_ROW) + j] = SCALE_DEGREE_NONE;
        }
    }

    int ascii_start = 32;
    int ascii_end = 126;
    int ascii_count = ascii_end - ascii_start + 1;

    int extra_symbols[] = {
        CHAR_SINGLE_8TH_NOTE,
        CHAR_BEAMED_8TH_NOTES,
        CHAR_SHARP,
        CHAR_FLAT,
        CHAR_NATURAL,
        CHAR_DIMINISHED,
        CHAR_PLAY,
    };

    int extra_symbol_count = sizeof(extra_symbols) / sizeof(extra_symbols[0]);
    int totalCount = ascii_count + extra_symbol_count;

    int all_chars[totalCount];
    for (int i = 0; i < ascii_count; i++) {
        all_chars[i] = ascii_start + i;
    }

    for (int i = 0; i < extra_symbol_count; i++) {
        all_chars[ascii_count + i] = extra_symbols[i];
    }

    state->font = LoadFontEx("DejaVuSans.ttf", 200, all_chars, totalCount);
    state->font_spacing = 2;

    state->chord_idx = 0;

    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(4096);
    state->audio_stream = LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(state->audio_stream, chord_synthesizer);
    PlayAudioStream(state->audio_stream);
}

void update() {
    state->mouse_position = GetMousePosition();

    // TODO: volume fade is not really doing anything relevant
    // was it not supposed to fade on start and stop?
    if (state->volume_fade < 1.0f) {
        state->volume_fade += GetFrameTime() * 10.0f;
        if (state->volume_fade > 1.0f) {
            state->volume_fade = 1.0f;
        }
    }

    {
        Rectangle rec = get_volume_slider_rectangle(state);
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && in_rectangle(rec, state->mouse_position)) {
            state->volume_manual = (state->mouse_position.x - rec.x) / rec.width;
        }
    }

    float vibes_per_chord = state->vibes_per_chord;
    {
        Rectangle rec = get_interval_slider_rectangle(state);
        if (IsMouseButtonDown(0) && in_rectangle(rec, state->mouse_position)) {
            float offset = state->min_time_per_chord;
            float offset_mouse = (state->mouse_position.x - rec.x - offset) / rec.width;
            state->time_per_chord = offset + (offset_mouse * get_time_per_chord_range());
        }
    }

    if (IsMouseButtonPressed(0)) {
        switch (state->state) {
            case STATE_MAIN: {
                if (in_rectangle(get_sequencer_state_section_rectangle(), state->mouse_position)) {
                    for (int i = 0; i < SEQUENCER_AMOUNT; i++) {
                        if (in_rectangle(get_sequencer_state_rectangle(i), state->mouse_position)) {
                            state->sequencer_states[i] = !state->sequencer_states[i];
                            break;
                        }
                    }
                } else if (in_rectangle(get_sequencer_rectangle(), state->mouse_position)) {
                    for (int i = 0; i < SEQUENCER_ELEMENTS; i++) {
                        if (in_rectangle(get_sequencer_element_rectangle(i), state->mouse_position)) {
                            Rectangle rec = get_sequencer_element_rectangle(i);
                            if (in_rectangle(get_sequencer_element_section_rectangle(rec, SEQUENCER_ELEMENT_SECTION_BUTTON), state->mouse_position)) {
                                prepare_select_state(SELECTABLE_TYPE_SCALE_DEGREE, state->mouse_position, &(state->sequencer[i]));
                            } else if (in_rectangle(get_sequencer_element_section_rectangle(rec, SEQUENCER_ELEMENT_SECTION_CURSOR), state->mouse_position)) {
                                state->chord_idx = i;
                            }
                            break;
                        }
                    }
                } else if (in_rectangle(get_progress_bar_rectangle(), state->mouse_position)) {
                    progress();
                } else if (in_rectangle(get_vibe_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBE, state->mouse_position, &(state->vibe));
                } else if (in_rectangle(get_vibes_per_chord_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_VIBES_PER_CHORD, state->mouse_position, &(state->vibes_per_chord));
                } else if (in_rectangle(get_scale_root_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_ROOT, state->mouse_position, &(state->scale_root));
                } else if (in_rectangle(get_scale_accidental_button_rectangle(), state->mouse_position)) {
                    toggle_flag(FLAG_FLATS);
                } else if (in_rectangle(get_scale_button_rectangle(), state->mouse_position)) {
                    prepare_select_state(SELECTABLE_TYPE_SCALE_TYPE, state->mouse_position, &(state->scale_root));
                } else if (in_rectangle(get_play_pause_button_rectangle(), state->mouse_position)) {
                    toggle_flag(FLAG_PLAYING);
                    if (has_flag(FLAG_PLAYING)) {
                        if (state->sequencer[state->chord_idx] == SCALE_DEGREE_NONE) {
                            progress();
                        }
                    } else {
                        state->chord_timer = 0.0f;
                    }
                }
            } break;
            case STATE_SELECT: {
                if (in_rectangle(state->selectables.rectangle, state->mouse_position)) {
                    int item_idx = (state->mouse_position.y - state->selectables.rectangle.y) / get_selectable_item_height();
                    switch (state->selectables.type) {
                        default: {
                            *(state->selectables.reference) = item_idx;
                        } break;
                        case SELECTABLE_TYPE_SCALE_TYPE: {
                            state->scale_type = item_idx;
                            refresh_scale();
                        } break;
                        case SELECTABLE_TYPE_VIBES_PER_CHORD: {
                            float prev_vibes_per_chord = (float)state->vibes_per_chord;
                            state->vibes_per_chord = pow(2, item_idx);
                            refresh_time_per_chord_range();
                            float multiplier = state->vibes_per_chord / prev_vibes_per_chord;
                            float new_time_per_chord = state->time_per_chord * multiplier;
                            set_time_per_chord(new_time_per_chord);
                        } break;
                        case SELECTABLE_TYPE_VIBE: {
                            float old_range = get_time_per_chord_range();
                            float old_location = (state->time_per_chord - state->min_time_per_chord) / old_range;
                            state->vibe = item_idx;
                            refresh_time_per_chord_range();
                            float new_range = get_time_per_chord_range();
                            float new_time_per_chord = state->min_time_per_chord + old_location * new_range;
                            set_time_per_chord(new_time_per_chord);
                        } break;
                    }
                }
                state->state = STATE_MAIN;
            } break;
        }
    }
}

void render() {
    BeginDrawing();

    ClearBackground(BG_COLOR);

    draw_volume_slider();

    draw_sequencer();

    draw_play_pause_button();
    draw_progress_bar();

    draw_scale_root_button();
    draw_scale_accidental_button();
    draw_scale_button();

    draw_vibe_button();
    draw_vibes_per_chord_button();
    draw_interval_slider();

    if (state->state == STATE_SELECT) {
        draw_selectables();
    }

    EndDrawing();
}

void cleanup() {
    UnloadAudioStream(state->audio_stream);
    CloseAudioDevice();
    free(state);
    CloseWindow();
}
