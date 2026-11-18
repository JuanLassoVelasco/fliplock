#include "fliplock_password_browser_scene.h"

void fliplock_password_browser_on_enter(void* context) {
    App* app = context;
    fliplock_pswrd_browser_reset(app->password_browser);
    fliplock_pswrd_browser_add_item(app->password_browser, FlipLockPswrdBrowserItemUp, "", 0U);
    fliplock_pswrd_browser_add_item(
        app->password_browser, FlipLockPswrdBrowserItemFolder, "     Test_folder", 2U);
    fliplock_pswrd_browser_add_item(
        app->password_browser, FlipLockPswrdBrowserItemPassword, "     Test_password", 3U);
    fliplock_pswrd_browser_add_item(
        app->password_browser, FlipLockPswrdBrowserItemAdd, "        Add", 1U);

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
