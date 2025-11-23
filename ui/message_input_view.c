/**
 * BitChat Message Input View Implementation
 */

#include "message_input_view.h"
#include <gui/view.h>
#include <furi.h>
#include <string.h>

#define MAX_MESSAGE_LENGTH 256

struct MessageInputView {
    TextInput* text_input;
    char message_buffer[MAX_MESSAGE_LENGTH];
    MessageInputViewCallback callback;
    void* callback_context;
};

/**
 * Text input callback
 */
static void message_input_view_text_input_callback(void* context) {
    MessageInputView* message_input_view = context;

    if(message_input_view->callback && message_input_view->message_buffer[0] != '\0') {
        message_input_view->callback(
            message_input_view->callback_context,
            message_input_view->message_buffer);

        // Clear buffer after sending
        memset(message_input_view->message_buffer, 0, MAX_MESSAGE_LENGTH);
    }
}

/**
 * Allocate message input view
 */
MessageInputView* message_input_view_alloc(void) {
    MessageInputView* message_input_view = malloc(sizeof(MessageInputView));
    memset(message_input_view, 0, sizeof(MessageInputView));

    message_input_view->text_input = text_input_alloc();

    text_input_set_header_text(message_input_view->text_input, "Enter message:");
    text_input_set_result_callback(
        message_input_view->text_input,
        message_input_view_text_input_callback,
        message_input_view,
        message_input_view->message_buffer,
        MAX_MESSAGE_LENGTH,
        true);  // Clear default text

    return message_input_view;
}

/**
 * Free message input view
 */
void message_input_view_free(MessageInputView* message_input_view) {
    furi_assert(message_input_view);
    text_input_free(message_input_view->text_input);
    free(message_input_view);
}

/**
 * Get the view
 */
View* message_input_view_get_view(MessageInputView* message_input_view) {
    furi_assert(message_input_view);
    return text_input_get_view(message_input_view->text_input);
}

/**
 * Set callback
 */
void message_input_view_set_callback(
    MessageInputView* message_input_view,
    MessageInputViewCallback callback,
    void* context) {
    furi_assert(message_input_view);
    message_input_view->callback = callback;
    message_input_view->callback_context = context;
}

/**
 * Reset the view
 */
void message_input_view_reset(MessageInputView* message_input_view) {
    furi_assert(message_input_view);
    memset(message_input_view->message_buffer, 0, MAX_MESSAGE_LENGTH);
}
