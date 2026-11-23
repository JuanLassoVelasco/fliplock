#include "fliplock.h"
#include "scenes/fliplock_startup_scene.h"
#include "scenes/fliplock_password_browser_scene.h"
#include "scenes/fliplock_new_folder_entry_scene.h"
#include "scenes/fliplock_new_password_entry_scene.h"
#include "scenes/fliplock_password_edit_scene.h"
#include "scenes/fliplock_enter_username_scene.h"
#include "scenes/fliplock_enter_password_scene.h"
#include "scenes/fliplock_username_generation_scene.h"
#include "scenes/fliplock_password_generation_scene.h"
#include "scenes/fliplock_password_info_scene.h"
#include "scenes/fliplock_delete_confirmation_scene.h"
#include "scenes/fliplock_username_generation_confirmation_scene.h"
#include "scenes/fliplock_password_generation_confirmation_scene.h"
#include "scenes/fliplock_username_generation_result_scene.h"
#include "scenes/fliplock_password_generation_result_scene.h"
#include "scenes/fliplock_unsaved_changes_scene.h"
#include "scenes/fliplock_password_injection_scene.h"
#include "scenes/fliplock_injection_progress_scene.h"
#include "scenes/fliplock_injection_complete_scene.h"

const char* fliplock_item_label_offset_string = "        ";

void (*const fliplock_scene_on_enter_handlers[])(void*) = {
    fliplock_startup_on_enter,
    fliplock_password_browser_on_enter,
    fliplock_new_folder_entry_on_enter,
    fliplock_password_entry_on_enter,
    fliplock_password_edit_on_enter,
    fliplock_enter_username_on_enter,
    fliplock_enter_password_on_enter,
    fliplock_username_generation_on_enter,
    fliplock_password_generation_on_enter,
    fliplock_password_info_on_enter,
    fliplock_delete_confirmation_on_enter,
    fliplock_username_generation_confirmation_on_enter,
    fliplock_password_generation_confirmation_on_enter,
    fliplock_username_generation_result_on_enter,
    fliplock_password_generation_result_on_enter,
    fliplock_unsaved_changes_on_enter,
    fliplock_password_injection_on_enter,
    fliplock_injection_progress_on_enter,
    fliplock_injection_complete_on_enter};

bool (*const fliplock_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    fliplock_startup_on_event,
    fliplock_password_browser_on_event,
    fliplock_new_folder_entry_on_event,
    fliplock_password_entry_on_event,
    fliplock_password_edit_on_event,
    fliplock_enter_username_on_event,
    fliplock_enter_password_on_event,
    fliplock_username_generation_on_event,
    fliplock_password_generation_on_event,
    fliplock_password_info_on_event,
    fliplock_delete_confirmation_on_event,
    fliplock_username_generation_confirmation_on_event,
    fliplock_password_generation_confirmation_on_event,
    fliplock_username_generation_result_on_event,
    fliplock_password_generation_result_on_event,
    fliplock_unsaved_changes_on_event,
    fliplock_password_injection_on_event,
    fliplock_injection_progress_on_event,
    fliplock_injection_complete_on_event};

void (*const fliplock_scene_on_exit_handlers[])(void*) = {
    fliplock_startup_on_exit,
    fliplock_password_browser_on_exit,
    fliplock_new_folder_entry_on_exit,
    fliplock_password_entry_on_exit,
    fliplock_password_edit_on_exit,
    fliplock_enter_username_on_exit,
    fliplock_enter_password_on_exit,
    fliplock_username_generation_on_exit,
    fliplock_password_generation_on_exit,
    fliplock_password_info_on_exit,
    fliplock_delete_confirmation_on_exit,
    fliplock_username_generation_confirmation_on_exit,
    fliplock_password_generation_confirmation_on_exit,
    fliplock_username_generation_result_on_exit,
    fliplock_password_generation_result_on_exit,
    fliplock_unsaved_changes_on_exit,
    fliplock_password_injection_on_exit,
    fliplock_injection_progress_on_exit,
    fliplock_injection_complete_on_exit};

static const SceneManagerHandlers fliplock_scene_manager_handlers = {
    .on_enter_handlers = fliplock_scene_on_enter_handlers,
    .on_event_handlers = fliplock_scene_on_event_handlers,
    .on_exit_handlers = fliplock_scene_on_exit_handlers,
    .scene_num = FlipLockSceneCount};

static bool fliplock_custom_callback(void* context, uint32_t custom_event) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

bool fliplock_back_event_callback(void* context) {
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static App* app_alloc() {
    App* app = malloc(sizeof(App));
    app->scene_manager = scene_manager_alloc(&fliplock_scene_manager_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, fliplock_custom_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, fliplock_back_event_callback);
    app->password_browser = fliplock_pswrd_browser_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipLockModules_PasswordBrowser,
        fliplock_pswrd_browser_get_view(app->password_browser));
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlipLockModules_TextInputEditor,
        text_input_get_view(app->text_input));
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipLockModules_Submenu, submenu_get_view(app->submenu));
    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipLockModules_Dialog, dialog_ex_get_view(app->dialog));
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipLockModules_Widget, widget_get_view(app->widget));
    app->loading = loading_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FlipLockModules_Loading, loading_get_view(app->loading));
    app->storage = furi_record_open(RECORD_STORAGE);
    app->storage_file = storage_file_alloc(app->storage);
    app->root_path = furi_string_alloc();
    furi_string_set(app->root_path, APP_DATA_PATH(""));
    app->current_display_path = furi_string_alloc();
    furi_string_set(app->current_display_path, furi_string_get_cstr(app->root_path));
    app->at_root_path = true;
    app->current_password_label = NULL;
    app->new_password_label_buffer = NULL;
    app->new_password_username_buffer = NULL;
    app->new_password_buffer = NULL;
    app->new_folder_label_buffer = NULL;
    app->folder_delete_label_buffer = NULL;

    return app;
}

static void app_free(App* app) {
    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_PasswordBrowser);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_TextInputEditor);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_Submenu);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_Dialog);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_Widget);
    view_dispatcher_remove_view(app->view_dispatcher, FlipLockModules_Loading);
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    fliplock_pswrd_browser_free(app->password_browser);
    text_input_free(app->text_input);
    submenu_free(app->submenu);
    dialog_ex_free(app->dialog);
    widget_free(app->widget);
    loading_free(app->loading);
    storage_file_free(app->storage_file);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(app->root_path);
    furi_string_free(app->current_display_path);
    if(app->current_password_label) {
        free(app->current_password_label);
    }
    if(app->new_password_label_buffer) {
        free(app->new_password_label_buffer);
    }
    if(app->new_password_username_buffer) {
        free(app->new_password_username_buffer);
    }
    if(app->new_password_buffer) {
        free(app->new_password_buffer);
    }
    if(app->new_folder_label_buffer) {
        free(app->new_folder_label_buffer);
    }
    if(app->folder_delete_label_buffer) {
        free(app->folder_delete_label_buffer);
    }

    free(app);
}

int32_t fliplock_app(void* p) {
    UNUSED(p);
    App* app = app_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, FlipLockScene_Startup);
    view_dispatcher_run(app->view_dispatcher);

    app_free(app);

    furi_record_close(RECORD_GUI);

    return 0;
}
