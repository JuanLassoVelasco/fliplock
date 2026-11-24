#include "fliplock_delete_confirmation_scene.h"

#define FLIPLOCK_CONFIRM_DELETE_FLAG "Confirm Delete"

void fliplock_confirm_delete_scene_callback(
    DialogExResult result,
    void* context) {
    App* app = context;

    FURI_LOG_D(FLIPLOCK_CONFIRM_DELETE_FLAG, "Result: %d", result);

    switch(result) {
    case DialogExResultLeft:
        scene_manager_handle_custom_event(
            app->scene_manager, FlipLockConfirmDeleteEventCancel);
        break;
    case DialogExResultRight:
        scene_manager_handle_custom_event(
            app->scene_manager, FlipLockConfirmDeleteEventDeleteFolder);
        break;
    default:
        break;
    }
}

void fliplock_delete_confirmation_on_enter(void* context) {
    FURI_LOG_D(
        FLIPLOCK_CONFIRM_DELETE_FLAG, "Delete confirmation scene entered");
    App* app = context;

    dialog_ex_reset(app->dialog);
    dialog_ex_set_header(
        app->dialog,
        "Confirm Deletion of",
        64U,
        22U,
        AlignCenter,
        AlignCenter);
    dialog_ex_set_text(
        app->dialog,
        app->folder_delete_label_buffer,
        64U,
        35U,
        AlignCenter,
        AlignCenter);
    dialog_ex_set_left_button_text(app->dialog, "Cancel");
    dialog_ex_set_right_button_text(app->dialog, "Delete");
    dialog_ex_set_result_callback(
        app->dialog, fliplock_confirm_delete_scene_callback);
    dialog_ex_set_context(app->dialog, context);

    view_dispatcher_switch_to_view(
        app->view_dispatcher, FlipLockModules_Dialog);
}

bool fliplock_delete_confirmation_on_event(
    void* context,
    SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    FuriString* folder_path =
        furi_string_alloc_set(app->current_display_path);

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case FlipLockConfirmDeleteEventCancel:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, FlipLockScene_PasswordBrowser);
            consumed = true;
            break;
        case FlipLockConfirmDeleteEventDeleteFolder:
            path_append(folder_path, app->folder_delete_label_buffer);
            storage_common_remove(
                app->storage, furi_string_get_cstr(folder_path));
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, FlipLockScene_PasswordBrowser);
            consumed = true;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if(folder_path) {
        furi_string_free(folder_path);
    }

    return consumed;
}

void fliplock_delete_confirmation_on_exit(void* context) {
    App* app = context;

    if(app->folder_delete_label_buffer) {
        free(app->folder_delete_label_buffer);
        app->folder_delete_label_buffer = NULL;
    }

    dialog_ex_reset(app->dialog);
}
