/**
 * BitChat Identity Management
 * Handles cryptographic identity storage and retrieval
 */

#include "../bitchat_app.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <string.h>

#define TAG "BitchatIdentity"
#define IDENTITY_FILE_PATH APP_DATA_PATH("bitchat") "/identity.bin"
#define IDENTITY_VERSION 1

struct BitchatIdentity {
    uint8_t version;
    uint8_t peer_id[8];
    uint8_t noise_private_key[32];
    uint8_t noise_public_key[32];
    uint8_t signing_private_key[32];
    uint8_t signing_public_key[32];
    char nickname[32];
};

/**
 * Generate a random peer ID from public key
 */
static void generate_peer_id(const uint8_t* public_key, uint8_t* peer_id) {
    // Use first 8 bytes of public key as peer ID
    memcpy(peer_id, public_key, 8);
}

/**
 * Create a new identity
 */
BitchatIdentity* bitchat_identity_create(void) {
    BitchatIdentity* identity = malloc(sizeof(BitchatIdentity));
    memset(identity, 0, sizeof(BitchatIdentity));

    identity->version = IDENTITY_VERSION;

    // Generate Noise key pair (simplified - using random for now)
    // In production, use proper Curve25519 key generation
    for(int i = 0; i < 32; i++) {
        identity->noise_private_key[i] = furi_hal_random_get() & 0xFF;
        identity->noise_public_key[i] = furi_hal_random_get() & 0xFF;
    }

    // Generate signing key pair (simplified - using random for now)
    // In production, use proper Ed25519 key generation
    for(int i = 0; i < 32; i++) {
        identity->signing_private_key[i] = furi_hal_random_get() & 0xFF;
        identity->signing_public_key[i] = furi_hal_random_get() & 0xFF;
    }

    // Generate peer ID from public key
    generate_peer_id(identity->noise_public_key, identity->peer_id);

    // Set default nickname
    snprintf(identity->nickname, sizeof(identity->nickname),
        "flipper_%02x%02x%02x%02x",
        identity->peer_id[0], identity->peer_id[1],
        identity->peer_id[2], identity->peer_id[3]);

    FURI_LOG_I(TAG, "Created new identity: %s", identity->nickname);

    return identity;
}

/**
 * Load identity from storage
 */
BitchatIdentity* bitchat_identity_load(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    BitchatIdentity* identity = NULL;

    if(storage_file_open(file, IDENTITY_FILE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        identity = malloc(sizeof(BitchatIdentity));

        uint16_t bytes_read = storage_file_read(file, identity, sizeof(BitchatIdentity));
        if(bytes_read == sizeof(BitchatIdentity) && identity->version == IDENTITY_VERSION) {
            FURI_LOG_I(TAG, "Loaded identity: %s", identity->nickname);
        } else {
            FURI_LOG_E(TAG, "Invalid identity file");
            free(identity);
            identity = NULL;
        }

        storage_file_close(file);
    } else {
        FURI_LOG_I(TAG, "No identity file found");
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return identity;
}

/**
 * Save identity to storage
 */
void bitchat_identity_save(BitchatIdentity* identity) {
    furi_assert(identity);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, APP_DATA_PATH("bitchat"));

    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, IDENTITY_FILE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint16_t bytes_written = storage_file_write(file, identity, sizeof(BitchatIdentity));
        if(bytes_written == sizeof(BitchatIdentity)) {
            FURI_LOG_I(TAG, "Identity saved");
        } else {
            FURI_LOG_E(TAG, "Failed to save identity");
        }
        storage_file_close(file);
    } else {
        FURI_LOG_E(TAG, "Failed to open identity file for writing");
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

/**
 * Free identity
 */
void bitchat_identity_free(BitchatIdentity* identity) {
    if(identity) {
        // Securely zero out private keys
        memset(identity->noise_private_key, 0, 32);
        memset(identity->signing_private_key, 0, 32);
        free(identity);
    }
}

/**
 * Get nickname
 */
bool bitchat_identity_get_nickname(BitchatIdentity* identity, char* nickname, size_t size) {
    furi_assert(identity);
    furi_assert(nickname);

    if(identity->nickname[0] == '\0') {
        return false;
    }

    strncpy(nickname, identity->nickname, size - 1);
    nickname[size - 1] = '\0';
    return true;
}

/**
 * Set nickname
 */
void bitchat_identity_set_nickname(BitchatIdentity* identity, const char* nickname) {
    furi_assert(identity);
    furi_assert(nickname);

    strncpy(identity->nickname, nickname, sizeof(identity->nickname) - 1);
    identity->nickname[sizeof(identity->nickname) - 1] = '\0';

    FURI_LOG_I(TAG, "Nickname changed to: %s", identity->nickname);
}

/**
 * Get Noise public key
 */
const uint8_t* bitchat_identity_get_public_key(BitchatIdentity* identity) {
    furi_assert(identity);
    return identity->noise_public_key;
}

/**
 * Get peer ID
 */
const uint8_t* bitchat_identity_get_peer_id(BitchatIdentity* identity) {
    furi_assert(identity);
    return identity->peer_id;
}
