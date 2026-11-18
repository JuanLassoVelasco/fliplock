#include "fliplock_enter_password_scene.h"

void fliplock_enter_password_on_enter(void* context) {
    UNUSED(context);
}

bool fliplock_enter_password_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_enter_password_on_exit(void* context) {
    UNUSED(context);
}
