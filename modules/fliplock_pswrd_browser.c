#include "fliplock_pswrd_browser.h"

#include <furi.h>
#include <gui/elements.h>
#include <storage/storage.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FLIPLOCK_PSWRD_BROWSER_SCROLL_INTERVAL_MS 333U
#define FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS       4U
#define FLIPLOCK_PSWRD_BROWSER_ROW_HEIGHT         11U
#define FLIPLOCK_PSWRD_BROWSER_LIST_Y             9U
#define FLIPLOCK_PSWRD_BROWSER_ICON_X             4U
#define FLIPLOCK_PSWRD_BROWSER_TEXT_X             17U

#define FLIPLOCK_PSWRD_BROWSER_ICON_FILE  APP_ASSETS_PATH("fliplock_icons_10x10.bin")
#define FLIPLOCK_PSWRD_BROWSER_ICON_COUNT 69U
#define FLIPLOCK_PSWRD_BROWSER_ICON_SIZE  10U
#define FLIPLOCK_PSWRD_BROWSER_ICON_BYTES 20U

typedef struct {
    char label[FLIPLOCK_PASSWORD_BROWSER_LABEL_SIZE];
    uint8_t type;
    uint8_t icon_id;
} FlipLockPswrdBrowserItem;

_Static_assert(
    FlipLockPswrdBrowserItemAdd <= UINT8_MAX,
    "FlipLockPswrdBrowserItemType must fit in uint8_t");

typedef struct {
    char header[FLIPLOCK_PASSWORD_BROWSER_HEADER_SIZE];
    FlipLockPswrdBrowserItem items[FLIPLOCK_PASSWORD_BROWSER_MAX_ITEMS];
    uint32_t item_count;
    uint32_t selected_index;
    uint32_t action_selected_index;
    size_t scroll_counter;
    bool has_parent;
    bool action_menu_open;
    FlipLockPswrdBrowserMode mode;
    FlipLockPswrdBrowserMenuType menu_type;
    FuriString* draw_string;
    uint8_t icon_ids[FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS];
    uint8_t icon_xbm[FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS][FLIPLOCK_PSWRD_BROWSER_ICON_BYTES];
} FlipLockPswrdBrowserModel;

struct FlipLockPswrdBrowser {
    View* view;
    FuriTimer* scroll_timer;
    FlipLockPswrdBrowserCallback callback;
    FlipLockPswrdBrowserBackFilter back_filter;
    void* callback_context;
    bool icon_file_unavailable;
};

static const char* fliplock_pswrd_browser_password_action_labels[FlipLockPswrdBrowserActionCount] =
    {
        "Edit",
        "Inject",
        "View",
        "Delete",
};

static const char* fliplock_pswrd_browser_create_action_labels[] = {
    "Folder",
    "Password",
};

static bool fliplock_pswrd_browser_selected_is_password(const FlipLockPswrdBrowserModel* model);
static bool fliplock_pswrd_browser_selected_is_add(const FlipLockPswrdBrowserModel* model);

static uint32_t fliplock_pswrd_browser_password_action_count(void) {
    return COUNT_OF(fliplock_pswrd_browser_password_action_labels);
}

static uint32_t fliplock_pswrd_browser_create_action_count(void) {
    return COUNT_OF(fliplock_pswrd_browser_create_action_labels);
}

static const char* fliplock_pswrd_browser_create_action_label(
    const FlipLockPswrdBrowserModel* model,
    uint32_t index) {
    return fliplock_pswrd_browser_create_action_labels[index];
}

static uint32_t fliplock_pswrd_browser_action_count(const FlipLockPswrdBrowserModel* model) {
    if(fliplock_pswrd_browser_selected_is_password(model)) {
        return fliplock_pswrd_browser_password_action_count();
    }
    return fliplock_pswrd_browser_create_action_count();
}

