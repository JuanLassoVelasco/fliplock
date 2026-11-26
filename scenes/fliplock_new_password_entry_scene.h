#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLIPLOCK_NEW_PASSWORD_ENTRY_SCENE_FLAG "New Password Entry"

typedef enum {
    FlipLockNewPasswordEntryItemLabel = 0,
    FlipLockNewPasswordEntryItemUsername,
    FlipLockNewPasswordEntryItemPassword,
    FlipLockNewPasswordEntryItemCreate,
    FlipLockNewPasswordEntryItemCount
} FlipLockNewPasswordEntryItem;

typedef enum {
    FlipLockNewPasswordEntryEventGoToFileNameEntry,
    FlipLockNewPasswordEntryEventGoToUsernameEntry,
    FlipLockNewPasswordEntryEventGoToPasswordEntry,
    FlipLockNewPasswordEntryEventCreatePasswordFile,
    FlipLockNewPasswordEntryEventCount
} FlipLockNewPasswordEntryEvent;

void fliplock_password_entry_on_enter(void* context);
bool fliplock_password_entry_on_event(
    void* context,
    SceneManagerEvent event);
void fliplock_password_entry_on_exit(void* context);

#ifdef __cplusplus
}
#endif
