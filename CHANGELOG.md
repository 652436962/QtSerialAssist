# Changelog

## 2.1.8

### 修复
- deb: control文件改用printf写入，避免YAML缩进导致字段前有空格

## 2.1.7

### 修复
- deb: 启动脚本恢复 QML2_IMPORT_PATH + xcb 平台设置

## 2.1.6 (2026-07-10)

### 修复
- AppImage: 手动复制 QtQml/QtQuick 模块，解决 linuxdeploy-plugin-qt 无法扫描 qrc 内 QML

## 2.1.5

### 修复
- AppImage: 恢复 QML2_IMPORT_PATH 修复 QML 模块加载

## 2.1.4 (2026-07-10)

### 修复
- Linux: AppImage/deb 添加 qt.conf 修复运行时加载不出插件
- Linux: AppImage 强制 xcb 平台避免 Deepin 主题污染

## 2.1.3 (2026-07-10)

### 修复
- Linux: deb 打包 qml 目录不存在时不中断构建

## 2.1.2 (2026-07-10)

### 变更
- Linux: deb 安装路径改为 /opt/QtSerialAssist，内置启动脚本
- Linux: AppImage/deb 使用真实应用图标替换占位图标

## 2.1.1 (2026-07-10)

### 修复
- macOS: 桩框架方案替代 CMAKE_EXE_LINKER_FLAGS，彻底解决 AGL/OpenGL 链接失败
- Linux: deb 包逐目录复制修复 usr 结构问题
- CI: 禁用 Qt 缓存避免旧版本残留

## 2.1.0 (2026-07-10)

### 新增
- Windows 安装版（NSIS 生成 .exe 安装程序，支持桌面快捷方式和开始菜单）
- macOS 分发格式从 .zip 改为 .dmg 磁盘映像

### 变更
- CI Qt 版本升级至 6.8.2，macOS runner 恢复为 macos-latest
- 统一所有平台产物命名格式：`QtSerialAssist-{version}-{platform}-{arch}.{ext}`
- Linux 包增加平台标识（`-linux-` 前缀）

### 修复
- 修复 macOS 15+ 构建失败（AGL/OpenGL 框架弱链接适配）
- 修复 install-qt-action 缓存污染导致的 AGL 链接错误

## 2.0.1 (2025-06-03)

### 新增
- QML 重构：Deepin 风格深色/浅色双主题，支持跟随系统
- 接收窗口 ScrollView，收发方向标识（RX/TX 颜色区分）
- 终端模式、Modbus 面板、快捷指令面板

### 变更
- CI Linux 打包 AppImage / deb / rpm 三种格式
- CI 自动创建 GitHub Release（推送 v* 标签触发）
- 文档添加 Linux 串口权限说明
