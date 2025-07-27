#include <stdlib.h>
#include "../raylib/include/raylib.h"
#include "engine.h"
#include "winwrapper.h"

#define WINDOW_NAME "Death"
#define DLL_INIT_FUNCTION "init"
#define DLL_ON_LOAD_FUNCTION "on_load"
#define DLL_ON_UNLOAD_FUNCTION "on_unload"
#define DLL_UPDATE_FUNCTION "update"
#define DLL_RENDER_FUNCTION "render"
#define DLL_CLEANUP_FUNCTION "cleanup"

typedef void (*DllFunction)(Raylib*,State*);

typedef struct DllData {
    void *dll;
    DllFunction on_load_function;
    DllFunction on_unload_function;
    DllFunction init_function;
    DllFunction update_function;
    DllFunction render_function;
    DllFunction cleanup_function;
} DllData;

static void dll_function_stub(Raylib *raylib, State *state){};

static DllData dll_data = {0};

static void try_map_function(DllFunction *function, char *dll_function_name) {
    (*function) = win_get_function_address(dll_function_name);
    if ((*function) == NULL) {
        (*function) == dll_function_stub;
    }
}

static void load_dll() {
    win_copy_dll();
    dll_data.dll = win_load_dll();
    if (dll_data.dll != NULL) {
        try_map_function(&(dll_data.init_function), DLL_INIT_FUNCTION);
        try_map_function(&(dll_data.on_load_function), DLL_ON_LOAD_FUNCTION);
        try_map_function(&(dll_data.on_unload_function), DLL_ON_UNLOAD_FUNCTION);
        try_map_function(&(dll_data.update_function), DLL_UPDATE_FUNCTION);
        try_map_function(&(dll_data.render_function), DLL_RENDER_FUNCTION);
        try_map_function(&(dll_data.cleanup_function), DLL_CLEANUP_FUNCTION);
    }
}

static void unload_dll() {
    dll_data.dll = NULL;
    dll_data.init_function = dll_function_stub;
    dll_data.on_load_function = dll_function_stub;
    dll_data.on_unload_function = dll_function_stub;
    dll_data.update_function = dll_function_stub;
    dll_data.render_function = dll_function_stub;
    dll_data.cleanup_function = dll_function_stub;
    win_free_dll();
}

int main() {
    Raylib *raylib = (Raylib *)calloc(1, sizeof(Raylib));
    raylib->is_window_resized = IsWindowResized;
    raylib->get_screen_width = GetScreenWidth;
    raylib->get_screen_height = GetScreenHeight;
    raylib->get_frame_time = GetFrameTime;
    raylib->get_random_value = GetRandomValue;
    raylib->text_format = TextFormat;
    raylib->text_to_lower = TextToLower;
    raylib->text_copy = TextCopy;
    raylib->is_key_pressed = IsKeyPressed;
    raylib->is_mouse_button_pressed = IsMouseButtonPressed;
    raylib->is_mouse_button_down = IsMouseButtonDown;
    raylib->get_mouse_position = GetMousePosition;
    raylib->init_audio_device = InitAudioDevice;
    raylib->close_audio_device = CloseAudioDevice;
    raylib->load_audio_stream = LoadAudioStream;
    raylib->play_audio_stream = PlayAudioStream;
    raylib->is_audio_stream_playing = IsAudioStreamPlaying;
    raylib->stop_audio_stream = StopAudioStream;
    raylib->unload_audio_stream = UnloadAudioStream;
    raylib->set_audio_stream_callback = SetAudioStreamCallback;
    raylib->set_audio_stream_buffer_size_default = SetAudioStreamBufferSizeDefault;
    raylib->load_font_ex = LoadFontEx;
    raylib->measure_text_ex = MeasureTextEx;
    raylib->draw_text_pro = DrawTextPro;
    raylib->clear_background = ClearBackground;
    raylib->draw_rectangle = DrawRectangle;
    raylib->draw_rectangle_rec = DrawRectangleRec;
    raylib->draw_rectangle_lines_ex = DrawRectangleLinesEx;
    raylib->draw_triangle = DrawTriangle;

    State *state = (State *)calloc(1, sizeof(State));

    load_dll();

    win_update_dll_write_time();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(1200, 800, WINDOW_NAME);

    dll_data.init_function(raylib, state);

    dll_data.on_load_function(raylib, state);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (win_is_dll_updated()) {

            dll_data.on_unload_function(raylib, state);

            unload_dll();

            load_dll();
            win_update_dll_write_time();

            dll_data.on_load_function(raylib, state);
        }

        if (dll_data.dll == NULL) {
            load_dll();
        }

        dll_data.update_function(raylib, state);

        BeginDrawing();
            dll_data.render_function(raylib, state);
        EndDrawing();
    }

    dll_data.cleanup_function(raylib, state);

    CloseWindow();

    free(state);

    if (dll_data.dll != NULL) {
        win_free_dll(dll_data.dll);
    }

    return 0;
}

