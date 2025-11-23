/**
 * BitChat Nickname Setup View Implementation
 */

#include "nickname_view.h"
#include <gui/view.h>
#include <furi.h>
#include <string.h>

#define MAX_NICKNAME_LENGTH 32

struct NicknameView {
    TextInput* text_input;
    char nickname_buffer[MAX_NICKNAME_LENGTH];
    NicknameViewCallback callback;
    void* callback_context;
};

/**
 * Text input callback
 */
static void nickname_view_text_input_callback(void* context) {
    NicknameView* nickname_view = context;

    if(nickname_view->callback) {
        nickname_view->callback(nickname_view->callback_context, nickname_view->nickname_buffer);
    }
}

/**
 * Allocate nickname view
 */
NicknameView* nickname_view_alloc(void) {
    NicknameView* nickname_view = malloc(sizeof(NicknameView));
    memset(nickname_view, 0, sizeof(NicknameView));

    nickname_view->text_input = text_input_alloc();

    text_input_set_header_text(nickname_view->text_input, "Enter your nickname:");
    text_input_set_result_callback(
        nickname_view->text_input,
        nickname_view_text_input_callback,
        nickname_view,
        nickname_view->nickname_buffer,
        MAX_NICKNAME_LENGTH,
        true);  // Clear default text

    return nickname_view;
}

/**
 * Free nickname view
 */
void nickname_view_free(NicknameView* nickname_view) {
    furi_assert(nickname_view);
    text_input_free(nickname_view->text_input);
    free(nickname_view);
}

/**
 * Get the view
 */
View* nickname_view_get_view(NicknameView* nickname_view) {
    furi_assert(nickname_view);
    return text_input_get_view(nickname_view->text_input);
}

/**
 * Set callback
 */
void nickname_view_set_callback(NicknameView* nickname_view, NicknameViewCallback callback, void* context) {
    furi_assert(nickname_view);
    nickname_view->callback = callback;
    nickname_view->callback_context = context;
}

/**
 * Set nickname (for editing existing)
 */
void nickname_view_set_nickname(NicknameView* nickname_view, const char* nickname) {
    furi_assert(nickname_view);
    furi_assert(nickname);

    strncpy(nickname_view->nickname_buffer, nickname, MAX_NICKNAME_LENGTH - 1);
    nickname_view->nickname_buffer[MAX_NICKNAME_LENGTH - 1] = '\0';
}

/**
 * Reset the view
 */
void nickname_view_reset(NicknameView* nickname_view) {
    furi_assert(nickname_view);
    memset(nickname_view->nickname_buffer, 0, MAX_NICKNAME_LENGTH);
}
