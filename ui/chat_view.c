/**
 * BitChat Chat View Implementation
 */

#include "chat_view.h"
#include <gui/elements.h>
#include <furi.h>
#include <string.h>

#define TAG "ChatView"
#define MAX_MESSAGES 50
#define MESSAGE_DISPLAY_LINES 5
#define MAX_LINE_LENGTH 22  // Flipper screen is ~128 pixels wide

typedef struct {
    char sender[32];
    char content[128];
    bool is_own;
    uint32_t timestamp;
} ChatMessage;

typedef struct {
    ChatMessage messages[MAX_MESSAGES];
    size_t message_count;
    size_t scroll_offset;
    uint8_t peer_count;
    bool is_connected;
    char local_nickname[32];
    ChatViewCallback callback;
    void* callback_context;
} ChatViewModel;

struct ChatView {
    View* view;
};

/**
 * Draw callback for chat view
 */
static void chat_view_draw_callback(Canvas* canvas, void* model) {
    ChatViewModel* vm = model;

    // Clear screen
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header bar with status
    canvas_draw_frame(canvas, 0, 0, 128, 12);

    // Connection status icon
    if(vm->is_connected) {
        // Draw filled circle for connected
        canvas_draw_disc(canvas, 6, 6, 3);
    } else {
        // Draw empty circle for disconnected
        canvas_draw_circle(canvas, 6, 6, 3);
    }

    // Title
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 12, 9, "BitChat");

    // Peer count
    char peer_str[16];
    snprintf(peer_str, sizeof(peer_str), "%d peer%s", vm->peer_count, vm->peer_count == 1 ? "" : "s");
    canvas_draw_str_aligned(canvas, 125, 9, AlignRight, AlignBottom, peer_str);

    // Message area
    if(vm->message_count == 0) {
        // No messages yet
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignCenter, "No messages yet");
        canvas_draw_str_aligned(canvas, 64, 46, AlignCenter, AlignCenter, "Press OK to send");
    } else {
        // Display messages with scrolling
        canvas_set_font(canvas, FontSecondary);

        size_t start_idx = vm->scroll_offset;
        size_t end_idx = vm->message_count;
        if(end_idx - start_idx > MESSAGE_DISPLAY_LINES) {
            end_idx = start_idx + MESSAGE_DISPLAY_LINES;
        }

        uint8_t y_pos = 22;
        for(size_t i = start_idx; i < end_idx; i++) {
            ChatMessage* msg = &vm->messages[i];

            // Format: "sender: message" or "You: message"
            char line[64];
            const char* display_sender = msg->is_own ? "You" : msg->sender;

            // Truncate message if needed
            char truncated_msg[40];
            strncpy(truncated_msg, msg->content, sizeof(truncated_msg) - 1);
            truncated_msg[sizeof(truncated_msg) - 1] = '\0';

            snprintf(line, sizeof(line), "%s: %s", display_sender, truncated_msg);

            // Highlight own messages
            if(msg->is_own) {
                canvas_draw_str(canvas, 2, y_pos, ">");
                canvas_draw_str(canvas, 8, y_pos, line);
            } else {
                canvas_draw_str(canvas, 2, y_pos, line);
            }

            y_pos += 10;
        }

        // Scroll indicators
        if(vm->scroll_offset > 0) {
            // Can scroll up
            canvas_draw_str_aligned(canvas, 64, 14, AlignCenter, AlignBottom, "^");
        }
        if(end_idx < vm->message_count) {
            // Can scroll down
            canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, "v");
        }
    }

    // Footer help text
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_frame(canvas, 0, 54, 128, 10);
    canvas_draw_str_aligned(canvas, 64, 61, AlignCenter, AlignBottom, "OK=Send Back=Menu");
}

/**
 * Input callback for chat view
 */
static bool chat_view_input_callback(InputEvent* event, void* context) {
    ChatView* chat_view = context;
    ChatViewModel* model = view_get_model(chat_view->view);
    bool consumed = false;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
            case InputKeyUp:
                // Scroll up
                if(model->scroll_offset > 0) {
                    model->scroll_offset--;
                    consumed = true;
                }
                break;

            case InputKeyDown:
                // Scroll down
                if(model->scroll_offset + MESSAGE_DISPLAY_LINES < model->message_count) {
                    model->scroll_offset++;
                    consumed = true;
                }
                break;

            case InputKeyOk:
                // Open text input
                if(model->callback) {
                    model->callback(model->callback_context, 0);
                }
                consumed = true;
                break;

            case InputKeyBack:
                // Return to menu (handled by view dispatcher)
                consumed = false;
                break;

            default:
                break;
        }
    }

    if(consumed) {
        view_commit_model(chat_view->view, true);
    }

    return consumed;
}