static uint32_t fliplock_pswrd_browser_clamp_action_index_for_model(
    const FlipLockPswrdBrowserModel* model,
    uint32_t index) {
    const uint32_t action_count = fliplock_pswrd_browser_action_count(model);
    if(action_count > 0U && index < action_count) {
        return index;
    }
    return 0U;
}

static bool fliplock_pswrd_browser_selected_is_password(const FlipLockPswrdBrowserModel* model) {
    return model->item_count > 0U && model->selected_index < model->item_count &&
           model->items[model->selected_index].type == FlipLockPswrdBrowserItemPassword;
}

static bool fliplock_pswrd_browser_selected_is_add(const FlipLockPswrdBrowserModel* model) {
    return model->item_count > 0U && model->selected_index < model->item_count &&
           model->items[model->selected_index].type == FlipLockPswrdBrowserItemAdd;
}

static bool fliplock_pswrd_browser_selected_is_folder(const FlipLockPswrdBrowserModel* model) {
    return model->item_count > 0U && model->selected_index < model->item_count &&
           model->items[model->selected_index].type == FlipLockPswrdBrowserItemFolder;
}

static bool fliplock_pswrd_browser_selected_is_actionable(const FlipLockPswrdBrowserModel* model) {
    return model->item_count > 0U && model->selected_index < model->item_count;
}

static uint32_t fliplock_pswrd_browser_window_offset(const FlipLockPswrdBrowserModel* model) {
    if(model->item_count <= FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS ||
       model->selected_index < FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS) {
        return 0U;
    }
    uint32_t max_offset = model->item_count - FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS;
    uint32_t offset = model->selected_index - FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS + 1U;
    if(model->selected_index > max_offset) {
        return max_offset;
    }
    return offset;
}

static bool fliplock_pswrd_browser_icon_needs_file(const FlipLockPswrdBrowserItem* item) {
    return item->icon_id < FLIPLOCK_PSWRD_BROWSER_ICON_COUNT &&
           (item->type == FlipLockPswrdBrowserItemPassword ||
            (item->type == FlipLockPswrdBrowserItemFolder && item->icon_id != 0U));
}

static const uint8_t*
    fliplock_pswrd_browser_cached_icon(const FlipLockPswrdBrowserModel* model, uint8_t icon_id) {
    for(uint32_t slot = 0U; slot < FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS; slot++) {
        if(model->items[slot].icon_id == icon_id) {
            return model->icon_xbm[slot];
        }
    }
    return NULL;
}

static void fliplock_pswrd_browser_draw_item_icon(
    Canvas* canvas,
    const FlipLockPswrdBrowserModel* model,
    const FlipLockPswrdBrowserItem* item,
    uint8_t x,
    uint8_t y) {
    const uint8_t* xbm = fliplock_pswrd_browser_icon_needs_file(item) ?
                             fliplock_pswrd_browser_cached_icon(model, item->icon_id) :
                             NULL;

    if(xbm) {
        canvas_draw_xbm(
            canvas, x, y, FLIPLOCK_PSWRD_BROWSER_ICON_SIZE, FLIPLOCK_PSWRD_BROWSER_ICON_SIZE, xbm);
        return;
    }

    switch(item->type) {
    case FlipLockPswrdBrowserItemFolder:
        canvas_draw_icon(canvas, x, y, &I_dir_10px);
        break;
    case FlipLockPswrdBrowserItemPassword:
        canvas_draw_icon(canvas, x, y, &I_C00_Password);
        break;
    case FlipLockPswrdBrowserItemUp:
        canvas_draw_icon(canvas, x, y, &I_back_10px);
        break;
    case FlipLockPswrdBrowserItemAdd:
        canvas_draw_icon(canvas, x, y, &I_Add);
        break;

    default:
        break;
    }
}

