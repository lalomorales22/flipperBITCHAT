/**
 * BitChat Message Input View
 * For composing and sending messages
 */

#pragma once

#include <gui/modules/text_input.h>

typedef struct MessageInputView MessageInputView;

/**
 * Callback when message is submitted
 */
typedef void (*MessageInputViewCallback)(void* context, const char* message);

/**
 * Allocate message input view
 */
MessageInputView* message_input_view_alloc(void);

/**
 * Free message input view
 */
void message_input_view_free(MessageInputView* message_input_view);

/**
 * Get the view
 */
View* message_input_view_get_view(MessageInputView* message_input_view);

/**
 * Set callback
 */
void message_input_view_set_callback(
    MessageInputView* message_input_view,
    MessageInputViewCallback callback,
    void* context);

/**
 * Reset the view (clear input)
 */
void message_input_view_reset(MessageInputView* message_input_view);
