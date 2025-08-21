#include "main.h"
#include "debug.h"

static State *state;

#include "common.c"
#include "name.c"
#include "rectangle.c"
#include "music.c"
#include "select.c"
#include "render.c"
#include "core.c"

int main() {

    init();

    while (!WindowShouldClose()) {

        update();

        render();

    }

    cleanup();
}

