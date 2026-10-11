#include "TelemetryPacket.h"
#include <QtTest>
#include <cstring>

class TestTelemetry : public QObject
{
    Q_OBJECT

private slots:
    void testPackUnpack();
    void testBadMagic();
    void testBadCrc();
    void testTooShort();
    void testBoundaryValues();
    void testCrcKnownVector();
    void testCrcEmpty();
    void testLittleEndian();
};

void TestTelemetry::testPackUnpack()
{
    TelemetryPacket in;
    in.timestamp = 1234567890;
    in.altitude = 150.5f;
    in.speed = 22.3f;
    in.pitch = -5.0f;
    in.roll = 10.5f;
    in.yaw = 270.0f;
    in.latitude = 55.751244;
    in.longitude = 37.618423;
    in.battery = 85;
    in.satellites = 10;

    uint8_t buffer[TelemetryPacket::PACKET_SIZE];
    TelemetryPacket::pack(buffer, in);

    auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
    QVERIFY(result.has_value());

    const auto &out = result.value();
    QCOMPARE(out.timestamp, in.timestamp);
    QCOMPARE(out.altitude, in.altitude);
    QCOMPARE(out.speed, in.speed);
    QCOMPARE(out.pitch, in.pitch);
    QCOMPARE(out.roll, in.roll);
    QCOMPARE(out.yaw, in.yaw);
    QCOMPARE(out.latitude, in.latitude);
    QCOMPARE(out.longitude, in.longitude);
    QCOMPARE(out.battery, in.battery);
    QCOMPARE(out.satellites, in.satellites);
}

void TestTelemetry::testBadMagic()
{
    uint8_t buffer[TelemetryPacket::PACKET_SIZE] = {};
    TelemetryPacket::pack(buffer, TelemetryPacket{});

    buffer[0] = 0x00;
    auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
    QVERIFY(!result.has_value());
}

void TestTelemetry::testBadCrc()
{
    uint8_t buffer[TelemetryPacket::PACKET_SIZE] = {};
    TelemetryPacket::pack(buffer, TelemetryPacket{});

    buffer[44] ^= 0xFF;
    auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
    QVERIFY(!result.has_value());
}

void TestTelemetry::testTooShort()
{
    uint8_t buf[10] = {};
    auto result = TelemetryPacket::unpack(buf, 10);
    QVERIFY(!result.has_value());
}

void TestTelemetry::testBoundaryValues()
{
    TelemetryPacket in;
    in.altitude = std::numeric_limits<float>::max();
    in.speed = -1.0f;
    in.battery = 0;
    in.satellites = 255;

    uint8_t buffer[TelemetryPacket::PACKET_SIZE];
    TelemetryPacket::pack(buffer, in);

    auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
    QVERIFY(result.has_value());

    const auto &out = result.value();
    QCOMPARE(out.altitude, in.altitude);
    QCOMPARE(out.speed, in.speed);
    QCOMPARE(out.battery, in.battery);
    QCOMPARE(out.satellites, in.satellites);
}

void TestTelemetry::testCrcKnownVector()
{
    // CRC-16/CCITT of "123456789" (0x31..0x39) = 0x29B1
    const uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
    uint16_t crc = TelemetryPacket::crc16(data, sizeof(data));
    QCOMPARE(crc, uint16_t(0x29B1));
}

void TestTelemetry::testCrcEmpty()
{
    // CRC-16/CCITT of empty input with init 0xFFFF = 0xFFFF
    uint16_t crc = TelemetryPacket::crc16(nullptr, 0);
    QCOMPARE(crc, uint16_t(0xFFFF));
}

void TestTelemetry::testLittleEndian()
{
    uint16_t value = 0x0102;
    uint8_t buf[2];
    std::memcpy(buf, &value, sizeof(buf));
    // On little-endian: buf[0]=0x02, buf[1]=0x01
    QCOMPARE(buf[0], uint8_t(0x02));
    QCOMPARE(buf[1], uint8_t(0x01));
}

QTEST_MAIN(TestTelemetry)
#include "test_telemetry.moc"
