#include "TelemetryPacket.h"
#include <QtTest>

class TestTelemetry : public QObject {
  Q_OBJECT

private slots:
  void testPackUnpack();
  void testBadMagic();
  void testBadCrc();
  void testTooShort();
  void testBoundaryValues();
};

void TestTelemetry::testPackUnpack() {
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

void TestTelemetry::testBadMagic() {
  uint8_t buffer[TelemetryPacket::PACKET_SIZE] = {};
  TelemetryPacket::pack(buffer, TelemetryPacket{});

  buffer[0] = 0x00;
  auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
  QVERIFY(!result.has_value());
}

void TestTelemetry::testBadCrc() {
  uint8_t buffer[TelemetryPacket::PACKET_SIZE] = {};
  TelemetryPacket::pack(buffer, TelemetryPacket{});

  buffer[44] ^= 0xFF;
  auto result = TelemetryPacket::unpack(buffer, TelemetryPacket::PACKET_SIZE);
  QVERIFY(!result.has_value());
}

void TestTelemetry::testTooShort() {
  uint8_t buf[10] = {};
  auto result = TelemetryPacket::unpack(buf, 10);
  QVERIFY(!result.has_value());
}

void TestTelemetry::testBoundaryValues() {
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

QTEST_MAIN(TestTelemetry)
#include "test_telemetry.moc"
