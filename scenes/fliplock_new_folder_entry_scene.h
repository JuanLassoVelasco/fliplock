#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLIPLOCK_NEW_FOLDER_ENTRY_SCENE_FLAG "New_Folder_entry"

typedef enum {
    FlipLockNewFolderEntryEventSubmitName = 0,
    FlipLockNewFolderEntryEventCancel,
    FlipLockNewFolderEntryEventReturnToBrowser,
    FlipLockNewFolderEntryEventCount,
} FlipLockNewFolderEntryEvent;

void fliplock_new_folder_entry_on_enter(void* context);
bool fliplock_new_folder_entry_on_event(void* context, SceneManagerEvent event);
void fliplock_new_folder_entry_on_exit(void* context);

#ifdef __cplusplus
}
#endif
