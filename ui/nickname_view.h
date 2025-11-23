/**
 * BitChat Nickname Setup View
 * For setting user nickname on first launch or in settings
 */

#pragma once

#include <gui/modules/text_input.h>

typedef struct NicknameView NicknameView;

/**
 * Callback when nickname is set
 */
typedef void (*NicknameViewCallback)(void* context, const char* nickname);

/**
 * Allocate nickname view
 */
NicknameView* nickname_view_alloc(void);

/**
 * Free nickname view
 */
void nickname_view_free(NicknameView* nickname_view);

/**
 * Get the view
 */
View* nickname_view_get_view(NicknameView* nickname_view);

/**
 * Set callback
 */
void nickname_view_set_callback(NicknameView* nickname_view, NicknameViewCallback callback, void* context);

/**
 * Set current nickname (for editing)
 */
void nickname_view_set_nickname(NicknameView* nickname_view, const char* nickname);

/**
 * Reset the view (prepare for new input)
 */
void nickname_view_reset(NicknameView* nickname_view);
