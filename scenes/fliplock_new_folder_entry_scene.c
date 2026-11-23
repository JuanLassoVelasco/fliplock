#include "fliplock_new_folder_entry_scene.h"

void fliplock_new_folder_entry_name_submit_callback(void* context) {
    UNUSED(context);
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipLockModules_TextInputEditor);
}

bool fliplock_new_folder_entry_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_new_folder_entry_on_exit(void* context) {
    App* app = context;

    if(app->new_folder_label_buffer) {
        free(app->new_password_label_buffer);
    }
    text_input_reset(app->text_input);
}
