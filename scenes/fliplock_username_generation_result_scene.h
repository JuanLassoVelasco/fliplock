#pragma once

#include "fliplock.h"

#ifdef __cplusplus
extern "C" {
#endif

void fliplock_username_generation_result_on_enter(void* context);
bool fliplock_username_generation_result_on_event(void* context, SceneManagerEvent event);
void fliplock_username_generation_result_on_exit(void* context);

#ifdef __cplusplus
}
#endif
