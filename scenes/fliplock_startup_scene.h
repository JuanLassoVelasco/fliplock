#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FlipLockStartupViewPasswordsEvent,
} FlipLockStartupEvent;

void fliplock_startup_button_callback(GuiButtonType result, InputType type, void* context);
void fliplock_startup_on_enter(void* context);
bool fliplock_startup_on_event(void* context, SceneManagerEvent event);
void fliplock_startup_on_exit(void* context);

#ifdef __cplusplus
}
#endif
