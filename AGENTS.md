# QtSerialAssist Agent Guide

**全局规则：使用中文回复用户。**

## Build & Run
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/QtSerialAssist
```
Release: `-DCMAKE_BUILD_TYPE=Release`; Windows: `--config Release`, run `.\build\Release\QtSerialAssist.exe`.

## Dependencies
- Qt 6.5+ (Core, Gui, Network, Quick, QuickControls2)
- SerialPort (optional): `sudo apt install libqt6serialport6 qt6-serialport-dev`

## Architecture
Single-binary **Qt6 QML** desktop app v2.0.1. `AppCore` (C++ backend) exposed to QML as `core` context property. All QML UI under `qml/`.

### Key files
| File | Purpose |
|------|---------|
| `src/main.cpp` | App entrypoint: QGuiApplication, translator, AppCore + QQmlApplicationEngine setup |
| `src/backend/AppCore.h/cpp` | Central backend hub, owned by main; bridges QML ↔ C++ modules |
| `src/backend/SerialPortManager.*` | QSerialPort wrapper (guarded by `HAS_SERIAL_PORT`) |
| `src/backend/NetworkManager.*` | QTcpSocket/QUdpSocket wrapper |
| `src/backend/ModbusEngine.*` | Modbus RTU/ASCII/TCP frame builder + CRC/LRC |
| `src/backend/QuickCmdManager.*` | Load/save/manage quick-command JSON groups |
| `src/backend/DataProcessor.*` | HEX↔ASCII conversion, RX/TX counters, framing |
| `src/backend/SettingsManager.*` | Theme, language, settings persistence |
| `qml/main.qml` | Main window layout (header, left panel loader, right panel, status bar) |
| `qml/pages/` | LeftPanel (serial/net), ModbusPanel, QuickCmdPanel, RightPanel, SettingsPage |
| `qml/components/` | DButton, DComboBox, DSwitch, DTextEdit, DTabBar, DropWindow |
| `qml/theme/DeepinTheme.qml` | Singleton theme (colors, fonts, spacing); switched via `themeMode` property |
| `commands/*.json` | Quick-command groups, copied to build dir post-build |

### Architecture notes
- **No stale v1 files are built**: root-level `mainwindow.*`, `mainwindow.ui`, `settingsdialog.*` are leftovers from a previous Qt Widgets version. CMake only compiles `src/main.cpp` + `src/backend/*`.
- SerialPort is conditional (`HAS_SERIAL_PORT` compile definition). Without it, serial UI elements are hidden/disabled at runtime.
- Theme is a QML singleton (`DeepinTheme.qml`). Theme switching changes `themeMode` property (0=dark, 1=light, 2=system). No QSS stylesheets.
- `commands/` must exist next to binary at runtime (post-build copies it). Fallback: single hardcoded "默认" group.
- `QtSerialAssist_zh_CN.qm` is searched at `applicationDirPath()`, `../`, and `:/` (qrc) — see `src/main.cpp:29-33`.
- i18n: after loading a new translator, QML UI rebinds automatically via context properties.

## CI / Packaging
- CI (`.github/workflows/build.yml`): Qt 6.8.2 + `qtserialport`, cross-platform (ubuntu-22.04, windows-2022, macos-latest). CMake configure + build only (no test/lint step).
- Linux packaging: AppImage (via linuxdeploy + qt plugin), deb, rpm
- Windows: `windeployqt` → zip
- macOS: zip the `.app` bundle

## Gotchas
- **`qt.conf` sets Qt6 prefix** to `/usr/lib/x86_64-linux-gnu/qt6` — only relevant for running system Qt without env vars.
- **macOS builds** need stub AGL/OpenGL frameworks + weak-link flags (`CMakeLists.txt:132-137`). CI creates empty dylibs under `stub/` before configure because aqtinstall's Qt pre-built binaries carry stale `-framework AGL` references on macOS 15+. CI disables Qt cache to avoid stale configs.
- **No lrelease Qt5 gotcha anymore** — `CMakeLists.txt:93` uses `find_program(LRELEASE_EXECUTABLE lrelease-qt6 lrelease ...)`, correctly finding Qt6's lrelease first.
- **No tests, no lint/formatter config**. No `.clang-format`, no `.clang-tidy`, no test framework.
- **Pre-built `build_linux/`** exists in-repo (out-of-source build artifact, tracked).
