/**
 * BitChat Protocol Implementation
 */

#include "bitchat_protocol.h"
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>
#include <furi_hal_rtc.h>
#include <string.h>
#include <stdlib.h>

#define TAG "BitchatProtocol"

/**
 * Encode a 16-bit value to big-endian
 */
static void encode_u16_be(uint8_t* buf, uint16_t value) {
    buf[0] = (value >> 8) & 0xFF;
    buf[1] = value & 0xFF;
}

/**
 * Decode a 16-bit value from big-endian
 */
static uint16_t decode_u16_be(const uint8_t* buf) {
    return ((uint16_t)buf[0] << 8) | buf[1];
}

/**
 * Encode a 64-bit value to big-endian
 */
static void encode_u64_be(uint8_t* buf, uint64_t value) {
    for(int i = 7; i >= 0; i--) {
        buf[7 - i] = (value >> (i * 8)) & 0xFF;
    }
}

/**
 * Decode a 64-bit value from big-endian
 */
static uint64_t decode_u64_be(const uint8_t* buf) {
    uint64_t value = 0;
    for(int i = 0; i < 8; i++) {
        value = (value << 8) | buf[i];
    }
    return value;
}

/**
 * Encode a packet to binary format
 */
size_t bitchat_packet_encode(const BitchatPacket* packet, uint8_t* buffer, size_t buffer_size) {
    furi_assert(packet);
    furi_assert(buffer);

    // Calculate required size
    size_t required_size = BITCHAT_HEADER_SIZE + BITCHAT_SENDER_ID_SIZE + packet->payload_length;
    if(packet->has_recipient) required_size += BITCHAT_RECIPIENT_ID_SIZE;
    if(packet->has_signature) required_size += BITCHAT_SIGNATURE_SIZE;

    if(buffer_size < required_size) {
        FURI_LOG_E(TAG, "Buffer too small: need %zu, have %zu", required_size, buffer_size);
        return 0;
    }

    size_t offset = 0;

    // Header (13 bytes)
    buffer[offset++] = packet->version;
    buffer[offset++] = packet->type;
    buffer[offset++] = packet->ttl;

    // Timestamp (8 bytes, big-endian)
    encode_u64_be(&buffer[offset], packet->timestamp);
    offset += 8;

    // Flags
    uint8_t flags = 0;
    if(packet->has_recipient) flags |= BITCHAT_FLAG_HAS_RECIPIENT;
    if(packet->has_signature) flags |= BITCHAT_FLAG_HAS_SIGNATURE;
    if(packet->is_compressed) flags |= BITCHAT_FLAG_IS_COMPRESSED;
    buffer[offset++] = flags;

    // Payload length (2 bytes, big-endian)
    encode_u16_be(&buffer[offset], packet->payload_length);
    offset += 2;

    // Sender ID (8 bytes)
    memcpy(&buffer[offset], packet->sender_id, BITCHAT_SENDER_ID_SIZE);
    offset += BITCHAT_SENDER_ID_SIZE;

    // Recipient ID (8 bytes, optional)
    if(packet->has_recipient) {
        memcpy(&buffer[offset], packet->recipient_id, BITCHAT_RECIPIENT_ID_SIZE);
        offset += BITCHAT_RECIPIENT_ID_SIZE;
    }

    // Payload
    if(packet->payload && packet->payload_length > 0) {
        memcpy(&buffer[offset], packet->payload, packet->payload_length);
        offset += packet->payload_length;
    }

    // Signature (64 bytes, optional)
    if(packet->has_signature) {
        memcpy(&buffer[offset], packet->signature, BITCHAT_SIGNATURE_SIZE);
        offset += BITCHAT_SIGNATURE_SIZE;
    }

    FURI_LOG_D(TAG, "Encoded packet: type=%d, ttl=%d, payload=%d bytes, total=%zu bytes",
        packet->type, packet->ttl, packet->payload_length, offset);

    return offset;
}

/**
 * Decode binary data to a packet
 */
