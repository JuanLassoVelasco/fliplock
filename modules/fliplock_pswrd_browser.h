#pragma once

#include <gui/view.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLIPLOCK_PASSWORD_BROWSER_MAX_ITEMS   48U
#define FLIPLOCK_PASSWORD_BROWSER_LABEL_SIZE  64U
#define FLIPLOCK_PASSWORD_BROWSER_HEADER_SIZE 96U

typedef enum {
    FlipLockPswrdBrowserItemFolder = 0,
    FlipLockPswrdBrowserItemPassword,
    FlipLockPswrdBrowserItemUp,
    FlipLockPswrdBrowserItemAdd,
} FlipLockPswrdBrowserItemType;

typedef enum {
    FlipLockPswrdBrowserModeBrowse = 0,
    FlipLockPswrdBrowserModeActions,
} FlipLockPswrdBrowserMode;

typedef enum {
    FlipLockPswrdBrowserMenuTypeObject = 0,
    FlipLockPswrdBrowserMenuTypeItem,
} FlipLockPswrdBrowserMenuType;

typedef enum {
    FlipLockPswrdBrowserActionEdit = 0,
    FlipLockPswrdBrowserActionInject,
    FlipLockPswrdBrowserActionView,
    FlipLockPswrdBrowserActionDelete,
    FlipLockPswrdBrowserActionCount,
} FlipLockPswrdBrowserAction;

typedef enum {
    FlipLockPswrdBrowserEventEnter = 1,
    FlipLockPswrdBrowserEventBack,
    FlipLockPswrdBrowserEventOpenActionMenu,
    FlipLockPswrdBrowserEventSelectAction,
    FlipLockPswrdBrowserEventOpenEdit,
    FlipLockPswrdBrowserEventOpenInject,
    FlipLockPswrdBrowserEventOpenView,
    FlipLockPswrdBrowserEventOpenDelete,
    FlipLockPswrdBrowserEventCloseActionMenu,
} FlipLockPswrdBrowserEvent;

typedef void (*FlipLockPswrdBrowserCallback)(FlipLockPswrdBrowserEvent event, void* context);
typedef bool (*FlipLockPswrdBrowserBackFilter)(void* context);

typedef struct FlipLockPswrdBrowser FlipLockPswrdBrowser;

FlipLockPswrdBrowser* fliplock_pswrd_browser_alloc(void);
void fliplock_pswrd_browser_free(FlipLockPswrdBrowser* browser);
View* fliplock_pswrd_browser_get_view(FlipLockPswrdBrowser* browser);
void fliplock_pswrd_browser_set_callback(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserCallback callback,
    void* context);
void fliplock_pswrd_browser_set_back_filter(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserBackFilter back_filter);
void fliplock_pswrd_browser_reset(FlipLockPswrdBrowser* browser);
void fliplock_pswrd_browser_set_header(FlipLockPswrdBrowser* browser, const char* header);
void fliplock_pswrd_browser_set_has_parent(FlipLockPswrdBrowser* browser, bool has_parent);
void fliplock_pswrd_browser_set_mode(FlipLockPswrdBrowser* browser, FlipLockPswrdBrowserMode mode);
void fliplock_pswrd_browser_set_add_menu_type(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserMenuType add_menu_type);
void fliplock_pswrd_browser_add_item(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserItemType type,
    const char* label,
    uint8_t icon_id);
void fliplock_pswrd_browser_set_selected_item(FlipLockPswrdBrowser* browser, uint32_t index);
uint32_t fliplock_pswrd_browser_get_selected_item(const FlipLockPswrdBrowser* browser);
void fliplock_pswrd_browser_set_action_selected(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserAction action);
uint32_t fliplock_pswrd_browser_get_action_selected(const FlipLockPswrdBrowser* browser);
void fliplock_pswrd_browser_set_action_menu_open(FlipLockPswrdBrowser* browser, bool is_open);
bool fliplock_pswrd_browser_is_action_menu_open(const FlipLockPswrdBrowser* browser);
FlipLockPswrdBrowserItemType
    fliplock_pswrd_browser_get_selected_type(const FlipLockPswrdBrowser* browser);

#ifdef __cplusplus
}
#endif
