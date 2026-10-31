#include "Theme.h"
#include <QtTest>

class TestTheme : public QObject
{
    Q_OBJECT

private slots:
    void testBatteryStateBoundaries();
};

void TestTheme::testBatteryStateBoundaries()
{
    // Границы: предупреждение < 40%, критично < 25%
    QVERIFY(Theme::batteryState(100) == Theme::BatteryState::Ok);
    QVERIFY(Theme::batteryState(41) == Theme::BatteryState::Ok);
    QVERIFY(Theme::batteryState(40) == Theme::BatteryState::Ok);
    QVERIFY(Theme::batteryState(39) == Theme::BatteryState::Low);
    QVERIFY(Theme::batteryState(26) == Theme::BatteryState::Low);
    QVERIFY(Theme::batteryState(25) == Theme::BatteryState::Low);
    QVERIFY(Theme::batteryState(24) == Theme::BatteryState::Critical);
    QVERIFY(Theme::batteryState(0) == Theme::BatteryState::Critical);
}

QTEST_MAIN(TestTheme)
#include "test_theme.moc"
