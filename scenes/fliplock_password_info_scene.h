#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

void fliplock_password_info_on_enter(void* context);
bool fliplock_password_info_on_event(void* context, SceneManagerEvent event);
void fliplock_password_info_on_exit(void* context);

#ifdef __cplusplus
}
#endif
