#include "fliplock_password_browser_scene.h"

#define FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG "PSWRD_BROWSER_SCENE"

void fliplock_password_browser_scene_callback(FlipLockPswrdBrowserEvent event, void* context) {
    App* app = context;
    FlipLockPswrdBrowserItemType selected_type =
        fliplock_pswrd_browser_get_selected_type(app->password_browser);
    uint32_t action_selected = fliplock_pswrd_browser_get_action_selected(app->password_browser);
    FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Processing brower event");
    FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Action Selected: %ld", action_selected);
    FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Event Id: %d", event);

    switch(event) {
    case FlipLockPswrdBrowserEventEnter:
        FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Enter event called");
        if(selected_type == FlipLockPswrdBrowserItemUp) {
            scene_manager_handle_custom_event(
                app->scene_manager, FlipLockPasswordBrowserEventGoUpFolder);
            break;
        }
        break;
    case FlipLockPswrdBrowserEventBack:
        FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Back event called");
        if(!app->at_root_path &&
           !fliplock_pswrd_browser_is_action_menu_open(app->password_browser)) {
            scene_manager_handle_custom_event(
                app->scene_manager, FlipLockPasswordBrowserEventGoUpFolder);
            break;
        }
        break;
    case FlipLockPswrdBrowserEventOpenActionMenu:
        FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Open Action menu event called");
        break;
    case FlipLockPswrdBrowserEventSelectAction:
        FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Select action event called");
        if(!fliplock_pswrd_browser_is_action_menu_open(app->password_browser)) {
            break;
        }
        if(selected_type == FlipLockPswrdBrowserItemPassword) {
            switch(action_selected) {
            case FlipLockPswrdBrowserActionEdit:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPswrdBrowserEventOpenEdit);
                break;
            case FlipLockPswrdBrowserActionInject:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPswrdBrowserEventOpenInject);
                break;
            case FlipLockPswrdBrowserActionView:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPswrdBrowserEventOpenView);
                break;
            case FlipLockPswrdBrowserActionDelete:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPswrdBrowserEventOpenDelete);
                break;
            default:
                break;
            }
            break;
        } else if(selected_type == FlipLockPswrdBrowserItemAdd) {
            switch(action_selected) {
            case FlipLockPswrdBrowserCreateFolder:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPasswordBrowserEventCreateFolderRequest);
                break;
            case FlipLockPswrdBrowserCreatePassword:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPasswordBrowserEventCreatePasswordRequest);
                break;
            default:
                break;
            }
            break;
        } else if(selected_type == FlipLockPswrdBrowserItemFolder) {
            switch(action_selected) {
            case FlipLockPswrdBrowserFolderActionOpen:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPasswordBrowserEventEnterFolder);
                break;
            case FlipLockPswrdBrowserFolderActionDelete:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPasswordBrowserEventDeleteFolderRequest);
                break;
            default:
                break;
            }
            break;
        }
        break;
    case FlipLockPswrdBrowserEventCloseActionMenu:
        FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Close action menu event called");
        break;
    default:
        break;
    }
}

void fliplock_password_browser_load_contents(void* context) {
    App* app = context;
    FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Loaded password browser contents");
    if(!storage_dir_open(app->storage_file, furi_string_get_cstr(app->current_display_path))) {
        // Handle error opening directory
        storage_dir_close(app->storage_file);
        return;
    }

    FileInfo* file_info = malloc(sizeof(FileInfo));
    char* entry_name = malloc(FLIPLOCK_PASSWORD_BROWSER_LABEL_SIZE);
    while(storage_dir_read(
        app->storage_file, file_info, entry_name, FLIPLOCK_PASSWORD_BROWSER_LABEL_SIZE)) {
        if(file_info_is_dir(file_info)) {
            fliplock_pswrd_browser_add_item(
                app->password_browser,
                FlipLockPswrdBrowserItemFolder,
                entry_name,
                FLIPLOCK_PSWRD_BROWSER_ICON_NONE);
        } else {
            fliplock_pswrd_browser_add_item(
                app->password_browser,
                FlipLockPswrdBrowserItemPassword,
                entry_name,
                FLIPLOCK_PSWRD_BROWSER_ICON_NONE);
        }
    }
    free(file_info);
    free(entry_name);

    storage_dir_close(app->storage_file);
}

