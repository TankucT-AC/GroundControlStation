# Theme

## Обзор

`receiver/Theme.h` — заголовочный файл без .cpp: семантическая палитра тёмной темы и типографика. Все цвета интерфейса именованы по роли (назначению), а не по оттенку; виджеты не должны хардкодить `QColor`. Единственное нетривиальное поведение — классификация состояния батареи, покрытая тестом `tests/test_theme.cpp`.

## Пространство имён

`Theme` — единственное пространство имён, группирует цвета, шрифты и классификацию батареи.

## Константы

| Имя | Тип / значение | Описание |
|-----|----------------|----------|
| `window` | `QColor 0x14161A` | Фон окна |
| `surface` | `QColor 0x1E2228` | Панели, `QGroupBox` |
| `gaugeFace` | `QColor 0x0E0F12` | Лицевая часть приборов и графиков |
| `outline` | `QColor 0x3A404A` | Рамки, ободки циферблатов |
| `grid` | `QColor 0x2A2E36` | Линии сетки графиков |
| `textPrimary` | `QColor 0xE8EAED` | Основной текст |
| `textDim` | `QColor 0x9AA0A8` | Подписи, второстепенный текст |
| `needle` | `QColor 0xFF5A5A` | Стрелки приборов |
| `ok` | `QColor 0x4CC38A` | Норма (`LIVE`, заливка батареи, линия графика) |
| `warn` | `QColor 0xE8B44A` | Предупреждение (`PAUSED`, батарея LOW) |
| `crit` | `QColor 0xE5484D` | Критично (`LINK LOST`, батарея CRIT) |
| `sky` | `QColor 0x2E5A8F` | Авиагоризонт: небо |
| `ground` | `QColor 0x6B4A2B` | Авиагоризонт: земля |

## Типы

| Имя | Вид | Описание |
|-----|-----|---------|
| `BatteryState` | `enum class` | `Ok`, `Low`, `Critical` — состояние батареи |

## Функции

#### BatteryState batteryState(int percent)

Классифицирует заряд: `Critical` при < 25 %, `Low` при < 40 %, иначе `Ok`. Используется в `MainWindow::onTelemetryReceived` для цвета заливки `QProgressBar` и суффикса `LOW`/`CRIT` (состояние всегда дублируется текстом, не только цветом).

#### QFont fixedFont(int pointSize)

Системный моноширинный шрифт заданного кегля. Базовый шрифт создаётся один раз (function-local static), возвращаются дёшево копируемые COW-копии.

#### QFont fixedFontBold(int pointSize)

Вариант с жирным начертанием — выделенные значения приборов.

## Пример использования

```cpp
p.setPen(Theme::textPrimary);
p.setFont(Theme::fixedFontBold(11));

switch (Theme::batteryState(23)) { // BatteryState::Critical
}
```
