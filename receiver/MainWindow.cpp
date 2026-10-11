#include "MainWindow.h"
#include <QDateTime>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <cmath>
#include <qwidget.h>

static constexpr float PI = 3.14159265358979323846f;

// ============================== MainWindow ==============================

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("UAV Telemetry Receiver");
    resize(900, 700);

    m_altimeter = new AltimeterWidget(this);
    m_altimeter->setMinimumSize(200, 200);

    m_speedometer = new SpeedometerWidget(this);
    m_speedometer->setMinimumSize(200, 200);

    m_attitude = new AttitudeWidget(this);
    m_attitude->setMinimumSize(250, 250);

    m_compass = new CompassWidget(this);
    m_compass->setMinimumSize(200, 200);

    m_gpsLabel = new QLabel("Lat: --\nLon: --", this);
    m_gpsLabel->setStyleSheet("font-size: 14px; padding: 4px;");

    m_batteryLabel = new QLabel("Battery:", this);
    m_batteryBar = new QProgressBar(this);
    m_batteryBar->setRange(0, 100);
    m_batteryBar->setTextVisible(true);

    m_satLabel = new QLabel("Satellites: --", this);
    m_satLabel->setStyleSheet("font-size: 14px; padding: 4px;");

    m_errorLabel = new QLabel("Errors: 0", this);
    m_errorLabel->setStyleSheet("font-size: 12px; color: red; padding: 4px;");

    m_timestampLabel = new QLabel("Waiting for data...", this);
    m_timestampLabel->setStyleSheet("font-size: 12px; padding: 4px;");

    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_altimeter);
    topLayout->addWidget(m_speedometer);
    topLayout->addWidget(m_attitude);
    topLayout->addWidget(m_compass);

    auto *infoLayout = new QGridLayout;
    infoLayout->addWidget(m_gpsLabel, 0, 0);
    infoLayout->addWidget(m_batteryLabel, 0, 1);
    infoLayout->addWidget(m_batteryBar, 0, 2);
    infoLayout->addWidget(m_satLabel, 1, 0);
    infoLayout->addWidget(m_errorLabel, 1, 1);
    infoLayout->addWidget(m_timestampLabel, 2, 0, 1, 3);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(infoLayout);

    m_uiTimer = new QTimer(this);
    m_uiTimer->start(1000);
}

void MainWindow::onTelemetryReceived(const TelemetryPacket &pkt)
{
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

// ========================== BaseGaudgeWidget ============================

BaseGaudgeWidget::BaseGaudgeWidget(QWidget *parent)
    : QWidget(parent)
{
}

void BaseGaudgeWidget::paintEvent(QPaintEvent *event)
{
    // Инициализация инструмента рисования в виджете
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Выделение на физическом экране квадрата
    int side = qMin(width(), height());
    p.setViewport((width() - side) / 2, (height() - side) / 2, side, side);

    // Связывание виртуальных координат с рисовалкой
    p.setWindow(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    drawGaudge(p);
}

// ============================== AltimeterWidget ==============================

AltimeterWidget::AltimeterWidget(QWidget *parent)
    : BaseGaudgeWidget(parent)
{
}

void AltimeterWidget::setAltitude(float alt)
{
    m_altitude = alt;
    update();
}

void AltimeterWidget::drawGaudge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::black);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    // Изображение текста (ALT)
    p.setPen(Qt::white);
    p.setFont(QFont("monospace", 10));
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
    p.setPen(QPen(Qt::red, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    // Изображение единицы измерения шкалы высоты
    p.setPen(Qt::white);
    p.setFont(QFont("monospace", 12, QFont::Bold));
    p.drawText(-25, 10, QString::number(static_cast<int>(m_altitude)) + "m");
}

// =============== SpeedometerWidget =================

SpeedometerWidget::SpeedometerWidget(QWidget *parent)
    : BaseGaudgeWidget(parent)
{
}

void SpeedometerWidget::setSpeed(float speed)
{
    m_speed = speed;
    update();
}

void SpeedometerWidget::drawGaudge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::black);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    // Изображение текста (SPD)
    p.setPen(Qt::white);
    p.setFont(QFont("monospace", 10));
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
    p.setPen(QPen(Qt::red, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    // Изображение текста единицы измерения скорости
    p.setPen(Qt::white);
    p.setFont(QFont("monospace", 12, QFont::Bold));
    p.drawText(-25, 10, QString::number(static_cast<int>(m_speed)) + "m/s");
}

// ============================== AttitudeWidget ==============================

AttitudeWidget::AttitudeWidget(QWidget *parent)
    : BaseGaudgeWidget(parent)
{
}

void AttitudeWidget::setPitchRoll(float pitch, float roll)
{
    m_pitch = pitch;
    m_roll = roll;
    update();
}

void AttitudeWidget::drawGaudge(QPainter &p)
{
    // Включаем обрезку по рамке
    p.setClipRect(widgetLeft + 2, widgetTop + 2, widgetWidth - 4, widgetHeight - 4);

    // Рисование динамичной части
    p.save();
    p.translate(0, 0);
    p.rotate(-m_roll);

    int pitchOffset = static_cast<int>(m_pitch * 2.0f);

    // Небо
    p.setBrush(QColor(80, 130, 200));
    p.setPen(Qt::NoPen);
    QPolygon sky;
    // Домножение на 2 нужно чтобы не образовывались пустоты при повороте
    sky << QPoint(widgetLeft * 2, widgetTop * 2) << QPoint(widgetRight * 2, widgetTop * 2) << QPoint(widgetRight * 2, pitchOffset)
        << QPoint(widgetLeft * 2, pitchOffset);
    p.drawPolygon(sky);

    // Земля
    p.setBrush(QColor(139, 90, 43));
    QPolygon ground;
    // Домножение на 2 нужно чтобы не образовывались пустоты при повороте
    ground << QPoint(widgetLeft * 2, pitchOffset) << QPoint(widgetRight * 2, pitchOffset) << QPoint(widgetRight * 2, widgetBottom * 2)
           << QPoint(widgetLeft * 2, widgetBottom * 2);
    p.drawPolygon(ground);

    // Линия горизонта
    p.setPen(QPen(Qt::white, 2));
    // Домножение на 2 нужно чтобы не образовывались обрывы горизонта при повороте
    p.drawLine(widgetLeft * 2, pitchOffset, widgetRight * 2, pitchOffset);

    p.restore();

    // Отрисовка статической части
    p.setClipping(false);
    p.setPen(QPen(Qt::white, 2));
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
    : BaseGaudgeWidget(parent)
{
}

void CompassWidget::setYaw(float yaw)
{
    m_yaw = yaw;
    update();
}

void CompassWidget::drawGaudge(QPainter &p)
{
    // Изображение подложки циферблата
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(Qt::black);
    p.drawEllipse(widgetLeft, widgetTop, widgetWidth, widgetHeight);

    p.setFont(QFont("monospace", 9));
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
    p.setPen(QPen(Qt::red, 3));
    p.drawLine(0, 0, static_cast<int>(70 * std::cos(needleRad)), static_cast<int>(70 * std::sin(needleRad)));

    p.setPen(Qt::white);
    p.setFont(QFont("monospace", 10, QFont::Bold));
    p.drawText(-15, 10, QString::number(static_cast<int>(m_yaw)) + QChar(0xB0));
}
