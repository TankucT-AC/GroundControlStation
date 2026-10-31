#include "TelemetryPacket.h"
#include "UdpWorker.h"
#include <QSignalSpy>
#include <QUdpSocket>
#include <QtTest>
#include <cstdio>

class TestUdpWorker : public QObject
{
    Q_OBJECT

private slots:
    void testGoodPacketEmitsDataReceived();
    void testBadPacketEmitsErrorCount();
    void testOversizedDatagramRejected();
    void testBindFailureEmitsError();
};

void TestUdpWorker::testGoodPacketEmitsDataReceived()
{
    UdpWorker worker(0);
    QSignalSpy dataSpy(&worker, &UdpWorker::dataReceived);
    worker.start();
    QVERIFY(worker.boundPort() != 0);

    TelemetryPacket pkt;
    pkt.altitude = 150.5f;
    pkt.speed = 22.3f;
    uint8_t buf[TelemetryPacket::PACKET_SIZE];
    TelemetryPacket::pack(buf, pkt);

    QUdpSocket sender;
    qint64 sent = sender.writeDatagram(reinterpret_cast<const char *>(buf), TelemetryPacket::PACKET_SIZE, QHostAddress::LocalHost, worker.boundPort());
    QCOMPARE(sent, qint64(TelemetryPacket::PACKET_SIZE));

    QTRY_COMPARE(dataSpy.size(), 1);
}

void TestUdpWorker::testBadPacketEmitsErrorCount()
{
    UdpWorker worker(0);
    QSignalSpy errSpy(&worker, &UdpWorker::errorCountChanged);
    worker.start();
    QVERIFY(worker.boundPort() != 0);

    QUdpSocket sender;
    char garbage[10] = {};
    sender.writeDatagram(garbage, sizeof(garbage), QHostAddress::LocalHost, worker.boundPort());

    QTRY_COMPARE(errSpy.size(), 1);
    QCOMPARE(errSpy.takeFirst().at(0).toInt(), 1);
}

void TestUdpWorker::testOversizedDatagramRejected()
{
    UdpWorker worker(0);
    QSignalSpy dataSpy(&worker, &UdpWorker::dataReceived);
    QSignalSpy errSpy(&worker, &UdpWorker::errorCountChanged);
    worker.start();
    QVERIFY(worker.boundPort() != 0);

    // Валидный пакет + мусор в хвосте: датаграмм нестандартного размера отбраковывается целиком
    TelemetryPacket pkt;
    uint8_t buf[TelemetryPacket::PACKET_SIZE + 10];
    TelemetryPacket::pack(buf, pkt);
    for (size_t i = TelemetryPacket::PACKET_SIZE; i < sizeof(buf); ++i)
        buf[i] = 0xEE;

    QUdpSocket sender;
    sender.writeDatagram(reinterpret_cast<const char *>(buf), sizeof(buf), QHostAddress::LocalHost, worker.boundPort());

    QTRY_COMPARE(errSpy.size(), 1);
    QCOMPARE(dataSpy.size(), 0);
}

void TestUdpWorker::testBindFailureEmitsError()
{
    // Занимаем порт, чтобы bind() воркера гарантированно провалился
    QUdpSocket blocker;
    QVERIFY(blocker.bind(QHostAddress::Any, 0));
    quint16 takenPort = blocker.localPort();

    UdpWorker worker(takenPort);
    QSignalSpy failSpy(&worker, &UdpWorker::errorOccurred);
    worker.start();
    QCOMPARE(failSpy.size(), 1);
}

QTEST_MAIN(TestUdpWorker)
#include "test_udpworker.moc"