static void
    fliplock_pswrd_browser_draw_list(Canvas* canvas, const FlipLockPswrdBrowserModel* model) {
    const bool show_scrollbar = model->item_count > FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS;
    const uint32_t selected = (model->item_count > 0U) ? model->selected_index : 0U;
    const uint32_t window_offset = fliplock_pswrd_browser_window_offset(model);

    canvas_set_font(canvas, FontSecondary);

    for(uint32_t row = 0U; row < FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS; row++) {
        const uint32_t item_index = window_offset + row;
        const uint32_t y = FLIPLOCK_PSWRD_BROWSER_LIST_Y + row * FLIPLOCK_PSWRD_BROWSER_ROW_HEIGHT;
        const bool is_selected = item_index < model->item_count && item_index == selected;

        if(item_index >= model->item_count) {
            continue;
        }

        if(is_selected) {
            canvas_set_color(canvas, ColorBlack);
            elements_slightly_rounded_box(
                canvas, 0, y, show_scrollbar ? 122U : 127U, FLIPLOCK_PSWRD_BROWSER_ROW_HEIGHT);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        furi_string_set_str(model->draw_string, model->items[item_index].label);
        const FlipLockPswrdBrowserItemType type = model->items[item_index].type;
        uint8_t label_x = 4U;
        uint8_t label_width = show_scrollbar ? 116U : 121U;

        if(type == FlipLockPswrdBrowserItemUp || type == FlipLockPswrdBrowserItemAdd ||
           type == FlipLockPswrdBrowserItemFolder || type == FlipLockPswrdBrowserItemPassword) {
            fliplock_pswrd_browser_draw_item_icon(
                canvas,
                model,
                &model->items[item_index],
                FLIPLOCK_PSWRD_BROWSER_ICON_X,
                (uint8_t)y);
            label_x = FLIPLOCK_PSWRD_BROWSER_ICON_X;
            label_width = show_scrollbar ? 103U : 108U;
        }

        elements_scrollable_text_line(
            canvas,
            label_x,
            (uint8_t)(y + 9U),
            label_width,
            model->draw_string,
            is_selected ? model->scroll_counter : 0U,
            true);
    }

    canvas_set_color(canvas, ColorBlack);
    if(show_scrollbar) {
        elements_scrollbar_pos(
            canvas,
            126,
            FLIPLOCK_PSWRD_BROWSER_LIST_Y,
            FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS * FLIPLOCK_PSWRD_BROWSER_ROW_HEIGHT - 1U,
            selected,
            model->item_count);
    }

    if(model->item_count == 0U) {
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, "Empty");
    }
}

static void
    fliplock_pswrd_browser_draw_action_menu(Canvas* canvas, FlipLockPswrdBrowserModel* model) {
    // Implementation for drawing the action menu goes here
    const uint8_t box_x = 60U;
    const uint8_t box_y = 0U;
    const uint8_t box_w = 68U;
    const uint8_t box_h = 50U;
    const bool pswrd_menu = fliplock_pswrd_browser_selected_is_password(model);
    const bool create_menu = fliplock_pswrd_browser_selected_is_add(model);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, box_x - 1U, box_y - 1U, box_w - 2U, box_h - 2U);
    canvas_set_color(canvas, ColorBlack);
    elements_slightly_rounded_frame(canvas, box_x, box_y, box_w, box_h);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, box_x + 4U, box_y + 4U, AlignLeft, AlignTop, pswrd_menu ? "Options" : "Create");

    canvas_draw_line(canvas, box_x + 1U, box_y + 12U, box_x + box_w - 2U, box_y + 12U);

    canvas_set_font(canvas, FontSecondary);
    for(uint32_t i = 0U; i < fliplock_pswrd_browser_action_count(model); i++) {
        const uint8_t row_y = (uint8_t)(15U + i * 9U);
        const bool is_selected = model->action_selected_index == i;
        const char* label = pswrd_menu ? fliplock_pswrd_browser_password_action_labels[i] :
                                         fliplock_pswrd_browser_create_action_label(model, i);

        if(is_selected) {
            canvas_draw_str(canvas, box_x + 4U, row_y + 7U, ">");
        }

        canvas_draw_str(canvas, box_x + 11U, row_y + 7U, label);
    }

    elements_button_center(canvas, "Select");
}

