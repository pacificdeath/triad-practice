#include <windows.h>
#define DLL "./build/main.dll"
#define DLL_COPY "./build/main-temp.dll"

static void *dll;
static FILETIME last_write_time;

void win_copy_dll() {
    CopyFile(DLL, DLL_COPY, FALSE);
}

void *win_get_function_address(char *function_name) {
    return GetProcAddress(dll, function_name);
}

void *win_load_dll() {
    dll = LoadLibrary(DLL_COPY);
    return dll;
}

void win_free_dll() {
    FreeLibrary(dll);
}

void win_update_dll_write_time() {
    WIN32_FILE_ATTRIBUTE_DATA file_data;
    if (GetFileAttributesEx(DLL, GetFileExInfoStandard, &file_data)) {
        last_write_time = file_data.ftLastWriteTime;
    }
}

#include <stdio.h>

int win_is_dll_updated() {
    FILETIME new_write_time = {0};
    WIN32_FILE_ATTRIBUTE_DATA file_data;
    if (GetFileAttributesEx(DLL, GetFileExInfoStandard, &file_data)) {
        new_write_time = file_data.ftLastWriteTime;
    }
    if (CompareFileTime(&new_write_time, &last_write_time) > 0) {
        return 1;
    }
    return 0;
}

