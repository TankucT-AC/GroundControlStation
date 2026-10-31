#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TelemetryPacket.h"
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QWidget>
#include <qtmetamacros.h>

class AltimeterWidget;
class SpeedometerWidget;
class AttitudeWidget;
class CompassWidget;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void onTelemetryReceived(const TelemetryPacket &pkt);
    void onErrorCountChanged(int count);
    void onLinkError(const QString &message);

private slots:
    void onWatchdogTick();

private:
    void updateStatusLabel();

private:
    enum class LinkState { NoData, Live, Lost };
    LinkState m_linkState = LinkState::NoData;
    qint64 m_lastPacketMs = 0;

private:
    AltimeterWidget *m_altimeter;
    SpeedometerWidget *m_speedometer;
    AttitudeWidget *m_attitude;
    CompassWidget *m_compass;
    QLabel *m_gpsLabel;
    QLabel *m_batteryLabel;
    QProgressBar *m_batteryBar;
    QLabel *m_satLabel;
    QLabel *m_errorLabel;
    QLabel *m_timestampLabel;
    QLabel *m_statusLabel;
    QTimer *m_uiTimer;
};

// ---------- BaseGaugeWidget ----------

class BaseGaugeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseGaugeWidget(QWidget *parent = nullptr);

    // true — прибор «заморожен»: затемнение и надпись NO DATA (нет данных от воркера)
    void setStale(bool stale);

protected:
    // Эти константы задают виртуальные координаты для работы с виджетами
    // Смещение левого верхнего угла в (-100, -100) позволяет удобно пользоваться
    // полярной системой координат
    static constexpr int widgetLeft = -100;
    static constexpr int widgetTop = -100;
    static constexpr int widgetWidth = 200;
    static constexpr int widgetHeight = 200;

    // Правая и нижняя границы
    static constexpr int widgetRight = widgetLeft + widgetWidth;
    static constexpr int widgetBottom = widgetTop + widgetHeight;

    void paintEvent(QPaintEvent *event) override;

    virtual void drawGauge(QPainter &p) = 0;

private:
    bool m_stale = false;
};

// ---------- Altimeter ----------
class AltimeterWidget : public BaseGaugeWidget
{
    Q_OBJECT
public:
    explicit AltimeterWidget(QWidget *parent = nullptr);
    void setAltitude(float alt);

protected:
    void drawGauge(QPainter &p) override;

private:
    float m_altitude = 0.0f;
};

// ---------- Speedometer ----------
class SpeedometerWidget : public BaseGaugeWidget
{
    Q_OBJECT
public:
    explicit SpeedometerWidget(QWidget *parent = nullptr);
    void setSpeed(float speed);

protected:
    void drawGauge(QPainter &p) override;

private:
    float m_speed = 0.0f;
};

// ---------- Attitude (pitch/roll) ----------
class AttitudeWidget : public BaseGaugeWidget
{
    Q_OBJECT
public:
    explicit AttitudeWidget(QWidget *parent = nullptr);
    void setPitchRoll(float pitch, float roll);

protected:
    void drawGauge(QPainter &p) override;

private:
    float m_pitch = 0.0f;
    float m_roll = 0.0f;
};

// ---------- Compass (yaw) ----------
class CompassWidget : public BaseGaugeWidget
{
    Q_OBJECT
public:
    explicit CompassWidget(QWidget *parent = nullptr);
    void setYaw(float yaw);

protected:
    void drawGauge(QPainter &p) override;

private:
    float m_yaw = 0.0f;
};

#endif