bool bitchat_packet_decode(const uint8_t* data, size_t data_size, BitchatPacket* packet) {
    furi_assert(data);
    furi_assert(packet);

    // Minimum size check (header + sender ID)
    if(data_size < BITCHAT_HEADER_SIZE + BITCHAT_SENDER_ID_SIZE) {
        FURI_LOG_E(TAG, "Packet too small: %zu bytes", data_size);
        return false;
    }

    size_t offset = 0;

    // Parse header
    packet->version = data[offset++];
    if(packet->version != BITCHAT_VERSION) {
        FURI_LOG_E(TAG, "Invalid version: %d", packet->version);
        return false;
    }

    packet->type = data[offset++];
    packet->ttl = data[offset++];

    // Timestamp
    packet->timestamp = decode_u64_be(&data[offset]);
    offset += 8;

    // Flags
    uint8_t flags = data[offset++];
    packet->has_recipient = (flags & BITCHAT_FLAG_HAS_RECIPIENT) != 0;
    packet->has_signature = (flags & BITCHAT_FLAG_HAS_SIGNATURE) != 0;
    packet->is_compressed = (flags & BITCHAT_FLAG_IS_COMPRESSED) != 0;
    packet->flags = flags;

    // Payload length
    packet->payload_length = decode_u16_be(&data[offset]);
    offset += 2;

    // Sender ID
    memcpy(packet->sender_id, &data[offset], BITCHAT_SENDER_ID_SIZE);
    offset += BITCHAT_SENDER_ID_SIZE;

    // Recipient ID (optional)
    if(packet->has_recipient) {
        if(offset + BITCHAT_RECIPIENT_ID_SIZE > data_size) return false;
        memcpy(packet->recipient_id, &data[offset], BITCHAT_RECIPIENT_ID_SIZE);
        offset += BITCHAT_RECIPIENT_ID_SIZE;
    }

    // Payload
    if(offset + packet->payload_length > data_size) {
        FURI_LOG_E(TAG, "Payload overflow: need %d, have %zu", packet->payload_length, data_size - offset);
        return false;
    }

    // Allocate and copy payload
    if(packet->payload_length > 0) {
        packet->payload = malloc(packet->payload_length);
        memcpy(packet->payload, &data[offset], packet->payload_length);
        offset += packet->payload_length;
    } else {
        packet->payload = NULL;
    }

    // Signature (optional)
    if(packet->has_signature) {
        if(offset + BITCHAT_SIGNATURE_SIZE > data_size) return false;
        memcpy(packet->signature, &data[offset], BITCHAT_SIGNATURE_SIZE);
        offset += BITCHAT_SIGNATURE_SIZE;
    }

    FURI_LOG_D(TAG, "Decoded packet: type=%d, ttl=%d, payload=%d bytes",
        packet->type, packet->ttl, packet->payload_length);

    return true;
}

/**
 * Encode a message to binary payload
 */
size_t bitchat_message_encode(const BitchatMessage* message, uint8_t* buffer, size_t buffer_size) {
    furi_assert(message);
    furi_assert(buffer);
    UNUSED(buffer_size);  // TODO: Add bounds checking

    size_t offset = 0;

    // Flags
    uint8_t flags = 0;
    if(message->is_relay) flags |= 0x01;
    if(message->is_private) flags |= 0x02;
    if(message->original_sender[0] != '\0') flags |= 0x04;
    if(message->recipient_nickname[0] != '\0') flags |= 0x08;
    if(message->sender_peer_id[0] != '\0') flags |= 0x10;

    buffer[offset++] = flags;

    // Timestamp (8 bytes, big-endian milliseconds)
    encode_u64_be(&buffer[offset], message->timestamp);
    offset += 8;

    // ID length + ID
    uint8_t id_len = strlen(message->id);
    buffer[offset++] = id_len;
    memcpy(&buffer[offset], message->id, id_len);
    offset += id_len;

    // Sender length + sender
    uint8_t sender_len = strlen(message->sender);
    buffer[offset++] = sender_len;
    memcpy(&buffer[offset], message->sender, sender_len);
    offset += sender_len;

    // Content length + content (2 bytes length)
    uint16_t content_len = strlen(message->content);
    encode_u16_be(&buffer[offset], content_len);
    offset += 2;
    memcpy(&buffer[offset], message->content, content_len);
    offset += content_len;

    // Optional fields based on flags
    if(flags & 0x04) {  // original_sender
        uint8_t len = strlen(message->original_sender);
        buffer[offset++] = len;
        memcpy(&buffer[offset], message->original_sender, len);
        offset += len;
    }

    if(flags & 0x08) {  // recipient_nickname
        uint8_t len = strlen(message->recipient_nickname);
        buffer[offset++] = len;
        memcpy(&buffer[offset], message->recipient_nickname, len);
        offset += len;
    }

    if(flags & 0x10) {  // sender_peer_id
        uint8_t len = strlen(message->sender_peer_id);
        buffer[offset++] = len;
        memcpy(&buffer[offset], message->sender_peer_id, len);
        offset += len;
    }

    return offset;
}

/**
 * Decode binary payload to a message
 */