void fliplock_password_browser_on_enter(void* context) {
    App* app = context;
    FURI_LOG_D(FLIPLOCK_DEBUG_LOG_TAG, "Entered password browser scene");
    fliplock_pswrd_browser_reset(app->password_browser);
    fliplock_pswrd_browser_set_callback(
        app->password_browser, fliplock_password_browser_scene_callback, context);
    if(fliplock_pswrd_browser_get_has_parent(app->password_browser)) {
        fliplock_pswrd_browser_add_item(
            app->password_browser,
            FlipLockPswrdBrowserItemUp,
            "..",
            FLIPLOCK_PSWRD_BROWSER_ICON_NONE);
    }
    fliplock_password_browser_load_contents(app);
    if(furi_string_equal(app->current_display_path, app->root_path)) {
        fliplock_pswrd_browser_set_header(app->password_browser, "Password Library");
    } else {
        fliplock_pswrd_browser_set_header(
            app->password_browser, furi_string_get_cstr(app->current_display_path));
    }
    fliplock_pswrd_browser_add_item(
        app->password_browser,
        FlipLockPswrdBrowserItemAdd,
        "Add",
        FLIPLOCK_PSWRD_BROWSER_ICON_NONE);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipLockModules_PasswordBrowser);
}

bool fliplock_password_browser_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;
    FuriString* new_display_path = NULL;
    FuriString* upper_directory_path = NULL;
    char* selected_label = fliplock_pswrd_browser_get_selected_label(app->password_browser);

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case FlipLockPasswordBrowserEventEnterFolder:
            new_display_path = furi_string_alloc();
            furi_string_cat(new_display_path, app->current_display_path);
            path_append(new_display_path, selected_label);
            app->current_display_path = new_display_path;
            fliplock_pswrd_browser_set_has_parent(app->password_browser, true);
            fliplock_password_browser_on_enter(context);
            consumed = true;
            break;
        case FlipLockPasswordBrowserEventGoUpFolder:
            if(app->at_root_path) {
                consumed = true;
                break;
            }
            upper_directory_path = furi_string_alloc();
            path_extract_dirname(
                furi_string_get_cstr(app->current_display_path), upper_directory_path);
            app->current_display_path = upper_directory_path;
            if(furi_string_equal(app->current_display_path, app->root_path)) {
                fliplock_pswrd_browser_set_has_parent(app->password_browser, false);
                app->at_root_path = true;
            }
            fliplock_password_browser_on_enter(context);
            consumed = true;
            break;
        case FlipLockPasswordBrowserEventCreateFolderRequest:
            FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Create Folder request made");
            scene_manager_next_scene(app->scene_manager, FlipLockScene_NewFolderEntry);
            consumed = true;
            break;
        case FlipLockPasswordBrowserEventCreatePasswordRequest:
            FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Create password request made");
            scene_manager_next_scene(app->scene_manager, FlipLockScene_PasswordEntry);
            consumed = true;
            break;
        case FlipLockPswrdBrowserEventOpenEdit:
            FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Open Edit Request made");
            app->current_password_label = strdup(selected_label);
            scene_manager_next_scene(app->scene_manager, FlipLockScene_PasswordEdit);
            consumed = true;
            break;
        case FlipLockPswrdBrowserEventOpenInject:
            app->current_password_label = strdup(selected_label);
            scene_manager_next_scene(app->scene_manager, FlipLockScene_PasswordInjection);
            consumed = true;
            break;
        case FlipLockPswrdBrowserEventOpenView:
            app->current_password_label = strdup(selected_label);
            scene_manager_next_scene(app->scene_manager, FlipLockScene_PasswordInfo);
            consumed = true;
            break;
        case FlipLockPswrdBrowserEventOpenDelete:
            app->current_password_label = strdup(selected_label);
            scene_manager_next_scene(app->scene_manager, FlipLockScene_DeleteConfirmation);
            consumed = true;
            break;
        case FlipLockPasswordBrowserEventDeleteFolderRequest:
            app->folder_delete_label_buffer = strdup(selected_label);
            scene_manager_next_scene(app->scene_manager, FlipLockScene_DeleteConfirmation);
            consumed = true;
            break;
        default:
            break;
        }
    default:
        break;
    }

    if(new_display_path) {
        furi_string_free(new_display_path);
    }
    if(upper_directory_path) {
        furi_string_free(upper_directory_path);
    }

    return consumed; // event not handled
}

void fliplock_password_browser_on_exit(void* context) {
    FURI_LOG_D(FLIPLOCK_PASSWORD_BROWSER_SCENE_TAG, "Leaving password browser scene");
    App* app = context;
    fliplock_pswrd_browser_reset(app->password_browser);
}
