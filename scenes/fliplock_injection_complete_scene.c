#include "fliplock_injection_complete_scene.h"

void fliplock_injection_complete_on_enter(void* context) {
    UNUSED(context);
}

bool fliplock_injection_complete_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_injection_complete_on_exit(void* context) {
    UNUSED(context);
}
