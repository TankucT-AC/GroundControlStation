#include "MainWindow.h"
#include "Theme.h"
#include <QApplication>
#include <QDateTime>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <cmath>
#include <qwidget.h>

static constexpr float PI = 3.14159265358979323846f;
static constexpr qint64 kLinkTimeoutMs = 2000;

// ============================== MainWindow ==============================

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("UAV Telemetry Receiver");
    resize(1020, 720);

    // Приборы: равные доли строки
    m_altimeter = new AltimeterWidget(this);
    m_speedometer = new SpeedometerWidget(this);
    m_attitude = new AttitudeWidget(this);
    m_compass = new CompassWidget(this);
    QWidget *gauges[] = {m_altimeter, m_speedometer, m_attitude, m_compass};
    for (QWidget *gauge : gauges) {
        gauge->setMinimumSize(200, 200);
        gauge->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    // Типографика: значения — 11 pt моно, подписи — 9 pt
    QFont valueFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    valueFont.setPointSizeF(11);
    QFont captionFont;
    captionFont.setPointSizeF(9);

    m_gpsLabel = new QLabel("Lat: --\nLon: --", this);
    m_gpsLabel->setFont(valueFont);

    m_satLabel = new QLabel("Satellites: --", this);
    m_satLabel->setFont(valueFont);

    m_batteryLabel = new QLabel("Battery: --", this);
    m_batteryLabel->setFont(valueFont);

    m_batteryBar = new QProgressBar(this);
    m_batteryBar->setRange(0, 100);
    m_batteryBar->setTextVisible(true);

    m_errorLabel = new QLabel("Errors: 0", this);
    m_errorLabel->setFont(captionFont);

    m_timestampLabel = new QLabel("Waiting for data...", this);
    m_timestampLabel->setFont(captionFont);

    m_statusLabel = new QLabel("NO DATA", this);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    m_statusLabel->setFont(statusFont);
    updateStatusLabel();

    // Группировка информации: GPS / Power / Link
    auto *gpsGroup = new QGroupBox("GPS", this);
    auto *gpsLayout = new QVBoxLayout(gpsGroup);
    gpsLayout->addWidget(m_gpsLabel);
    gpsLayout->addWidget(m_satLabel);

    auto *powerGroup = new QGroupBox("Power", this);
    auto *powerLayout = new QVBoxLayout(powerGroup);
    powerLayout->addWidget(m_batteryLabel);
    powerLayout->addWidget(m_batteryBar);

    auto *linkGroup = new QGroupBox("Link", this);
    auto *linkLayout = new QVBoxLayout(linkGroup);
    linkLayout->addWidget(m_statusLabel);
    linkLayout->addWidget(m_errorLabel);
    linkLayout->addWidget(m_timestampLabel);

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_altimeter, 1);
    topLayout->addWidget(m_speedometer, 1);
    topLayout->addWidget(m_attitude, 1);
    topLayout->addWidget(m_compass, 1);

    auto *infoRow = new QHBoxLayout;
    infoRow->addWidget(gpsGroup, 1);
    infoRow->addWidget(powerGroup, 1);
    infoRow->addWidget(linkGroup, 2);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout, 3);
    mainLayout->addLayout(infoRow, 1);

    m_uiTimer = new QTimer(this);
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::onWatchdogTick);
    m_uiTimer->start(1000);
}

void MainWindow::onTelemetryReceived(const TelemetryPacket &pkt)
{
    m_linkState = LinkState::Live;
    m_lastPacketMs = QDateTime::currentMSecsSinceEpoch();
    m_altimeter->setStale(false);
    m_speedometer->setStale(false);
    m_attitude->setStale(false);
    m_compass->setStale(false);
    updateStatusLabel();

    m_altimeter->setAltitude(pkt.altitude);
    m_speedometer->setSpeed(pkt.speed);
    m_attitude->setPitchRoll(pkt.pitch, pkt.roll);
    m_compass->setYaw(pkt.yaw);

    m_gpsLabel->setText(QString("Lat: %1\nLon: %2").arg(pkt.latitude, 0, 'f', 6).arg(pkt.longitude, 0, 'f', 6));

    m_batteryBar->setValue(pkt.battery);
    m_batteryLabel->setText(QString("Battery: %1%").arg(pkt.battery));

    m_satLabel->setText(QString("Satellites: %1").arg(pkt.satellites));

    QDateTime dt;
    dt.setMSecsSinceEpoch(static_cast<qint64>(pkt.timestamp));
    m_timestampLabel->setText("Last: " + dt.toString("hh:mm:ss.zzz"));
}

void MainWindow::onErrorCountChanged(int count)
{
    m_errorLabel->setText(QString("Errors: %1").arg(count));
}

void MainWindow::onLinkError(const QString &message)
{
    QMessageBox::critical(this, "UAV Telemetry Receiver", message);
    qApp->exit(1);
}

