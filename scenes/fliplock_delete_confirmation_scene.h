#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FlipLockConfirmDeleteEventCancel = 0,
    FlipLockConfirmDeleteEventDeleteFolder,
    FlipLockConfirmDeleteEventCount
} FlipLockConfirmDeleteEvent;

void fliplock_delete_confirmation_on_enter(void* context);
bool fliplock_delete_confirmation_on_event(
    void* context,
    SceneManagerEvent event);
void fliplock_delete_confirmation_on_exit(void* context);

#ifdef __cplusplus
}
#endif
