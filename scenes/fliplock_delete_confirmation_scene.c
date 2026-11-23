#include "fliplock_delete_confirmation_scene.h"

#define FLIPLOCK_CONFIRM_DELETE_FLAG "Confirm Delete"

void fliplock_delete_confirmation_on_enter(void* context) {
    FURI_LOG_D(FLIPLOCK_CONFIRM_DELETE_FLAG, "Back event called");
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
