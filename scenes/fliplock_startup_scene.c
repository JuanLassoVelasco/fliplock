#include "fliplock_startup_scene.h"

void fliplock_startup_button_callback(GuiButtonType result, InputType type, void* context) {
    App* app = context;
    if(result == GuiButtonTypeRight && type == InputTypeRelease) {
        scene_manager_handle_custom_event(app->scene_manager, FlipLockStartupViewPasswordsEvent);
    }
}

void fliplock_startup_on_enter(void* context) {
    App* app = context;

    widget_reset(app->widget);
    widget_add_icon_element(app->widget, 0, 10, &I_WarningDolphin_45x42);
    widget_add_rect_element(app->widget, 62, 6, 58, 20, 4, false);
    widget_add_string_element(
        app->widget, 70, 16, AlignLeft, AlignCenter, FontPrimary, "FlipLock!");
    widget_add_string_element(
        app->widget, 55, 34, AlignLeft, AlignCenter, FontSecondary, "Virtual Password");
    widget_add_string_element(
        app->widget, 70, 46, AlignLeft, AlignCenter, FontSecondary, "Notebook");
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Browse Passwords", fliplock_startup_button_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipLockModules_Widget);
}

bool fliplock_startup_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case FlipLockStartupViewPasswordsEvent:
            scene_manager_next_scene(app->scene_manager, FlipLockScene_PasswordBrowser);
            consumed = true;
            break;
        }
        break;
    default:
        break;
    }
    return consumed;
}

void fliplock_startup_on_exit(void* context) {
    App* app = context;
    widget_reset(app->widget);
}
