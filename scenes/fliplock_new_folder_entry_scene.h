#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FlipLockNewFolderEntryEventSubmitName = 0,
    FlipLockNewFolderEntryEventCancel,
    FlipLockNewFolderEntryEventCount,
} FlipLockNewFolderEntryEvent;

void fliplock_new_folder_entry_on_enter(void* context);
bool fliplock_new_folder_entry_on_event(void* context, SceneManagerEvent event);
void fliplock_new_folder_entry_on_exit(void* context);

#ifdef __cplusplus
}
#endif
