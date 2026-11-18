#include "fliplock_delete_confirmation_scene.h"

void fliplock_delete_confirmation_on_enter(void* context) {
    UNUSED(context);
}

bool fliplock_delete_confirmation_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_delete_confirmation_on_exit(void* context) {
    UNUSED(context);
}
