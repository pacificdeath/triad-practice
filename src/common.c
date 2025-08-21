inline static float size_multiplier() {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    return (width < height) ? width : height;
}

inline static float font_size() {
    return 0.04f * size_multiplier();
}

inline static int rectangle_lines_size() {
    return 0.004f * size_multiplier();
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

bool in_rectangle(Rectangle rec, Vector2 v) {
    return (
        v.x > rec.x &&
        v.y > rec.y &&
        v.x < rec.x + rec.width &&
        v.y < rec.y + rec.height
    );
}