bool bitchat_message_decode(const uint8_t* data, size_t data_size, BitchatMessage* message) {
    furi_assert(data);
    furi_assert(message);

    memset(message, 0, sizeof(BitchatMessage));

    if(data_size < 13) return false;  // Minimum size

    size_t offset = 0;

    // Flags
    uint8_t flags = data[offset++];
    message->is_relay = (flags & 0x01) != 0;
    message->is_private = (flags & 0x02) != 0;
    bool has_original = (flags & 0x04) != 0;
    bool has_recipient = (flags & 0x08) != 0;
    bool has_peer_id = (flags & 0x10) != 0;

    // Timestamp
    message->timestamp = decode_u64_be(&data[offset]);
    offset += 8;

    // ID
    uint8_t id_len = data[offset++];
    if(offset + id_len > data_size) return false;
    memcpy(message->id, &data[offset], id_len);
    message->id[id_len] = '\0';
    offset += id_len;

    // Sender
    uint8_t sender_len = data[offset++];
    if(offset + sender_len > data_size) return false;
    memcpy(message->sender, &data[offset], sender_len);
    message->sender[sender_len] = '\0';
    offset += sender_len;

    // Content
    uint16_t content_len = decode_u16_be(&data[offset]);
    offset += 2;
    if(offset + content_len > data_size) return false;
    size_t copy_len = content_len < sizeof(message->content) - 1 ?
                      content_len : sizeof(message->content) - 1;
    memcpy(message->content, &data[offset], copy_len);
    message->content[copy_len] = '\0';
    offset += content_len;

    // Optional fields
    if(has_original && offset < data_size) {
        uint8_t len = data[offset++];
        if(offset + len <= data_size) {
            memcpy(message->original_sender, &data[offset], len);
            message->original_sender[len] = '\0';
            offset += len;
        }
    }

    if(has_recipient && offset < data_size) {
        uint8_t len = data[offset++];
        if(offset + len <= data_size) {
            memcpy(message->recipient_nickname, &data[offset], len);
            message->recipient_nickname[len] = '\0';
            offset += len;
        }
    }

    if(has_peer_id && offset < data_size) {
        uint8_t len = data[offset++];
        if(offset + len <= data_size) {
            memcpy(message->sender_peer_id, &data[offset], len);
            message->sender_peer_id[len] = '\0';
            offset += len;
        }
    }

    return true;
}

/**
 * Allocate a new packet
 */
BitchatPacket* bitchat_packet_alloc(void) {
    BitchatPacket* packet = malloc(sizeof(BitchatPacket));
    memset(packet, 0, sizeof(BitchatPacket));
    packet->version = BITCHAT_VERSION;
    return packet;
}

/**
 * Free a packet
 */
void bitchat_packet_free(BitchatPacket* packet) {
    if(packet) {
        if(packet->payload) {
            free(packet->payload);
        }
        free(packet);
    }
}

/**
 * Allocate a new message
 */
BitchatMessage* bitchat_message_alloc(void) {
    BitchatMessage* message = malloc(sizeof(BitchatMessage));
    memset(message, 0, sizeof(BitchatMessage));
    bitchat_generate_message_id(message->id, sizeof(message->id));
    message->timestamp = bitchat_get_timestamp_ms();
    return message;
}

/**
 * Free a message
 */
void bitchat_message_free(BitchatMessage* message) {
    if(message) {
        free(message);
    }
}

/**
 * Generate a unique message ID (simplified UUID v4)
 */
void bitchat_generate_message_id(char* id, size_t size) {
    furi_assert(id);
    furi_assert(size >= 37);

    // Generate random UUID v4
    uint32_t r1 = furi_hal_random_get();
    uint32_t r2 = furi_hal_random_get();
    uint32_t r3 = furi_hal_random_get();
    uint32_t r4 = furi_hal_random_get();

    snprintf(id, size,
        "%08lx-%04lx-4%03lx-%04lx-%012llx",
        r1,
        (r2 >> 16) & 0xFFFF,
        r2 & 0x0FFF,
        ((r3 >> 16) & 0x3FFF) | 0x8000,
        ((uint64_t)(r3 & 0xFFFF) << 32) | r4
    );
}

/**
 * Get current timestamp in milliseconds
 */
uint64_t bitchat_get_timestamp_ms(void) {
    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);

    // Convert to Unix timestamp (approximate)
    // This is simplified - doesn't account for all edge cases
    uint32_t days = datetime.day;
    for(int m = 1; m < datetime.month; m++) {
        if(m == 2) days += 28;
        else if(m == 4 || m == 6 || m == 9 || m == 11) days += 30;
        else days += 31;
    }
    days += (datetime.year - 1970) * 365;

    uint64_t seconds = days * 86400ULL +
                      datetime.hour * 3600ULL +
                      datetime.minute * 60ULL +
                      datetime.second;

    return seconds * 1000ULL;
}
