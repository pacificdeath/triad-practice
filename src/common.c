inline static bool mouse_in_rectangle(Rectangle rec) {
    return (
        state->mouse_position.x > rec.x &&
        state->mouse_position.y > rec.y &&
        state->mouse_position.x < rec.x + rec.width &&
        state->mouse_position.y < rec.y + rec.height
    );
}

inline static float size_multiplier() {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    return (width < height) ? width : height;
}

inline static float font_size() {
    return 0.04f * size_multiplier();
}

inline static bool has_flag(int flag) {
    return (state->flags & flag) == flag;
}

inline static void toggle_flag(int flag) {
    state->flags ^= flag;
}

inline static float get_thing_height() {
    return GetScreenHeight() / VERTICAL_POSITION_COUNT;
}

