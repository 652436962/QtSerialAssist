# QtSerialAssist

跨平台串口/网络调试助手，基于 Qt6 开发。

## 功能

- **串口调试**：波特率/数据位/校验位/流控设置，HEX/ASCII 收发
- **网络调试**：TCP 客户端/服务器、UDP 通信
- **Modbus**：RTU/ASCII/TCP 格式请求帧生成，CRC16/LRC 校验
- **快捷指令**：分组管理，支持延时顺序发送，配置持久化为 JSON 文件
- **深色主题**：现代化深色 UI

## 构建

### 依赖

- Qt 6.5+（Core、Widgets、Network）
- Qt SerialPort（可选，用于串口功能）

### Linux

```bash
sudo apt install qt6-base-dev libqt6serialport6-dev cmake g++
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
./QtSerialAssist
```

### Windows

```powershell
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --parallel
.\Release\QtSerialAssist.exe
```

### macOS

```bash
brew install qt@6
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
open QtSerialAssist.app
```

## 快捷指令配置

`commands/` 目录下的 JSON 文件定义快捷指令分组，程序启动时自动加载，运行时可通过界面增删改。

```json
{
    "group": "分组名称",
    "commands": [
        {"name": "指令名称", "data": "指令数据\r\n"}
    ]
}
```

## 项目结构

| 文件 | 说明 |
|------|------|
| `mainwindow.h/cpp` | 主窗口逻辑 |
| `mainwindow.ui` | UI 布局 |
| `CMakeLists.txt` | 构建配置 |
| `commands/` | 快捷指令 JSON 配置 |

## 许可

MIT License
