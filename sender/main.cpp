#include "TelemetryPacket.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QRandomGenerator>
#include <QTimer>
#include <QUdpSocket>
#include <cmath>

static constexpr float PI = 3.14159265358979323846f;
static constexpr double BASE_LAT = 55.751244;
static constexpr double BASE_LON = 37.618423;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("UAV Telemetry Sender");
    QCoreApplication::setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Sends binary UAV telemetry over UDP");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOpt({"p", "port"}, "Destination port", "port", "12345");
    QCommandLineOption addrOpt({"a", "addr"}, "Destination address", "addr", "127.0.0.1");
    QCommandLineOption rateOpt({"r", "rate"}, "Send interval in ms", "rate", "50");

    parser.addOption(portOpt);
    parser.addOption(addrOpt);
    parser.addOption(rateOpt);
    parser.process(app);

    auto port = static_cast<quint16>(parser.value(portOpt).toUInt());
    QHostAddress addr(parser.value(addrOpt));
    int rateMs = parser.value(rateOpt).toInt();

    QUdpSocket socket;
    QTimer timer;

    float t = 0.0f;
    float batteryLevel = 100.0f;

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        TelemetryPacket pkt;
        pkt.timestamp = QDateTime::currentMSecsSinceEpoch();

        pkt.altitude = 100.0f + 50.0f * std::sin(0.5f * t);
        pkt.speed = 15.0f + 10.0f * std::sin(0.3f * t + 1.0f);
        pkt.pitch = 15.0f * std::sin(0.4f * t);
        pkt.roll = 20.0f * std::sin(0.35f * t + 0.5f);
        pkt.yaw = 180.0f + 180.0f * std::sin(0.1f * t);

        pkt.latitude = BASE_LAT + 0.01 * std::sin(0.05 * t);
        pkt.longitude = BASE_LON + 0.01 * std::cos(0.05 * t);

        batteryLevel -= 0.1f;
        if (batteryLevel < 20.0f)
            batteryLevel = 100.0f;
        pkt.battery = static_cast<uint8_t>(std::round(batteryLevel));
        pkt.satellites = static_cast<uint8_t>(8 + QRandomGenerator::global()->bounded(5));

        uint8_t buffer[TelemetryPacket::PACKET_SIZE];
        TelemetryPacket::pack(buffer, pkt);

        qint64 sent = socket.writeDatagram(QByteArray(reinterpret_cast<const char *>(buffer), TelemetryPacket::PACKET_SIZE), addr, port);

        if (sent == -1) {
            qWarning() << "Send failed:" << socket.errorString();
        }

        t += 0.05f;
    });

    timer.start(rateMs);

    return app.exec();
}
