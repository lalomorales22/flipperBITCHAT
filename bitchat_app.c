/**
 * BitChat for Flipper Zero
 * Made by lalo with <3 for jack
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include "bitchat_app.h"
#include "ui/chat_view.h"
#include "ui/nickname_view.h"
#include "ui/message_input_view.h"
#include "ble/bitchat_ble.h"
#include "protocol/bitchat_protocol.h"

#define TAG "BitChat"

// View IDs
typedef enum {
    BitchatViewChat,
    BitchatViewNickname,
    BitchatViewMessageInput,
} BitchatViewId;

struct BitchatApp {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;

    // Views
    ChatView* chat_view;
    NicknameView* nickname_view;
    MessageInputView* message_input_view;

    // Backend
    BitchatIdentity* identity;
    BitchatBle* ble;
    FuriMessageQueue* event_queue;

    // State
    bool is_running;
};

// Forward declarations
static bool bitchat_app_back_event_callback(void* context);
static void bitchat_app_chat_callback(void* context, uint32_t index);
static void bitchat_app_nickname_callback(void* context, const char* nickname);
static void bitchat_app_message_callback(void* context, const char* message);

/**
 * Back button handler
 */
static bool bitchat_app_back_event_callback(void* context) {
    BitchatApp* app = context;

    // Get current view
    uint32_t current_view = view_dispatcher_get_current_view(app->view_dispatcher);

    if(current_view == BitchatViewChat) {
        // Exit app from main chat view
        return false;  // Allow exit
    } else {
        // Return to chat view from other views
        view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewChat);
        return true;  // Consume event
    }
}

/**
 * Chat view callback - handles opening message input
 */
static void bitchat_app_chat_callback(void* context, uint32_t index) {
    UNUSED(index);
    BitchatApp* app = context;

    // Open message input
    message_input_view_reset(app->message_input_view);
    view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewMessageInput);
}

/**
 * Nickname setup callback
 */
static void bitchat_app_nickname_callback(void* context, const char* nickname) {
    BitchatApp* app = context;

    FURI_LOG_I(TAG, "Nickname set to: %s", nickname);

    // Update identity
    bitchat_identity_set_nickname(app->identity, nickname);
    bitchat_identity_save(app->identity);

    // Update chat view
    chat_view_set_nickname(app->chat_view, nickname);

    // Start BLE
    bitchat_ble_start(app->ble, app->identity);
    chat_view_set_connected(app->chat_view, true);

    // Add welcome message
    char welcome_msg[128];
    snprintf(welcome_msg, sizeof(welcome_msg),
        "Welcome to BitChat! Looking for peers...");
    chat_view_add_message(app->chat_view, "System", welcome_msg, false);

    // Switch to chat view
    view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewChat);

    // Vibrate to confirm
    notification_message(app->notifications, &sequence_success);
}

/**
 * Message input callback
 */
static void bitchat_app_message_callback(void* context, const char* message) {
    BitchatApp* app = context;

    FURI_LOG_I(TAG, "Sending message: %s", message);

    // Get nickname
    char nickname[32];
    bitchat_identity_get_nickname(app->identity, nickname, sizeof(nickname));

    // Add to chat view
    chat_view_add_message(app->chat_view, nickname, message, true);

    // TODO: Encode and send via BLE
    // For now, just show in UI

    // Return to chat
    view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewChat);

    // Vibrate to confirm send
    notification_message(app->notifications, &sequence_single_vibro);
}

/**
 * Allocate app
 */
static BitchatApp* bitchat_app_alloc() {
    BitchatApp* app = malloc(sizeof(BitchatApp));
    memset(app, 0, sizeof(BitchatApp));

    // Initialize GUI
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Create event queue
    app->event_queue = furi_message_queue_alloc(8, sizeof(BitchatEvent));

    // Load or create identity
    app->identity = bitchat_identity_load();
    if(!app->identity) {
        FURI_LOG_I(TAG, "Creating new identity");
        app->identity = bitchat_identity_create();
        bitchat_identity_save(app->identity);
    }

    // Initialize BLE
    app->ble = bitchat_ble_alloc(app->event_queue);

    // Initialize view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, bitchat_app_back_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, bitchat_app_back_event_callback);

    // Initialize views
    app->chat_view = chat_view_alloc();
    chat_view_set_callback(app->chat_view, bitchat_app_chat_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BitchatViewChat,
        chat_view_get_view(app->chat_view));

    app->nickname_view = nickname_view_alloc();
    nickname_view_set_callback(app->nickname_view, bitchat_app_nickname_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BitchatViewNickname,
        nickname_view_get_view(app->nickname_view));

    app->message_input_view = message_input_view_alloc();
    message_input_view_set_callback(app->message_input_view, bitchat_app_message_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BitchatViewMessageInput,
        message_input_view_get_view(app->message_input_view));

    // Check if we need nickname setup
    char nickname[32];
    if(!bitchat_identity_get_nickname(app->identity, nickname, sizeof(nickname))) {
        // First time setup - show nickname view
        view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewNickname);
    } else {
        // Start BLE and go to chat
        chat_view_set_nickname(app->chat_view, nickname);
        bitchat_ble_start(app->ble, app->identity);
        chat_view_set_connected(app->chat_view, bitchat_ble_is_active(app->ble));

        // Add system message
        chat_view_add_message(app->chat_view, "System", "BitChat started. Looking for peers...", false);

        view_dispatcher_switch_to_view(app->view_dispatcher, BitchatViewChat);
    }

    return app;
}

/**
 * Free app
 */
static void bitchat_app_free(BitchatApp* app) {
    furi_assert(app);

    // Stop BLE
    if(app->ble) {
        bitchat_ble_free(app->ble);
    }

    // Free identity
    if(app->identity) {
        bitchat_identity_free(app->identity);
    }

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, BitchatViewChat);
    view_dispatcher_remove_view(app->view_dispatcher, BitchatViewNickname);
    view_dispatcher_remove_view(app->view_dispatcher, BitchatViewMessageInput);

    chat_view_free(app->chat_view);
    nickname_view_free(app->nickname_view);
    message_input_view_free(app->message_input_view);

    // Free dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Free event queue
    furi_message_queue_free(app->event_queue);

    // Close records
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(app);
}

/**
 * Main app entry point
 */
int32_t bitchat_app(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "BitChat starting...");

    BitchatApp* app = bitchat_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    bitchat_app_free(app);

    FURI_LOG_I(TAG, "BitChat stopped");
    return 0;
}