static void fliplock_pswrd_browser_draw_buttons(Canvas* canvas, FlipLockPswrdBrowserModel* model) {
    if(model->action_menu_open) {
        fliplock_pswrd_browser_draw_action_menu(canvas, model);
        return;
    }

    if(model->has_parent) {
        elements_button_left(canvas, "Back");
    }

    if(fliplock_pswrd_browser_selected_is_folder(model)) {
        elements_button_right(canvas, "Open");
    }

    if(fliplock_pswrd_browser_selected_is_password(model)) {
        elements_button_right(canvas, "Options");
    }

    if(fliplock_pswrd_browser_selected_is_add(model)) {
        elements_button_right(canvas, "Create");
    }
}

static void fliplock_pswrd_browser_draw_callback(Canvas* canvas, void* _model) {
    FlipLockPswrdBrowserModel* model = _model;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    furi_string_set_str(model->draw_string, model->header);
    elements_scrollable_text_line(
        canvas, 2, 8, 124U, model->draw_string, model->scroll_counter, true);

    fliplock_pswrd_browser_draw_list(canvas, model);
    fliplock_pswrd_browser_draw_buttons(canvas, model);
}

static void
    fliplock_pswrd_browser_emit(FlipLockPswrdBrowser* browser, FlipLockPswrdBrowserEvent event) {
    furi_assert(browser);

    if(browser->callback != NULL) {
        browser->callback(event, browser->callback_context);
    }
}

static void fliplock_pswrd_browser_sync_icons(FlipLockPswrdBrowser* browser) {
    uint8_t wanted[FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS];
    uint8_t missing[FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS];
    uint8_t loaded[FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS][FLIPLOCK_PSWRD_BROWSER_ICON_BYTES];
    uint8_t wanted_count = 0;
    uint8_t missing_count = 0;
    uint8_t loaded_count = 0;

    if(browser->icon_file_unavailable) {
        return;
    }

    const FlipLockPswrdBrowserModel* model = view_get_model(browser->view);
    const uint32_t offset = fliplock_pswrd_browser_window_offset(model);

    for(uint32_t rows = 0;
        rows < FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS && offset + rows < model->item_count;
        rows++) {
        const FlipLockPswrdBrowserItem* item = &model->items[offset + rows];
        if(!fliplock_pswrd_browser_icon_needs_file(item) ||
           memchr(wanted, item->icon_id, wanted_count) != NULL) {
            continue;
        }

        wanted[wanted_count++] = item->icon_id;

        if(fliplock_pswrd_browser_cached_icon(model, item->icon_id) == NULL) {
            missing[missing_count++] = item->icon_id;
        }
    }

    view_commit_model(browser->view, false);

    if(missing_count == 0U) {
        return;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(storage_file_open(file, FLIPLOCK_PSWRD_BROWSER_ICON_FILE, FSAM_READ, FSOM_OPEN_EXISTING) &&
       storage_file_size(file) ==
           (uint16_t)FLIPLOCK_PSWRD_BROWSER_ICON_COUNT * FLIPLOCK_PSWRD_BROWSER_ICON_BYTES) {
        while(
            loaded_count < missing_count &&
            storage_file_seek(
                file, (uint32_t)missing[loaded_count] * FLIPLOCK_PSWRD_BROWSER_ICON_BYTES, true) &&
            storage_file_read(file, loaded[loaded_count], FLIPLOCK_PSWRD_BROWSER_ICON_BYTES) ==
                FLIPLOCK_PSWRD_BROWSER_ICON_BYTES) {
            loaded_count++;
        }
        if(loaded_count != missing_count) {
            browser->icon_file_unavailable = true;
        }
    } else {
        browser->icon_file_unavailable = true;
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if(loaded_count == 0U) {
        return;
    }

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * mutable_model,
        {
            uint32_t slot = 0U;
            for(uint8_t i = 0U; i < loaded_count; i++) {
                while(slot < FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS &&
                      memchr(wanted, mutable_model->icon_ids[slot], wanted_count) != NULL) {
                    slot++;
                }
                if(slot == FLIPLOCK_PSWRD_BROWSER_VISIBLE_ROWS) {
                    break;
                }
                mutable_model->icon_ids[slot] = missing[i];
                memcpy(
                    mutable_model->icon_xbm[slot], loaded[i], FLIPLOCK_PSWRD_BROWSER_ICON_BYTES);
            }
        },
        true);
}

static void fliplock_pswrd_browser_move_selection(FlipLockPswrdBrowser* browser, int8_t delta) {
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            if(model->action_menu_open) {
                const uint32_t action_count = fliplock_pswrd_browser_action_count(model);
                const uint32_t current = fliplock_pswrd_browser_clamp_action_index_for_model(
                    model, model->action_selected_index);
                model->action_selected_index = (delta > 0) ?
                                                   ((current + 1U) % action_count) :
                                                   ((current + action_count - 1U) % action_count);
            } else if(model->item_count > 0U) {
                const uint32_t count = model->item_count;
                const uint32_t current = (model->selected_index < count) ? model->selected_index :
                                                                           0U;
                model->selected_index = (delta > 0) ? ((current + 1U) % count) :
                                                      ((current + count - 1U) % count);
            }
            model->scroll_counter = 0U;
        },
        true);
    fliplock_pswrd_browser_sync_icons(browser);
}

