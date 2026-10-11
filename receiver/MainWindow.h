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
    QTimer *m_uiTimer;
};

// ---------- BaseGaudgeWidget ----------

class BaseGaudgeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseGaudgeWidget(QWidget *parent = nullptr);

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

    virtual void drawGaudge(QPainter &p) = 0;
};

// ---------- Altimeter ----------
class AltimeterWidget : public BaseGaudgeWidget
{
    Q_OBJECT
public:
    explicit AltimeterWidget(QWidget *parent = nullptr);
    void setAltitude(float alt);

protected:
    void drawGaudge(QPainter &p) override;

private:
    float m_altitude = 0.0f;
};

// ---------- Speedometer ----------
class SpeedometerWidget : public BaseGaudgeWidget
{
    Q_OBJECT
public:
    explicit SpeedometerWidget(QWidget *parent = nullptr);
    void setSpeed(float speed);

protected:
    void drawGaudge(QPainter &p) override;

private:
    float m_speed = 0.0f;
};

// ---------- Attitude (pitch/roll) ----------
class AttitudeWidget : public BaseGaudgeWidget
{
    Q_OBJECT
public:
    explicit AttitudeWidget(QWidget *parent = nullptr);
    void setPitchRoll(float pitch, float roll);

protected:
    void drawGaudge(QPainter &p) override;

private:
    float m_pitch = 0.0f;
    float m_roll = 0.0f;
};

// ---------- Compass (yaw) ----------
class CompassWidget : public BaseGaudgeWidget
{
    Q_OBJECT
public:
    explicit CompassWidget(QWidget *parent = nullptr);
    void setYaw(float yaw);

protected:
    void drawGaudge(QPainter &p) override;

private:
    float m_yaw = 0.0f;
};

#endif