void MainWindow::onWatchdogTick()
{
    if (m_lastPacketMs <= 0)
        return;

    const qint64 ageMs = QDateTime::currentMSecsSinceEpoch() - m_lastPacketMs;
    if (ageMs > kLinkTimeoutMs && m_linkState == LinkState::Live) {
        m_linkState = LinkState::Lost;
        m_altimeter->setStale(true);
        m_speedometer->setStale(true);
        m_attitude->setStale(true);
        m_compass->setStale(true);
        updateStatusLabel();
    }
    if (m_linkState == LinkState::Lost)
        m_timestampLabel->setText(QString("Last packet %1 s ago").arg(ageMs / 1000));
}

void MainWindow::updateStatusLabel()
{
    QString text;
    QColor color;
    switch (m_linkState) {
    case LinkState::NoData:
        text = "NO DATA";
        color = Qt::gray;
        break;
    case LinkState::Live:
        text = "LIVE";
        color = Qt::green;
        break;
    case LinkState::Lost:
        text = "LINK LOST";
        color = Qt::red;
        break;
    }
    m_statusLabel->setText(text);
    QPalette pal = m_statusLabel->palette();
    pal.setColor(QPalette::WindowText, color);
    m_statusLabel->setPalette(pal);
}

// ========================== BaseGaugeWidget ============================

BaseGaugeWidget::BaseGaugeWidget(QWidget *parent)
    : QWidget(parent)
{
}

void BaseGaugeWidget::setStale(bool stale)
{
    if (m_stale == stale)
        return;
    m_stale = stale;
    update();
}

void BaseGaugeWidget::paintEvent(QPaintEvent *event)
{
    // Инициализация инструмента рисования в виджете
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Выделение на физическом экране квадрата
    int side = qMin(width(), height());
    p.setViewport((width() - side) / 2, (height() - side) / 2, side, side);

    // Связывание виртуальных координат с рисовалкой
    p.setWindow(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    drawGauge(p);

    // Оверлей потери данных поверх прибора
    if (m_stale) {
        p.setClipping(false);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 170));
        p.drawRect(widgetLeft, widgetTop, widgetWidth, widgetHeight);
        QFont staleFont = p.font();
        staleFont.setPointSize(12);
        staleFont.setBold(true);
        p.setFont(staleFont);
        p.setPen(Theme::textDim);
        p.drawText(QRect(widgetLeft, widgetTop, widgetWidth, widgetHeight), Qt::AlignCenter, "NO DATA");
    }
}

// ============================== AltimeterWidget ==============================

AltimeterWidget::AltimeterWidget(QWidget *parent)
    : BaseGaugeWidget(parent)
{
}

void AltimeterWidget::setAltitude(float alt)
{
    m_altitude = alt;
    update();
}

void AltimeterWidget::drawGauge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Theme::outline, 2));
    p.setBrush(Theme::gaugeFace);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    // Изображение текста (ALT)
    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(9));
    p.drawText(-30, -70, "ALT");

    // Изображение шкалы циферблата
    float angle = 0.0f;
    for (int i = 0; i <= 20; ++i) {
        float altVal = i * 10.0f;
        // Трансляция высоты в градусы со смещением на 135 градусов
        float a = -135.0f + (altVal / 200.0f) * 270.0f;
        float rad = a * PI / 180.0f;
        int rOuter = 85;
        int rInner = (i % 5 == 0) ? 75 : 80;
        int x1 = static_cast<int>(rOuter * std::cos(rad));
        int y1 = static_cast<int>(rOuter * std::sin(rad));
        int x2 = static_cast<int>(rInner * std::cos(rad));
        int y2 = static_cast<int>(rInner * std::sin(rad));
        p.drawLine(x1, y1, x2, y2);
    }

    // Расчет положения и изображение стрелки циферблата
    float clamped = std::clamp(m_altitude, 0.0f, 200.0f);
    float needleAngle = -135.0f + (clamped / 200.0f) * 270.0f;
    float needleRad = needleAngle * PI / 180.0f;
    p.setPen(QPen(Theme::needle, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    // Изображение единицы измерения шкалы высоты
    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(11, true));
    p.drawText(-25, 10, QString::number(static_cast<int>(m_altitude)) + "m");
}

// =============== SpeedometerWidget =================

SpeedometerWidget::SpeedometerWidget(QWidget *parent)
    : BaseGaugeWidget(parent)
{
}

void SpeedometerWidget::setSpeed(float speed)
{
    m_speed = speed;
    update();
}