static bool fliplock_pswrd_browser_input_callback(InputEvent* event, void* context) {
    FlipLockPswrdBrowser* browser = context;
    bool consumed = false;

    furi_assert(browser);
    furi_assert(event);

    if((event->key == InputKeyUp || event->key == InputKeyDown) &&
       (event->type == InputTypeShort || event->type == InputTypeRepeat)) {
        fliplock_pswrd_browser_move_selection(browser, event->key == InputKeyDown ? 1 : -1);
        return true;
    }

    FlipLockPswrdBrowserModel* model = view_get_model(browser->view);
    const bool action_menu_open = model->action_menu_open;
    const bool selected_folder = fliplock_pswrd_browser_selected_is_folder(model);
    const bool selected_password = fliplock_pswrd_browser_selected_is_password(model);
    const bool selected_up = model->item_count > 0U && model->selected_index < model->item_count &&
                             model->items[model->selected_index].type ==
                                 FlipLockPswrdBrowserItemUp;
    const bool selected_add = fliplock_pswrd_browser_selected_is_add(model);
    const bool selected_actionable = fliplock_pswrd_browser_selected_is_actionable(model);
    const bool has_parent = model->has_parent;
    view_commit_model(browser->view, false);

    if(event->key == InputKeyOk && selected_actionable && !action_menu_open) {
        if(selected_password || selected_add) {
            with_view_model(
                browser->view,
                FlipLockPswrdBrowserModel * mutable_model,
                { mutable_model->action_menu_open = true; },
                true);
        }
        fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventOpenActionMenu);
        return true;
    }

    if(action_menu_open) {
        if(event->key == InputKeyOk) {
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventSelectAction);
            return true;
        }

        if(event->key == InputKeyBack) {
            with_view_model(
                browser->view,
                FlipLockPswrdBrowserModel * mutable_model,
                { mutable_model->action_menu_open = false; },
                true);
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventCloseActionMenu);
            return true;
        }

        return false;
    }

    if(event->key == InputKeyBack && browser->back_filter != NULL &&
       browser->back_filter(browser->callback_context)) {
        return true;
    }

    switch(event->key) {
    case InputKeyLeft:
        if(has_parent) {
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventBack);
            consumed = true;
        }
        break;
    case InputKeyOk:
        if(selected_add || selected_password || selected_folder || selected_up) {
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventEnter);
            consumed = true;
        }
        break;
    case InputKeyRight:
        if(selected_folder) {
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventEnter);
            consumed = true;
        } else if(selected_password) {
            with_view_model(
                browser->view,
                FlipLockPswrdBrowserModel * mutable_model,
                { mutable_model->action_menu_open = true; },
                true);
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventOpenActionMenu);
            consumed = true;
            break;
        }
    case InputKeyBack:
        if(has_parent) {
            fliplock_pswrd_browser_emit(browser, FlipLockPswrdBrowserEventBack);
            consumed = true;
        }
        break;
    default:
        break;
    }

    return consumed;
}

