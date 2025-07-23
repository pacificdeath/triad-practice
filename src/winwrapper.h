#ifndef WINWRAPPER_H
#define WINWRAPPER_H

void win_copy_dll();
void *win_get_function_address(char *function_name);
void *win_load_dll();
void win_free_dll();
void win_update_dll_write_time();
int win_is_dll_updated();

#endif

