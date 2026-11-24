#include "fliplock_new_folder_entry_scene.h"

void fliplock_new_folder_entry_name_submit_callback(void* context) {
    App* app = context;
    furi_assert(app->current_display_path != NULL);
    furi_assert(app->new_folder_label_buffer != NULL);

    FuriString* dir_create_path =
        furi_string_alloc_set(app->current_display_path);
    path_append(dir_create_path, app->new_folder_label_buffer);

    FS_Error dir_create_error = storage_common_mkdir(
        app->storage, furi_string_get_cstr(dir_create_path));

    if(dir_create_error != FSE_OK) {
        FURI_LOG_E(
            FLIPLOCK_NEW_FOLDER_ENTRY_SCENE_FLAG,
            "Failed to create new directory");
        if(dir_create_path) {
            furi_string_free(dir_create_path);
        }

        return;
    }

    FURI_LOG_I(
        FLIPLOCK_NEW_FOLDER_ENTRY_SCENE_FLAG,
        "Successfully created folder %s",
        furi_string_get_cstr(dir_create_path));

    furi_string_free(dir_create_path);

    scene_manager_handle_custom_event(
        app->scene_manager, FlipLockNewFolderEntryEventReturnToBrowser);
}

void fliplock_new_folder_entry_on_enter(void* context) {
    FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Entered new folder entry scene");
    App* app = context;
    bool clear_text = true;
    app->new_folder_label_buffer = malloc(FLIPLOCK_FOLDER_NAME_MAX_SIZE);
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, "Enter Folder Name");
    text_input_set_result_callback(
        app->text_input,
        fliplock_new_folder_entry_name_submit_callback,
        context,
        app->new_folder_label_buffer,
        FLIPLOCK_FOLDER_NAME_MAX_SIZE,
        clear_text);

    view_dispatcher_switch_to_view(
        app->view_dispatcher, FlipLockModules_TextInputEditor);
}

bool fliplock_new_folder_entry_on_event(
    void* context,
    SceneManagerEvent event) {
    App* app = context;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case FlipLockNewFolderEntryEventReturnToBrowser:
            scene_manager_next_scene(
                app->scene_manager, FlipLockScene_PasswordBrowser);
        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

void fliplock_new_folder_entry_on_exit(void* context) {
    App* app = context;

    if(app->new_folder_label_buffer) {
        free(app->new_folder_label_buffer);
        app->new_password_label_buffer = NULL;
    }
    text_input_reset(app->text_input);
}
