#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TelemetryPacket.h"
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QProgressBar>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <qtmetamacros.h>

class AltimeterWidget;
class SpeedometerWidget;
class AttitudeWidget;
class CompassWidget;
class HistoryChart;
class QCheckBox;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void onTelemetryReceived(const TelemetryPacket &pkt);
    void onErrorCountChanged(int count);
    void onLinkError(const QString &message);

private slots:
    void onWatchdogTick();
    void onCsvToggled(bool on);

private:
    void updateStatusLabel();
    void updateLinkState();
    void updateStats();
    void togglePaused();

private:
    enum class LinkState {
        NoData,
        Live,
        Lost
    };
    LinkState m_linkState = LinkState::NoData;
    qint64 m_lastPacketMs = 0;
    bool m_paused = false;
    qint64 m_totalPackets = 0;
    qint64 m_totalErrors = 0;
    QVector<qint64> m_rxTimestamps;
    QElapsedTimer m_sessionTimer;

private:
    AltimeterWidget *m_altimeter;
    SpeedometerWidget *m_speedometer;
    AttitudeWidget *m_attitude;
    CompassWidget *m_compass;
    HistoryChart *m_altChart;
    HistoryChart *m_speedChart;
    QLabel *m_gpsLabel;
    QLabel *m_batteryLabel;
    QProgressBar *m_batteryBar;
    QLabel *m_satLabel;
    QLabel *m_errorLabel;
    QLabel *m_timestampLabel;
    QLabel *m_statusLabel;
    QLabel *m_statsLabel;
    QCheckBox *m_csvCheck;
    QLabel *m_csvPathLabel;
    QTimer *m_uiTimer;

    // CSV-журнал: пишется только вне паузы, буфер сбрасывается раз в секунду
    QFile m_csvFile;
    QTextStream m_csvStream{&m_csvFile};
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

    // Демпфирование стрелки: экспоненциальное приближение current к target, tau в секундах.
    // Вызывать внутри drawGauge(); планирует следующий кадр, пока значение не сошлось.
    float stepTowards(float current, float target, float tau);
    // Вариант для углов с переходом через 360 (курс)
    float stepTowardsAngle(float current, float target, float tau);

    virtual void drawGauge(QPainter &p) = 0;

private:
    bool m_stale = false;
    float m_dt = 0.0f;
    bool m_animating = false;
    QElapsedTimer m_frameTimer;
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
    float m_altitudeShown = 0.0f;
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
    float m_speedShown = 0.0f;
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
    float m_pitchShown = 0.0f;
    float m_rollShown = 0.0f;
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
    float m_yawShown = 0.0f;
};

#endif
