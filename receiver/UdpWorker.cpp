#include "UdpWorker.h"
#include <QDebug>

UdpWorker::UdpWorker(quint16 port, QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_port(port)
{
}

UdpWorker::~UdpWorker()
{
    stop();
}

quint16 UdpWorker::boundPort() const
{
    return m_socket ? m_socket->localPort() : 0;
}

void UdpWorker::start()
{
    // Идемпотентность: повторный start не должен терять первый сокет
    if (m_socket)
        return;

    m_socket = new QUdpSocket(this);
    if (!m_socket->bind(m_port)) {
        emit errorOccurred("Failed to bind port " + QString::number(m_port) + ": " + m_socket->errorString());
        m_socket->deleteLater();
        m_socket = nullptr;
        return;
    }
    connect(m_socket, &QUdpSocket::readyRead, this, &UdpWorker::onReadyRead);
}

void UdpWorker::stop()
{
    if (m_socket) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void UdpWorker::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        // Датаграмм нестандартного размера отбраковываем целиком: readDatagram
        // молча обрезает всё сверх maxSize, и «хвост» прошёл бы проверку CRC
        if (m_socket->pendingDatagramSize() != TelemetryPacket::PACKET_SIZE) {
            m_socket->readDatagram(nullptr, 0);
            ++m_errorCount;
            emit errorCountChanged(m_errorCount);
            continue;
        }

        QByteArray datagram(TelemetryPacket::PACKET_SIZE, Qt::Uninitialized);
        QHostAddress sender;
        quint16 senderPort;
        qint64 bytesRead = m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (bytesRead < 0) {
            // Ошибка чтения: выходим из цикла, чтобы не крутиться по одному датаграмму
            ++m_errorCount;
            emit errorCountChanged(m_errorCount);
            qWarning() << "readDatagram failed:" << m_socket->errorString();
            break;
        }
        datagram.resize(bytesRead);

        auto pkt = TelemetryPacket::unpack(reinterpret_cast<const uint8_t *>(datagram.constData()), static_cast<size_t>(datagram.size()));
        if (pkt.has_value()) {
            emit dataReceived(pkt.value());
        } else {
            ++m_errorCount;
            emit errorCountChanged(m_errorCount);
            qWarning() << "Bad packet from" << sender.toString() << "errors:" << m_errorCount;
        }
    }
}
