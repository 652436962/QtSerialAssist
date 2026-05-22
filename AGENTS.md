# QtSerialAssist Agent Guide

## Build & Run
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel
./QtSerialAssist
```

## Dependencies
- Qt 6.5+ (Core, Widgets, Network)
- SerialPort (optional): `sudo apt install libqt6serialport6 qt6-serialport-dev`

## Key Files
| File | Purpose |
|------|---------|
| `mainwindow.h/cpp` | 主窗口逻辑（串口/TCP/UDP） |
| `mainwindow.ui` | Qt Designer UI 布局 |
| `preview.html` | UI 效果预览（深色主题） |
| `任务文档.md` | 功能规划与待办事项 |
| `CMakeLists.txt` | 构建配置（SerialPort 条件编译） |

## UI Features (from preview.html)
- 左侧面板：串口设置（波特率/数据位/校验位/流控）+ 网络设置（TCP/UDP）
- 右侧面板：收发数据显示 + 发送区（HEX/ASCII 模式、定时发送）
- 工具栏按钮：快捷指令、批量发送、历史发送、自动应答、数据波形、Modbus 指令、ASCII 码表

## Notes
- SerialPort conditional: `#ifdef HAS_SERIAL_PORT` enables serial support when libs present
- `qt_add_executable` auto-processes `.ui` files
- i18n: `QtSerialAssist_zh_CN.ts` + `lrelease` for .qm files