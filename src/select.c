inline static float get_selectable_item_height() {
    return get_thing_height() / 2;
}

void prepare_select_state(int selectable_type, Vector2 position, uint8 *reference) {
    state->selectables.type = selectable_type;
    state->selectables.reference = reference;
    switch (selectable_type) {
        case SELECTABLE_TYPE_SCALE_DEGREE:
            Chord chord;
            state->selectables.item_count = SCALE_DEGREE_COUNT + 1;
            for (int i = 0; i < SCALE_DEGREE_COUNT; i++) {
                Chord chord = get_sequencer_chord(i);
                const char *text = TextFormat("%s (%s)", chord.roman, chord.symbol);
                TextCopy(state->selectables.items[i], text);
            }
            TextCopy(state->selectables.items[SCALE_DEGREE_COUNT], "off");
            break;
        case SELECTABLE_TYPE_SCALE_ROOT:
            state->selectables.item_count = NOTE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_note_name(i);
                TextCopy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_SCALE_TYPE:
            state->selectables.item_count = SCALE_TYPE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                const char *text = get_scale_name(i);
                TextCopy(state->selectables.items[i], text);
            }
            break;
        case SELECTABLE_TYPE_VIBE:
            state->selectables.item_count = VIBE_COUNT;
            for (int i = 0; i < state->selectables.item_count; i++) {
                TextCopy(state->selectables.items[i], get_vibe_name(i));
            }
            break;
        case SELECTABLE_TYPE_VIBES_PER_CHORD:
            state->selectables.item_count = 5;
            TextCopy(state->selectables.items[0], "1/chord");
            TextCopy(state->selectables.items[1], "2/chord");
            TextCopy(state->selectables.items[2], "4/chord");
            TextCopy(state->selectables.items[3], "8/chord");
            TextCopy(state->selectables.items[4], "16/chord");
            break;
    }

    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    state->selectables.rectangle.width = screen_width * SELECTABLES_BOX_WIDTH_MULTIPLIER;

    if (position.x < screen_width / 2) {
        state->selectables.rectangle.x = position.x;
    } else {
        state->selectables.rectangle.x = position.x - state->selectables.rectangle.width;
    }

    state->selectables.rectangle.height = state->selectables.item_count * get_selectable_item_height();

    if (position.y < screen_height / 2) {
        state->selectables.rectangle.y = position.y;
    } else {
        state->selectables.rectangle.y = position.y - state->selectables.rectangle.height;
    }

    state->state = STATE_SELECT;
}

