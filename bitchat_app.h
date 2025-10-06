/**
 * BitChat for Flipper Zero
 * Main application header
 */

#pragma once

#include <furi.h>

typedef struct BitchatApp BitchatApp;
typedef struct BitchatBle BitchatBle;
typedef struct BitchatIdentity BitchatIdentity;

/**
 * Event types for BitChat
 */
typedef enum {
    BitchatEventTypeMessage,
    BitchatEventTypePeerConnected,
    BitchatEventTypePeerDisconnected,
    BitchatEventTypeExit,
} BitchatEventType;

/**
 * Message event data
 */
typedef struct {
    char sender[32];
    char content[256];
    uint32_t timestamp;
    bool is_private;
} BitchatMessageEvent;

/**
 * Peer event data
 */
typedef struct {
    char nickname[32];
    uint8_t peer_id[8];
} BitchatPeerEvent;

/**
 * BitChat event
 */
typedef struct {
    BitchatEventType type;
    union {
        BitchatMessageEvent message;
        BitchatPeerEvent peer;
    } data;
} BitchatEvent;

/**
 * Identity management
 */
BitchatIdentity* bitchat_identity_create(void);
BitchatIdentity* bitchat_identity_load(void);
void bitchat_identity_save(BitchatIdentity* identity);
void bitchat_identity_free(BitchatIdentity* identity);
bool bitchat_identity_get_nickname(BitchatIdentity* identity, char* nickname, size_t size);
void bitchat_identity_set_nickname(BitchatIdentity* identity, const char* nickname);
const uint8_t* bitchat_identity_get_public_key(BitchatIdentity* identity);
const uint8_t* bitchat_identity_get_peer_id(BitchatIdentity* identity);
