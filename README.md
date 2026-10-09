# UAV Telemetry System

## 1. Идея

Два приложения для симуляции и отображения телеметрии БПЛА:

- **`sender`** — консольная утилита, отправляет бинарные UDP-пакеты с данными телеметрии по таймеру.
- **`receiver`** — Qt6 GUI-приложение, принимает пакеты, парсит и отображает данные в реальном времени на приборной панели со шкалами и стрелками.

## 2. Стек

| Компонент | Технология |
|-----------|------------|
| Язык | C++17 |
| Фреймворк | Qt6 (Widgets, Network, Test) |
| Сборка | CMake 3.20+ |
| Генератор | Ninja |
| Компиляторы | GCC (debug), Clang (release) |
| Тестирование | QTest |
| Лицензия | GPLv3 |

## 3. Бинарный протокол (little-endian, фикс. размер)

| Смещение | Поле | Тип | Размер | Описание |
|----------|------|-----|--------|----------|
| 0 | magic[0] | uint8 | 1 | 0x55 |
| 1 | magic[1] | uint8 | 1 | 0xAA |
| 2 | timestamp | uint32 | 4 | unix ms |
| 6 | altitude | float32 | 4 | метры |
| 10 | speed | float32 | 4 | м/с |
| 14 | pitch | float32 | 4 | градусы |
| 18 | roll | float32 | 4 | градусы |
| 22 | yaw | float32 | 4 | градусы |
| 26 | latitude | float64 | 8 | градусы |
| 34 | longitude | float64 | 8 | градусы |
| 42 | battery | uint8 | 1 | проценты |
| 43 | satellites | uint8 | 1 | кол-во |
| 44 | crc16 | uint16 | 2 | CRC-16/CCITT |
| **Всего** | | | **46** | |

## 4. Компоненты

### 4.1 `common` — статическая библиотека

- `TelemetryPacket` — структура (46 байт) + статические методы:
  - `pack(...)` — заполняет буфер из значений
  - `unpack(...)` — парсит буфер в поля (с валидацией magic + CRC)
  - `crc16(...)` — расчёт CRC-16/CCITT
  - enum-ы для индексов полей (опционально)

### 4.2 `sender` — консольная утилита

- Аргументы CLI (QCommandLineParser):
  - `--port` / `-p` : порт назначения (по умолч. 12345)
  - `--addr` / `-a` : адрес (по умолч. 127.0.0.1)
  - `--rate` / `-r` : интервал в мс (по умолч. 50)
- Генерация телеметрии:
  - Altitude: синусоида вокруг 100 м ±50
  - Speed: синусоида вокруг 15 м/с ±10
  - Pitch/Roll/Yaw: плавно меняются
  - GPS: статическая точка + шум
  - Battery: медленно убывает 100→20, затем сброс
  - Satellites: 8–12 случайно
- Цикл: QTimer + QUdpSocket::writeDatagram

### 4.3 `receiver` — Qt6 GUI

**UdpWorker (QObject)**
- Живёт в отдельном QThread
- Создаёт QUdpSocket, binds на порт
- При получении пакета парсит TelemetryPacket::unpack
- Испускает сигнал `dataReceived(TelemetryPacket)`

**MainWindow (QWidget)**
- Принимает сигнал от UdpWorker через QMetaObject::invokeMethod / сигнал-слот
- Обновляет приборную панель:
  - **Высотомер** — круглая шкала со стрелкой (QPainter или custom widget)
  - **Спидометр** — дуговая шкала со стрелкой
  - **Авиагоризонт** — pitch/roll (прямоугольник с линией горизонта)
  - **GPS карта/координаты** — текстовое отображение lat/lon
  - **Батарея** — прогресс-бар
  - **Спутники** — числовое значение
  - **Compass** — круг с указателем yaw

### 4.4 `tests` — QTest

- `test_telemetry.cpp`:
  - pack → buffer → unpack → сравнение полей
  - CRC правильный и повреждённый пакет
  - Граничные значения (NaN, inf, пустой буфер, wrong magic)

## 5. Потокобезопасность

- UdpWorker::dataReceived — сигнал, автоматически потокобезопасен
- Доступ к TelemetryPacket — только по значению (копируется)
- UI-виджеты обновляются только в главном потоке (гарантия Qt)

## 6. Обработка ошибок

- sender: не смог отправить — пишет в stderr, продолжает
- receiver: bad CRC/magic — дроп пакета, инкремент счётчика ошибок
- receiver: порт занят — QMessageBox и выход
- Все ошибки логируются через qWarning/qDebug

## 7. План реализации

1. Создать корневой CMakeLists.txt (project, Qt6 find, subdirectories)
2. Реализовать common: TelemetryPacket.h/.cpp + CRC
3. Реализовать sender/main.cpp
4. Реализовать receiver: UdpWorker, custom gauge widgets, MainWindow
5. Реализовать tests/test_telemetry.cpp
6. Проверить сборку (gcc-debug + clang-release)
7. Поправить .zed/debug.json (game → receiver)
8. Запустить тесты