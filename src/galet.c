#include "galet.h"

static Window *window;

int main(void) {
    init();
    app_event_loop();
    deinit();
    return 0;
}

void init(void) {
    window = window_create();
    window_stack_push(window, false /* animated */);
}

void deinit(void) {
    window_destroy(window);
}