void SpeedometerWidget::drawGauge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Theme::outline, 2));
    p.setBrush(Theme::gaugeFace);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    // Изображение текста (SPD)
    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(9));
    p.drawText(-30, -70, "SPD");

    // Отрисовка шкалы циферблата
    for (int i = 0; i <= 12; ++i) {
        float spdVal = i * 5.0f;
        // Перевод скорости в градусы со смещением на -120 градусов
        float a = -120.0f + (spdVal / 60.0f) * 240.0f;
        float rad = a * PI / 180.0f;
        int rOuter = 85;
        int rInner = (i % 3 == 0) ? 75 : 80;
        int x1 = static_cast<int>(rOuter * std::cos(rad));
        int y1 = static_cast<int>(rOuter * std::sin(rad));
        int x2 = static_cast<int>(rInner * std::cos(rad));
        int y2 = static_cast<int>(rInner * std::sin(rad));
        p.drawLine(x1, y1, x2, y2);
    }

    // Расчет угла и изображение стрелки спидометра
    float clamped = std::clamp(m_speed, 0.0f, 60.0f);
    float needleAngle = -120.0f + (clamped / 60.0f) * 240.0f;
    float needleRad = needleAngle * PI / 180.0f;
    p.setPen(QPen(Theme::needle, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    // Изображение текста единицы измерения скорости
    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(11, true));
    p.drawText(-25, 10, QString::number(static_cast<int>(m_speed)) + "m/s");
}

// ============================== AttitudeWidget ==============================

AttitudeWidget::AttitudeWidget(QWidget *parent)
    : BaseGaugeWidget(parent)
{
}

void AttitudeWidget::setPitchRoll(float pitch, float roll)
{
    m_pitch = pitch;
    m_roll = roll;
    update();
}

void AttitudeWidget::drawGauge(QPainter &p)
{
    // Включаем обрезку по рамке
    p.setClipRect(widgetLeft + 2, widgetTop + 2, widgetWidth - 4, widgetHeight - 4);

    // Рисование динамичной части
    p.save();
    p.translate(0, 0);
    p.rotate(-m_roll);

    int pitchOffset = static_cast<int>(m_pitch * 2.0f);

    // Небо
    p.setBrush(Theme::sky);
    p.setPen(Qt::NoPen);
    QPolygon sky;
    // Домножение на 2 нужно чтобы не образовывались пустоты при повороте
    sky << QPoint(widgetLeft * 2, widgetTop * 2) << QPoint(widgetRight * 2, widgetTop * 2) << QPoint(widgetRight * 2, pitchOffset)
        << QPoint(widgetLeft * 2, pitchOffset);
    p.drawPolygon(sky);

    // Земля
    p.setBrush(Theme::ground);
    QPolygon ground;
    // Домножение на 2 нужно чтобы не образовывались пустоты при повороте
    ground << QPoint(widgetLeft * 2, pitchOffset) << QPoint(widgetRight * 2, pitchOffset) << QPoint(widgetRight * 2, widgetBottom * 2)
           << QPoint(widgetLeft * 2, widgetBottom * 2);
    p.drawPolygon(ground);

    // Линия горизонта
    p.setPen(QPen(Theme::textPrimary, 2));
    // Домножение на 2 нужно чтобы не образовывались обрывы горизонта при повороте
    p.drawLine(widgetLeft * 2, pitchOffset, widgetRight * 2, pitchOffset);

    p.restore();

    // Отрисовка статической части
    p.setClipping(false);
    p.setPen(QPen(Theme::textPrimary, 2));
    p.setBrush(Qt::NoBrush);

    // Отрисовка внешней рамки
    p.drawRect(widgetLeft + 2, widgetTop + 2, widgetWidth - 4, widgetHeight - 4);

    static constexpr int crossSizeX = widgetWidth / 10;
    static constexpr int crossSizeY = widgetHeight / 20;
    p.drawLine(-crossSizeX, 0, crossSizeX, 0);
    p.drawLine(0, -crossSizeY, 0, crossSizeY);
}

// ============================== CompassWidget ==============================

CompassWidget::CompassWidget(QWidget *parent)
    : BaseGaugeWidget(parent)
{
}

void CompassWidget::setYaw(float yaw)
{
    m_yaw = yaw;
    update();
}

void CompassWidget::drawGauge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Theme::outline, 2));
    p.setBrush(Theme::gaugeFace);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    p.setFont(Theme::fixedFont(8));
    for (int i = 0; i < 360; i += 30) {
        float rad = (i - 90.0f) * PI / 180.0f;
        int rOuter = 85;
        int rInner = (i % 90 == 0) ? 72 : 80;
        int x1 = static_cast<int>(rOuter * std::cos(rad));
        int y1 = static_cast<int>(rOuter * std::sin(rad));
        int x2 = static_cast<int>(rInner * std::cos(rad));
        int y2 = static_cast<int>(rInner * std::sin(rad));
        p.drawLine(x1, y1, x2, y2);

        if (i % 90 == 0) {
            QString label;
            switch (i) {
            case 0:
                label = "N";
                break;
            case 90:
                label = "E";
                break;
            case 180:
                label = "S";
                break;
            case 270:
                label = "W";
                break;
            }
            int lx = static_cast<int>(60 * std::cos(rad));
            int ly = static_cast<int>(60 * std::sin(rad));
            p.drawText(lx - 5, ly + 4, label);
        }
    }

    float needleRad = (m_yaw - 90.0f) * PI / 180.0f;
    p.setPen(QPen(Theme::needle, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(9, true));
    p.drawText(-15, 10, QString::number(static_cast<int>(m_yaw)) + QChar(0xB0));
}
