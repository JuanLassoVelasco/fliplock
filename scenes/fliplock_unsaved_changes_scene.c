#include "fliplock_unsaved_changes_scene.h"

void fliplock_unsaved_changes_on_enter(void* context) {
    UNUSED(context);
}

bool fliplock_unsaved_changes_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_unsaved_changes_on_exit(void* context) {
    UNUSED(context);
}
