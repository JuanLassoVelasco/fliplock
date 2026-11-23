#include "fliplock_password_browser_scene.h"

void fliplock_password_browser_scene_callback(FlipLockPswrdBrowserEvent event, void* context) {
    App* app = context;
    FlipLockPswrdBrowserItemType selected_type =
        fliplock_pswrd_browser_get_selected_type(app->password_browser);
    uint32_t action_selected = fliplock_pswrd_browser_get_action_selected(app->password_browser);

    switch(event) {
    case FlipLockPswrdBrowserEventEnter:
        if(selected_type == FlipLockPswrdBrowserItemFolder) {
            scene_manager_handle_custom_event(
                app->scene_manager, FlipLockPasswordBrowserEventEnterFolder);
            break;
        }
        if(selected_type == FlipLockPswrdBrowserItemUp) {
            scene_manager_handle_custom_event(
                app->scene_manager, FlipLockPasswordBrowserEventGoUpFolder);
            break;
        }
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventEnter);
        break;
    case FlipLockPswrdBrowserEventBack:
        if(!app->at_root_path &&
           !fliplock_pswrd_browser_is_action_menu_open(app->password_browser)) {
            scene_manager_handle_custom_event(
                app->scene_manager, FlipLockPasswordBrowserEventGoUpFolder);
            break;
        }
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventBack);
        break;
    case FlipLockPswrdBrowserEventOpenActionMenu:
        scene_manager_handle_custom_event(
            app->scene_manager, FlipLockPswrdBrowserEventOpenActionMenu);
        break;
    case FlipLockPswrdBrowserEventSelectAction:
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
                    app->scene_manager, FlipLockPasswordBrowserEventCreateFolder);
                break;
            case FlipLockPswrdBrowserCreatePassword:
                scene_manager_handle_custom_event(
                    app->scene_manager, FlipLockPasswordBrowserEventCreatePassword);
                break;
            default:
                break;
            }
            break;
        }
        scene_manager_handle_custom_event(
            app->scene_manager, FlipLockPswrdBrowserEventSelectAction);
        break;
    case FlipLockPswrdBrowserEventOpenEdit:
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventOpenEdit);
        break;
    case FlipLockPswrdBrowserEventOpenInject:
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventOpenInject);
        break;
    case FlipLockPswrdBrowserEventOpenView:
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventOpenView);
        break;
    case FlipLockPswrdBrowserEventOpenDelete:
        scene_manager_handle_custom_event(app->scene_manager, FlipLockPswrdBrowserEventOpenDelete);
        break;
    case FlipLockPswrdBrowserEventCloseActionMenu:
        scene_manager_handle_custom_event(
            app->scene_manager, FlipLockPswrdBrowserEventCloseActionMenu);
        break;
    default:
        break;
    }
}

void fliplock_password_browser_load_contents(void* context) {
    App* app = context;
    if(!storage_dir_open(app->storage_file, app->current_display_path)) {
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
    fliplock_pswrd_browser_reset(app->password_browser);
    if(fliplock_pswrd_browser_get_has_parent(app->password_browser)) {
        fliplock_pswrd_browser_add_item(
            app->password_browser,
            FlipLockPswrdBrowserItemUp,
            "..",
            FLIPLOCK_PSWRD_BROWSER_ICON_NONE);
    }
    fliplock_password_browser_load_contents(app);
    if(strcmp(app->current_display_path, app->root_path) == 0) {
        fliplock_pswrd_browser_set_header(app->password_browser, "Password Library");
    } else {
        fliplock_pswrd_browser_set_header(app->password_browser, app->current_display_path);
    }
    fliplock_pswrd_browser_add_item(
        app->password_browser,
        FlipLockPswrdBrowserItemAdd,
        "Add",
        FLIPLOCK_PSWRD_BROWSER_ICON_NONE);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipLockModules_PasswordBrowser);
}

bool fliplock_password_browser_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false; // event not handled
}

void fliplock_password_browser_on_exit(void* context) {
    App* app = context;
    fliplock_pswrd_browser_reset(app->password_browser);
}
