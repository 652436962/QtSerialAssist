# QtSerialAssist Agent Guide

## Build & Run
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
./QtSerialAssist
```
A pre-built `build_linux/` directory already exists in-repo (out-of-source build). CI builds with Qt 6.5.3 on ubuntu-22.04/windows-2022/macos-13.

## Dependencies
- Qt 6.5+ (Core, Widgets, Network)
- SerialPort (optional): `sudo apt install libqt6serialport6 qt6-serialport-dev`

## Architecture
Single-binary Qt6 desktop app. One `MainWindow` class drives everything (serial, TCP/UDP, theme, i18n, quick commands, modbus, terminal mode). `SettingsDialog` is a standalone QDialog for theme/language.

### Key files
| File | Purpose |
|------|---------|
| `mainwindow.h/cpp` | All application logic (serial/TCP/UDP, send/receive, theme, quick cmds, modbus, terminal mode) |
| `mainwindow.ui` | Qt Designer UI layout |
| `settingsdialog.h/cpp` | Theme + language settings dialog |
| `CMakeLists.txt` | Build config (SerialPort conditional, lrelease, post-build copy) |

### Runtime data dirs
- `commands/` — JSON files for quick-command groups (copied to build dir by CMake post-build). Each JSON: `{"group": "...", "commands": [{"name": "...", "data": "..."}]}`.
- `QtSerialAssist_zh_CN.qm` — built from `QtSerialAssist_zh_CN.ts`, looked up next to the binary at runtime.

## Gotchas

### lrelease path is hardcoded to Qt5
`CMakeLists.txt:82` has `set(LRELEASE_EXECUTABLE "/usr/lib/qt5/bin/lrelease")`. This is wrong on Qt6 systems; `lrelease` won't be found and `.qm` generation is silently skipped. On Ubuntu the correct path is usually `/usr/lib/qt6/bin/lrelease` or use CMake's `qt6_add_translations`.

### SerialPort is conditional (`HAS_SERIAL_PORT`)
All serial code is guarded by `#ifdef HAS_SERIAL_PORT`. The macro is defined only when CMake finds `Qt6::SerialPort`. Without it, the serial tab is disabled at runtime and `QSerialPort`/`QSerialPortInfo` are not included.

### `retranslateUi()` clobbers styles and labels
After calling `ui->retranslateUi(this)` (language switch), you **must** re-apply the theme via `applyTheme()` and restore any dynamic label text (RX/TX counters). See `mainwindow.cpp:1172-1186`.

### Theme is raw QSS stylesheets
`applyTheme(int)` in `mainwindow.cpp:1047` hardcodes two complete QSS strings (dark/light). No `.qss` files are loaded from disk. Any styling change must be done in that method.

### No tests, no lint/formatter config
The repo has no test suite and no lint/format configuration (no `.clang-format`, no `.clang-tidy`). CI only does cross-platform build verification.

### `commands/` must exist next to binary at runtime
`findCommandsDir()` in `mainwindow.cpp:821` searches relative to `applicationDirPath()` and `currentPath()`. If the binary is run without `commands/` nearby, quick-command groups fall back to a single hardcoded "默认" group.