static void fliplock_pswrd_browser_timer_callback(void* context) {
    FlipLockPswrdBrowser* browser = context;
    furi_assert(browser);

    with_view_model(
        browser->view, FlipLockPswrdBrowserModel * model, { model->scroll_counter++; }, true);
}

static void fliplock_pswrd_browser_enter_callback(void* context) {
    FlipLockPswrdBrowser* browser = context;
    furi_assert(browser);

    with_view_model(
        browser->view, FlipLockPswrdBrowserModel * model, { model->scroll_counter = 0U; }, true);
    furi_timer_start(browser->scroll_timer, FLIPLOCK_PSWRD_BROWSER_SCROLL_INTERVAL_MS);
}

static void fliplock_pswrd_browser_exit_callback(void* context) {
    FlipLockPswrdBrowser* browser = context;
    furi_assert(browser);
    furi_timer_stop(browser->scroll_timer);
}

FlipLockPswrdBrowser* fliplock_pswrd_browser_alloc(void) {
    FlipLockPswrdBrowser* browser = malloc(sizeof(FlipLockPswrdBrowser));
    browser->view = view_alloc();
    browser->scroll_timer =
        furi_timer_alloc(fliplock_pswrd_browser_timer_callback, FuriTimerTypePeriodic, browser);
    browser->callback = NULL;
    browser->back_filter = NULL;
    browser->callback_context = NULL;
    browser->icon_file_unavailable = false;

    view_set_context(browser->view, browser);
    view_allocate_model(browser->view, ViewModelTypeLocking, sizeof(FlipLockPswrdBrowserModel));
    view_set_draw_callback(browser->view, fliplock_pswrd_browser_draw_callback);
    view_set_input_callback(browser->view, fliplock_pswrd_browser_input_callback);
    view_set_enter_callback(browser->view, fliplock_pswrd_browser_enter_callback);
    view_set_exit_callback(browser->view, fliplock_pswrd_browser_exit_callback);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            model->draw_string = furi_string_alloc();
            memset(model->icon_ids, FLIPLOCK_PSWRD_BROWSER_ICON_NONE, sizeof(model->icon_ids));
        },
        false);

    fliplock_pswrd_browser_reset(browser);
    return browser;
}

void fliplock_pswrd_browser_free(FlipLockPswrdBrowser* browser) {
    furi_check(browser);

    furi_timer_stop(browser->scroll_timer);
    furi_timer_free(browser->scroll_timer);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            if(model->draw_string != NULL) {
                furi_string_free(model->draw_string);
                model->draw_string = NULL;
            }
        },
        false);
    view_free(browser->view);
    free(browser);
}

View* fliplock_pswrd_browser_get_view(FlipLockPswrdBrowser* browser) {
    furi_check(browser);
    return browser->view;
}

void fliplock_pswrd_browser_set_callback(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserCallback callback,
    void* context) {
    furi_check(browser);
    browser->callback = callback;
    browser->callback_context = context;
}

void fliplock_pswrd_browser_set_back_filter(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserBackFilter filter) {
    furi_check(browser);
    browser->back_filter = filter;
}