/**
 * Allocate chat view
 */
ChatView* chat_view_alloc(void) {
    ChatView* chat_view = malloc(sizeof(ChatView));

    chat_view->view = view_alloc();
    view_allocate_model(chat_view->view, ViewModelTypeLocking, sizeof(ChatViewModel));
    view_set_context(chat_view->view, chat_view);
    view_set_draw_callback(chat_view->view, chat_view_draw_callback);
    view_set_input_callback(chat_view->view, chat_view_input_callback);

    // Initialize model
    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            memset(model, 0, sizeof(ChatViewModel));
            model->is_connected = false;
            model->peer_count = 0;
            model->message_count = 0;
            model->scroll_offset = 0;
            strncpy(model->local_nickname, "You", sizeof(model->local_nickname) - 1);
        },
        true);

    return chat_view;
}

/**
 * Free chat view
 */
void chat_view_free(ChatView* chat_view) {
    furi_assert(chat_view);
    view_free(chat_view->view);
    free(chat_view);
}

/**
 * Get the view
 */
View* chat_view_get_view(ChatView* chat_view) {
    furi_assert(chat_view);
    return chat_view->view;
}

/**
 * Set callback
 */
void chat_view_set_callback(ChatView* chat_view, ChatViewCallback callback, void* context) {
    furi_assert(chat_view);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            model->callback = callback;
            model->callback_context = context;
        },
        false);
}

/**
 * Add a message
 */
void chat_view_add_message(ChatView* chat_view, const char* sender, const char* message, bool is_own) {
    furi_assert(chat_view);
    furi_assert(sender);
    furi_assert(message);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            // Add message to ring buffer
            if(model->message_count < MAX_MESSAGES) {
                size_t idx = model->message_count;
                strncpy(model->messages[idx].sender, sender, sizeof(model->messages[idx].sender) - 1);
                strncpy(model->messages[idx].content, message, sizeof(model->messages[idx].content) - 1);
                model->messages[idx].is_own = is_own;
                model->messages[idx].timestamp = furi_get_tick();
                model->message_count++;
            } else {
                // Shift messages up (remove oldest)
                memmove(&model->messages[0], &model->messages[1], (MAX_MESSAGES - 1) * sizeof(ChatMessage));

                // Add new message at end
                size_t idx = MAX_MESSAGES - 1;
                strncpy(model->messages[idx].sender, sender, sizeof(model->messages[idx].sender) - 1);
                strncpy(model->messages[idx].content, message, sizeof(model->messages[idx].content) - 1);
                model->messages[idx].is_own = is_own;
                model->messages[idx].timestamp = furi_get_tick();
            }

            // Auto-scroll to bottom when new message arrives
            if(model->message_count > MESSAGE_DISPLAY_LINES) {
                model->scroll_offset = model->message_count - MESSAGE_DISPLAY_LINES;
            }
        },
        true);
}

/**
 * Set peer count
 */
void chat_view_set_peer_count(ChatView* chat_view, uint8_t count) {
    furi_assert(chat_view);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            model->peer_count = count;
        },
        true);
}

/**
 * Set connected status
 */
void chat_view_set_connected(ChatView* chat_view, bool connected) {
    furi_assert(chat_view);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            model->is_connected = connected;
        },
        true);
}

/**
 * Clear all messages
 */
void chat_view_clear_messages(ChatView* chat_view) {
    furi_assert(chat_view);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            model->message_count = 0;
            model->scroll_offset = 0;
        },
        true);
}

/**
 * Set local nickname
 */
void chat_view_set_nickname(ChatView* chat_view, const char* nickname) {
    furi_assert(chat_view);
    furi_assert(nickname);

    with_view_model(
        chat_view->view,
        ChatViewModel* model,
        {
            strncpy(model->local_nickname, nickname, sizeof(model->local_nickname) - 1);
            model->local_nickname[sizeof(model->local_nickname) - 1] = '\0';
        },
        false);
}
