#ifndef TELEMETRYPACKET_H
#define TELEMETRYPACKET_H

#include <cstdint>
#include <vector>
#include <optional>

struct TelemetryPacket {
    static constexpr uint8_t MAGIC_0 = 0x55;
    static constexpr uint8_t MAGIC_1 = 0xAA;
    static constexpr size_t PACKET_SIZE = 46;
    static constexpr size_t OFFSET_ALTITUDE = 6;
    static constexpr size_t OFFSET_SPEED = 10;
    static constexpr size_t OFFSET_PITCH = 14;
    static constexpr size_t OFFSET_ROLL = 18;
    static constexpr size_t OFFSET_YAW = 22;
    static constexpr size_t OFFSET_LATITUDE = 26;
    static constexpr size_t OFFSET_LONGITUDE = 34;
    static constexpr size_t OFFSET_BATTERY = 42;
    static constexpr size_t OFFSET_SATELLITES = 43;
    static constexpr size_t OFFSET_CRC = 44;

    uint32_t timestamp = 0;
    float altitude = 0.0f;
    float speed = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;
    double latitude = 0.0;
    double longitude = 0.0;
    uint8_t battery = 0;
    uint8_t satellites = 0;

    static uint16_t crc16(const uint8_t *data, size_t len);
    static void pack(uint8_t *buffer, const TelemetryPacket &pkt);
    static std::optional<TelemetryPacket> unpack(const uint8_t *buffer, size_t len);
};

#endif