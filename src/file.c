void cmd_save_file() {
    // oh not yet
}

void cmd_try_add_char(char c) {
    if (state->cmd_cursor < (CMD_MAX_TEXT - 2)) {
        state->cmd_buffer[state->cmd_cursor + 1] = '\0';
        state->cmd_buffer[state->cmd_cursor] = c;
        state->cmd_cursor++;
    }
}

void cmd_try_remove_char() {
    if (state->cmd_cursor > 0) {
        state->cmd_cursor--;
        state->cmd_buffer[state->cmd_cursor] = '\0';
    }
}

bool cmd_enter_file_name() {
    for (int key = KEY_A; key <= KEY_Z; key++) {
        if (IsKeyPressed(key)) {
            if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
                key += 32;
            }
            cmd_try_add_char(key);
            return false;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) {
        cmd_try_add_char(' ');
        return false;
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        cmd_try_remove_char();
        return false;
    }

    return IsKeyPressed(KEY_ENTER);
}
