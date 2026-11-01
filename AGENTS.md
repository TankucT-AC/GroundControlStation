# AGENTS.md

C++17 / Qt6 UAV telemetry: `sender` (console UDP generator) → 46-byte binary packets → `receiver` (Qt Widgets gauge dashboard). `common` is a static lib holding the wire protocol. No CI — verification is local build + ctest.

Learning material: `TUTORIAL.md` (step-by-step custom gauge widget) and `STYLEGUIDE.md` (architecture, conventions, and process used in this repo).

## Build & test

```sh
cmake --preset gcc-debug && cmake --build out/gcc-debug          # GCC Debug
cmake --preset clang-release && cmake --build out/clang-release  # Clang Release

ctest --test-dir out/gcc-debug/tests        # from tests/, NOT build root
./out/gcc-debug/tests/test_telemetry testBadCrc   # single QTest function = pass name as arg
```

`enable_testing()` lives in `tests/CMakeLists.txt`, so `ctest --test-dir out/gcc-debug` silently finds zero tests. Same for the clang-release build tree.

## Run

```sh
./out/gcc-debug/receiver/receiver -p 12345   # -p/--port: UDP listen port (default 12345)
./out/gcc-debug/sender/sender -p 12345 -a 127.0.0.1 -r 50
```

GUI controls: Space pauses the display (data keeps flowing). CSV toggle lives in the Link group.

## Protocol

Single source of truth is `common/TelemetryPacket.h` (constexpr offsets/sizes, `PACKET_SIZE = 46`), not the README table — update both together. Little-endian; CRC-16/CCITT with init `0xFFFF` (known vector `"123456789"` → `0x29B1` is asserted in tests).

## Conventions & gotchas

- `CMAKE_AUTOMOC` is ON: any file containing `Q_OBJECT` must be listed in its target's `CMakeLists.txt`.
- All UI colors go through semantic roles in `receiver/Theme.h` (`Theme::crit`, `Theme::gaugeFace`, `Theme::batteryState()`...) — don't hardcode QColor values in widgets.
- Format via `.clang-format` (WebKit base, 4-space indent, **160-col limit**, Linux braces, `Type *var`) — don't re-wrap code to 80/100 columns by hand.
- `.clangd` reads `out/gcc-debug/compile_commands.json`; configure the gcc-debug preset or LSP diagnostics go stale.
- Threading: `UdpWorker` runs on its own `QThread` and communicates only via signals (`dataReceived`, `errorOccurred`, `errorCountChanged`). UI is touched exclusively from `MainWindow` slots on the GUI thread.
- Tests live one per target in `tests/`; `test_udpworker` compiles `../receiver/UdpWorker.cpp` directly and needs the `../receiver` include path (see `tests/CMakeLists.txt`).
- Gauge widgets derive from `BaseGaugeWidget` in `receiver/MainWindow.h`; painting uses a virtual −100..100 coordinate space. History charts are a separate `HistoryChart` class using pixel coordinates.
- README and comments are Russian; commits on `main` use conventional prefixes (`refactor:`, `feat:`).
