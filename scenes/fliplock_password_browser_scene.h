#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FlipLockPasswordBrowserEventEnterFolder = 0,
    FlipLockPasswordBrowserEventGoUpFolder,
    FlipLockPasswordBrowserEventCreateFolderRequest,
    FlipLockPasswordBrowserEventCreatePasswordRequest,
    FlipLockPasswordBrowserEventDeleteFolderRequest,
    FlipLockPasswordBrowserEventEditPasswordRequest,
    FlipLockPasswordBrowserEventInjectPasswordRequest,
    FlipLockPasswordBrowserEventViewPasswordRequest,
    FlipLockPasswordBrowserEventDeletePasswordRequest,
    FlipLockPasswordBrowserEventCount,
} FlipLockPasswordBrowserEvent;

void fliplock_password_browser_scene_callback(
    FlipLockPswrdBrowserEvent event,
    void* context);
void fliplock_password_browser_load_contents(void* context);
void fliplock_password_browser_on_enter(void* context);
bool fliplock_password_browser_on_event(
    void* context,
    SceneManagerEvent event);
void fliplock_password_browser_on_exit(void* context);

#ifdef __cplusplus
}
#endif
