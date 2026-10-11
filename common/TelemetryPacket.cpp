#include "TelemetryPacket.h"
#include <cstring>

static constexpr uint16_t CRC16_CCITT_POLY = 0x1021;

uint16_t TelemetryPacket::crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ CRC16_CCITT_POLY;
            else
                crc <<= 1;
        }
    }
    return crc;
}

void TelemetryPacket::pack(uint8_t *buffer, const TelemetryPacket &pkt)
{
    buffer[0] = MAGIC_0;
    buffer[1] = MAGIC_1;

    std::memcpy(buffer + 2, &pkt.timestamp, sizeof(pkt.timestamp));
    std::memcpy(buffer + OFFSET_ALTITUDE, &pkt.altitude, sizeof(pkt.altitude));
    std::memcpy(buffer + OFFSET_SPEED, &pkt.speed, sizeof(pkt.speed));
    std::memcpy(buffer + OFFSET_PITCH, &pkt.pitch, sizeof(pkt.pitch));
    std::memcpy(buffer + OFFSET_ROLL, &pkt.roll, sizeof(pkt.roll));
    std::memcpy(buffer + OFFSET_YAW, &pkt.yaw, sizeof(pkt.yaw));
    std::memcpy(buffer + OFFSET_LATITUDE, &pkt.latitude, sizeof(pkt.latitude));
    std::memcpy(buffer + OFFSET_LONGITUDE, &pkt.longitude, sizeof(pkt.longitude));
    buffer[OFFSET_BATTERY] = pkt.battery;
    buffer[OFFSET_SATELLITES] = pkt.satellites;

    uint16_t crc = crc16(buffer, OFFSET_CRC);
    std::memcpy(buffer + OFFSET_CRC, &crc, sizeof(crc));
}

std::optional<TelemetryPacket> TelemetryPacket::unpack(const uint8_t *buffer, size_t len)
{
    if (len < PACKET_SIZE)
        return std::nullopt;

    if (buffer[0] != MAGIC_0 || buffer[1] != MAGIC_1)
        return std::nullopt;

    uint16_t expected_crc;
    std::memcpy(&expected_crc, buffer + OFFSET_CRC, sizeof(expected_crc));
    uint16_t actual_crc = crc16(buffer, OFFSET_CRC);
    if (actual_crc != expected_crc)
        return std::nullopt;

    TelemetryPacket pkt;
    std::memcpy(&pkt.timestamp, buffer + 2, sizeof(pkt.timestamp));
    std::memcpy(&pkt.altitude, buffer + OFFSET_ALTITUDE, sizeof(pkt.altitude));
    std::memcpy(&pkt.speed, buffer + OFFSET_SPEED, sizeof(pkt.speed));
    std::memcpy(&pkt.pitch, buffer + OFFSET_PITCH, sizeof(pkt.pitch));
    std::memcpy(&pkt.roll, buffer + OFFSET_ROLL, sizeof(pkt.roll));
    std::memcpy(&pkt.yaw, buffer + OFFSET_YAW, sizeof(pkt.yaw));
    std::memcpy(&pkt.latitude, buffer + OFFSET_LATITUDE, sizeof(pkt.latitude));
    std::memcpy(&pkt.longitude, buffer + OFFSET_LONGITUDE, sizeof(pkt.longitude));
    pkt.battery = buffer[OFFSET_BATTERY];
    pkt.satellites = buffer[OFFSET_SATELLITES];

    return pkt;
}
