#ifndef UDPWORKER_H
#define UDPWORKER_H

#include "TelemetryPacket.h"
#include <QObject>
#include <QUdpSocket>

class UdpWorker : public QObject
{
    Q_OBJECT
public:
    explicit UdpWorker(quint16 port, QObject *parent = nullptr);
    ~UdpWorker() override;

public slots:
    void start();
    void stop();

signals:
    void dataReceived(TelemetryPacket pkt);
    void errorOccurred(const QString &message);

private slots:
    void onReadyRead();

private:
    QUdpSocket *m_socket;
    quint16 m_port;
    int m_errorCount = 0;
};

#endif
