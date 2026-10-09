#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "TelemetryPacket.h"
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QWidget>

class AltimeterWidget;
class SpeedometerWidget;
class AttitudeWidget;
class CompassWidget;

class MainWindow : public QWidget {
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

// ---------- Altimeter ----------
class AltimeterWidget : public QWidget {
  Q_OBJECT
public:
  explicit AltimeterWidget(QWidget *parent = nullptr);
  void setAltitude(float alt);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  float m_altitude = 0.0f;
};

// ---------- Speedometer ----------
class SpeedometerWidget : public QWidget {
  Q_OBJECT
public:
  explicit SpeedometerWidget(QWidget *parent = nullptr);
  void setSpeed(float speed);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  float m_speed = 0.0f;
};

// ---------- Attitude (pitch/roll) ----------
class AttitudeWidget : public QWidget {
  Q_OBJECT
public:
  explicit AttitudeWidget(QWidget *parent = nullptr);
  void setPitchRoll(float pitch, float roll);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  float m_pitch = 0.0f;
  float m_roll = 0.0f;
};

// ---------- Compass (yaw) ----------
class CompassWidget : public QWidget {
  Q_OBJECT
public:
  explicit CompassWidget(QWidget *parent = nullptr);
  void setYaw(float yaw);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  float m_yaw = 0.0f;
};

#endif
