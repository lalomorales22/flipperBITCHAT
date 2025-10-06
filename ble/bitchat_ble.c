/**
 * BitChat BLE Transport Layer Implementation
 */

#include "bitchat_ble.h"
#include "../protocol/bitchat_protocol.h"
#include <furi.h>
#include <furi_hal.h>
#include <string.h>

#define TAG "BitchatBLE"

struct BitchatBle {
    FuriMessageQueue* event_queue;
    BitchatBlePeer peers[BITCHAT_BLE_MAX_PEERS];
    size_t peer_count;
    bool is_active;
    FuriMutex* mutex;

    // BLE state
    uint8_t local_peer_id[8];

    // Stream assembler for fragmented packets
    uint8_t rx_buffer[BITCHAT_BLE_MTU * 2];
    size_t rx_buffer_size;
};

// BLE event handler removed - will be implemented when BLE API is used

/**
 * Initialize BLE service
 */
BitchatBle* bitchat_ble_alloc(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    BitchatBle* ble = malloc(sizeof(BitchatBle));
    memset(ble, 0, sizeof(BitchatBle));

    ble->event_queue = event_queue;
    ble->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    ble->is_active = false;
    ble->peer_count = 0;

    // Generate random local peer ID
    for(int i = 0; i < 8; i++) {
        ble->local_peer_id[i] = furi_hal_random_get() & 0xFF;
    }

    FURI_LOG_I(TAG, "BLE service initialized");

    return ble;
}

/**
 * Free BLE service
 */
void bitchat_ble_free(BitchatBle* ble) {
    furi_assert(ble);

    if(ble->is_active) {
        bitchat_ble_stop(ble);
    }

    furi_mutex_free(ble->mutex);
    free(ble);

    FURI_LOG_I(TAG, "BLE service freed");
}

/**
 * Start BLE advertising and scanning
 */
void bitchat_ble_start(BitchatBle* ble, BitchatIdentity* identity) {
    furi_assert(ble);
    furi_assert(identity);

    furi_mutex_acquire(ble->mutex, FuriWaitForever);

    if(ble->is_active) {
        furi_mutex_release(ble->mutex);
        return;
    }

    // Copy peer ID from identity
    memcpy(ble->local_peer_id, bitchat_identity_get_peer_id(identity), 8);

    // TODO: Start BLE advertising with BitChat service UUID
    // TODO: Start BLE scanning for other BitChat devices
    // For now, just mark as active

    ble->is_active = true;

    furi_mutex_release(ble->mutex);

    FURI_LOG_I(TAG, "BLE started, peer_id=%02X%02X%02X%02X%02X%02X%02X%02X",
        ble->local_peer_id[0], ble->local_peer_id[1],
        ble->local_peer_id[2], ble->local_peer_id[3],
        ble->local_peer_id[4], ble->local_peer_id[5],
        ble->local_peer_id[6], ble->local_peer_id[7]);
}

/**
 * Stop BLE advertising and scanning
 */
void bitchat_ble_stop(BitchatBle* ble) {
    furi_assert(ble);

    furi_mutex_acquire(ble->mutex, FuriWaitForever);

    if(!ble->is_active) {
        furi_mutex_release(ble->mutex);
        return;
    }

    // TODO: Stop BLE advertising and scanning
    // TODO: Disconnect all peers

    ble->is_active = false;
    ble->peer_count = 0;

    furi_mutex_release(ble->mutex);

    FURI_LOG_I(TAG, "BLE stopped");
}

/**
 * Send a packet to all connected peers (broadcast)
 */
bool bitchat_ble_broadcast(BitchatBle* ble, const uint8_t* data, size_t size) {
    furi_assert(ble);
    furi_assert(data);

    if(!ble->is_active) {
        FURI_LOG_W(TAG, "Cannot broadcast: BLE not active");
        return false;
    }

    if(size > BITCHAT_BLE_MTU) {
        FURI_LOG_W(TAG, "Packet too large: %zu bytes", size);
        return false;
    }

    furi_mutex_acquire(ble->mutex, FuriWaitForever);

    // TODO: Send to all connected peers via BLE characteristic write
    FURI_LOG_D(TAG, "Broadcasting %zu bytes to %zu peers", size, ble->peer_count);

    furi_mutex_release(ble->mutex);

    return true;
}

/**
 * Send a packet to a specific peer
 */
bool bitchat_ble_send_to_peer(
    BitchatBle* ble,
    const uint8_t* peer_id,
    const uint8_t* data,
    size_t size) {
    furi_assert(ble);
    furi_assert(peer_id);
    furi_assert(data);

    if(!ble->is_active) {
        return false;
    }

    furi_mutex_acquire(ble->mutex, FuriWaitForever);

    // Find peer
    bool found = false;
    for(size_t i = 0; i < ble->peer_count; i++) {
        if(memcmp(ble->peers[i].peer_id, peer_id, 8) == 0) {
            found = true;
            break;
        }
    }

    if(!found) {
        FURI_LOG_W(TAG, "Peer not found");
        furi_mutex_release(ble->mutex);
        return false;
    }

    // TODO: Send to specific peer via BLE
    FURI_LOG_D(TAG, "Sending %zu bytes to peer", size);

    furi_mutex_release(ble->mutex);

    return true;
}

/**
 * Get list of connected peers
 */
size_t bitchat_ble_get_peers(BitchatBle* ble, BitchatBlePeer* peers, size_t max_peers) {
    furi_assert(ble);
    furi_assert(peers);

    furi_mutex_acquire(ble->mutex, FuriWaitForever);

    size_t count = ble->peer_count < max_peers ? ble->peer_count : max_peers;
    memcpy(peers, ble->peers, count * sizeof(BitchatBlePeer));

    furi_mutex_release(ble->mutex);

    return count;
}

/**
 * Check if BLE is active
 */
bool bitchat_ble_is_active(BitchatBle* ble) {
    furi_assert(ble);
    return ble->is_active;
}