void fliplock_pswrd_browser_reset(FlipLockPswrdBrowser* browser) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            model->header[0] = '\0';
            memset(model->items, 0, sizeof(model->items));
            model->item_count = 0U;
            model->selected_index = 0U;
            model->action_selected_index = 0U;
            model->scroll_counter = 0U;
            model->has_parent = false;
            model->action_menu_open = false;
            model->mode = FlipLockPswrdBrowserModeBrowse;
            model->menu_type = FlipLockPswrdBrowserMenuTypeObject;
        },
        true);
}

void fliplock_pswrd_browser_set_header(FlipLockPswrdBrowser* browser, const char* header) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        { snprintf(model->header, sizeof(model->header), "%s", header != NULL ? header : ""); },
        true);
}

void fliplock_pswrd_browser_set_has_parent(FlipLockPswrdBrowser* browser, bool has_parent) {
    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        { model->has_parent = has_parent; },
        true);
}

void fliplock_pswrd_browser_set_mode(FlipLockPswrdBrowser* browser, FlipLockPswrdBrowserMode mode) {
    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            model->mode = mode;
            model->action_menu_open = false;
            model->scroll_counter = 0U;
        },
        true);
}

void fliplock_pswrd_browser_set_add_menu_type(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserMenuType kind) {
    furi_check(browser);
    with_view_model(
        browser->view, FlipLockPswrdBrowserModel * model, { model->menu_type = kind; }, true);
}

void fliplock_pswrd_browser_add_item(
    FlipLockPswrdBrowser* browser,
    FlipLockPswrdBrowserItemType type,
    const char* label,
    uint8_t icon_id) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            if(model->item_count < FLIPLOCK_PASSWORD_BROWSER_MAX_ITEMS) {
                FlipLockPswrdBrowserItem* item = &model->items[model->item_count++];
                item->type = type;
                item->icon_id = icon_id;
                snprintf(item->label, sizeof(item->label), "%s", label != NULL ? label : "");
            }
        },
        true);
}

void fliplock_pswrd_browser_set_selected_item(FlipLockPswrdBrowser* browser, uint32_t index) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            model->selected_index =
                (model->item_count == 0U) ? 0U : ((index < model->item_count) ? index : 0U);
            model->scroll_counter = 0U;
        },
        true);
    /* Every list rebuild ends here, so the first frame already has its icons. */
    fliplock_pswrd_browser_sync_icons(browser);
}

uint32_t fliplock_pswrd_browser_get_selected_item(const FlipLockPswrdBrowser* browser) {
    uint32_t selected = 0U;

    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        { selected = model->selected_index; },
        false);
    return selected;
}

void fliplock_pswrd_browser_set_action_selected(FlipLockPswrdBrowser* browser, uint32_t index) {
    furi_check(browser);

    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            model->action_selected_index =
                fliplock_pswrd_browser_clamp_action_index_for_model(model, index);
        },
        true);
}

uint32_t fliplock_pswrd_browser_get_action_selected(const FlipLockPswrdBrowser* browser) {
    uint32_t selected = 0U;

    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            selected = fliplock_pswrd_browser_clamp_action_index_for_model(
                model, model->action_selected_index);
        },
        false);
    return selected;
}

void fliplock_pswrd_browser_set_action_menu_open(FlipLockPswrdBrowser* browser, bool open) {
    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        { model->action_menu_open = open; },
        true);
}

bool fliplock_pswrd_browser_is_action_menu_open(const FlipLockPswrdBrowser* browser) {
    bool open = false;

    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        { open = model->action_menu_open; },
        false);
    return open;
}

FlipLockPswrdBrowserItemType
    fliplock_pswrd_browser_get_selected_type(const FlipLockPswrdBrowser* browser) {
    FlipLockPswrdBrowserItemType type = FlipLockPswrdBrowserItemAdd;

    furi_check(browser);
    with_view_model(
        browser->view,
        FlipLockPswrdBrowserModel * model,
        {
            if(model->item_count > 0U && model->selected_index < model->item_count) {
                type = model->items[model->selected_index].type;
            }
        },
        false);
    return type;
}
