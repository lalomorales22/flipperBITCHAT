/**
 * BitChat for Flipper Zero
 * Made by lalo with <3 for jack
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>

#define TAG "BitChat"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Widget* widget;
} BitchatApp;

static BitchatApp* bitchat_app_alloc() {
    BitchatApp* app = malloc(sizeof(BitchatApp));

    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Initialize widget
    app->widget = widget_alloc();

    widget_add_string_element(
        app->widget,
        64, 10,
        AlignCenter, AlignTop,
        FontPrimary,
        "BitChat"
    );

    widget_add_string_element(
        app->widget,
        64, 24,
        AlignCenter, AlignTop,
        FontSecondary,
        "Bluetooth Mesh Chat"
    );

    widget_add_string_element(
        app->widget,
        64, 38,
        AlignCenter, AlignTop,
        FontSecondary,
        "Made by lalo <3 for jack"
    );

    widget_add_string_element(
        app->widget,
        64, 52,
        AlignCenter, AlignTop,
        FontSecondary,
        "Press Back to exit"
    );

    // Add widget to view dispatcher
    view_dispatcher_add_view(app->view_dispatcher, 0, widget_get_view(app->widget));
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void bitchat_app_free(BitchatApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t bitchat_app(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "BitChat starting...");

    BitchatApp* app = bitchat_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    bitchat_app_free(app);

    FURI_LOG_I(TAG, "BitChat stopped");
    return 0;
}
