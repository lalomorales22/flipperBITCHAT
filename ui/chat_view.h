/**
 * BitChat Chat View
 * Main chat interface for displaying messages and peers
 */

#pragma once

#include <gui/view.h>
#include "../bitchat_app.h"

typedef struct ChatView ChatView;

/**
 * Callback for input events from chat view
 */
typedef void (*ChatViewCallback)(void* context, uint32_t index);

/**
 * Allocate chat view
 */
ChatView* chat_view_alloc(void);

/**
 * Free chat view
 */
void chat_view_free(ChatView* chat_view);

/**
 * Get the view
 */
View* chat_view_get_view(ChatView* chat_view);

/**
 * Set callback for view events
 */
void chat_view_set_callback(ChatView* chat_view, ChatViewCallback callback, void* context);

/**
 * Add a message to the chat view
 */
void chat_view_add_message(ChatView* chat_view, const char* sender, const char* message, bool is_own);

/**
 * Update peer count display
 */
void chat_view_set_peer_count(ChatView* chat_view, uint8_t count);

/**
 * Update connection status
 */
void chat_view_set_connected(ChatView* chat_view, bool connected);

/**
 * Clear all messages
 */
void chat_view_clear_messages(ChatView* chat_view);

/**
 * Set local nickname
 */
void chat_view_set_nickname(ChatView* chat_view, const char* nickname);
