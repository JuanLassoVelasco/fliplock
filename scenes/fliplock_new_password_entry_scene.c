#include "fliplock_new_password_entry_scene.h"

void fliplock_new_password_entry_item_callback(
    void* context,
    uint32_t index) {
    App* app = context;

    switch(index) {
    case FlipLockNewPasswordEntryItemLabel:
        scene_manager_handle_custom_event(
            app->scene_manager,
            FlipLockNewPasswordEntryEventGoToFileNameEntry);
        break;
    case FlipLockNewPasswordEntryItemUsername:
        scene_manager_handle_custom_event(
            app->scene_manager,
            FlipLockNewPasswordEntryEventGoToUsernameEntry);
        break;
    case FlipLockNewPasswordEntryItemPassword:
        scene_manager_handle_custom_event(
            app->scene_manager,
            FlipLockNewPasswordEntryEventGoToPasswordEntry);
        break;
    case FlipLockNewPasswordEntryItemCreate:
        scene_manager_handle_custom_event(
            app->scene_manager,
            FlipLockNewPasswordEntryEventCreatePasswordFile);
        break;
    default:
        break;
    }
}

bool create_new_password_entry(void* context) {
    App* app = context;
    bool success = false;

    if(!app->new_password_label_buffer) {
        FURI_LOG_E(
            FLIPLOCK_NEW_PASSWORD_ENTRY_SCENE_FLAG,
            "Failed to create new login credentials: No label given");
        return false;
    }
    if(!app->new_password_username_buffer) {
        FURI_LOG_E(
            FLIPLOCK_NEW_PASSWORD_ENTRY_SCENE_FLAG,
            "Failed to create new login credentials: No username given");
        return false;
    }
    if(!app->new_password_buffer) {
        FURI_LOG_E(
            FLIPLOCK_NEW_PASSWORD_ENTRY_SCENE_FLAG,
            "Failed to create new login credentials: No password given");
        return false;
    }

    FuriString* newFileName =
        furi_string_alloc_set(app->current_display_path);
    FuriString* passwordCredentials =
        furi_string_alloc_set(app->new_password_username_buffer);
    path_append(newFileName, app->new_password_label_buffer);
    furi_string_cat(newFileName, ".txt");
    furi_string_cat(passwordCredentials, "\n");
    furi_string_cat(passwordCredentials, app->new_password_buffer);
    furi_string_cat(passwordCredentials, "\n");
    size_t credentials_size =
        strlen(furi_string_get_cstr(passwordCredentials));

    if(storage_file_open(
           app->storage_file,
           furi_string_get_cstr(newFileName),
           FSAM_WRITE,
           FSOM_CREATE_NEW)) {
        size_t written = storage_file_write(
            app->storage_file,
            furi_string_get_cstr(passwordCredentials),
            credentials_size);

        if(written != credentials_size) {
            success = false;
        } else {
            success = true;
        }
    }

    return success;
}

void fliplock_password_entry_on_enter(void* context) {
    App* app = context;
    FuriString* nameItemLabel = furi_string_alloc_set("Entry Label");
    FuriString* usernameItemLabel = furi_string_alloc_set("Username");
    FuriString* passwordItemLabel = furi_string_alloc_set("Password");

    if(app->new_password_label_buffer) {
        furi_string_cat(nameItemLabel, "  *****");
    }
    if(app->new_password_username_buffer) {
        furi_string_cat(usernameItemLabel, "     *****");
    }
    if(app->new_password_buffer) {
        furi_string_cat(passwordItemLabel, "     *****");
    }

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "New Password Entry");
    submenu_add_item(
        app->submenu,
        furi_string_get_cstr(nameItemLabel),
        0U,
        fliplock_new_password_entry_item_callback,
        context);
    submenu_add_item(
        app->submenu,
        furi_string_get_cstr(usernameItemLabel),
        1U,
        fliplock_new_password_entry_item_callback,
        context);
    submenu_add_item(
        app->submenu,
        furi_string_get_cstr(passwordItemLabel),
        2U,
        fliplock_new_password_entry_item_callback,
        context);
    submenu_add_item(
        app->submenu,
        "Create",
        3U,
        fliplock_new_password_entry_item_callback,
        context);

    view_dispatcher_switch_to_view(
        app->view_dispatcher, FlipLockModules_Submenu);

    furi_string_free(nameItemLabel);
    furi_string_free(usernameItemLabel);
    furi_string_free(passwordItemLabel);
}

bool fliplock_password_entry_on_event(
    void* context,
    SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeBack:
        if(app->new_password_label_buffer ||
           app->new_password_username_buffer || app->new_password_buffer) {
            scene_manager_next_scene(
                app->scene_manager, FlipLockScene_UnsavedChanges);
            consumed = true;
        }
        break;
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case FlipLockNewPasswordEntryEventGoToFileNameEntry:
            scene_manager_next_scene(
                app->scene_manager, FlipLockScene_PasswordNameEntry);
            consumed = true;
            break;
        case FlipLockNewPasswordEntryEventGoToUsernameEntry:
            scene_manager_next_scene(
                app->scene_manager, FlipLockScene_UsernameGeneration);
            consumed = true;
            break;
        case FlipLockNewPasswordEntryEventGoToPasswordEntry:
            scene_manager_next_scene(
                app->scene_manager, FlipLockScene_PasswordGeneration);
            consumed = true;
            break;
        case FlipLockNewPasswordEntryEventCreatePasswordFile:
            if(app->new_password_label_buffer &&
               app->new_password_username_buffer &&
               app->new_password_buffer) {
                create_new_password_entry(app);
                scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, FlipLockScene_PasswordBrowser);
                consumed = true;
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return consumed; // event not handled
}

void fliplock_password_entry_on_exit(void* context) {
    App* app = context;
    submenu_reset(app->submenu);
}
