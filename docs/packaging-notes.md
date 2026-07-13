# Linux 打包问题与修复记录

## 1. 平台插件缺失（dxcb）

### 现象
```
qt.qpa.plugin: Could not find the Qt platform plugin "dxcb" in ""
```

### 原因
Deepin 桌面环境设置了 `QT_QPA_PLATFORMTHEME=deepin`，导致 Qt 尝试加载 `dxcb` 平台插件。但 linuxdeploy 打包的 Qt 预编译库只包含标准 `xcb`，不含 Deepin 定制的 `dxcb`。

### 修复
AppRun / 启动脚本中强制指定标准平台插件并清空主题覆盖：

```bash
export QT_QPA_PLATFORM=xcb
export QT_QPA_PLATFORMTHEME=
```

同时放置 `qt.conf` 为插件路径兜底。

---

## 2. QML 模块缺失

### 现象
```
QQmlApplicationEngine failed to load component
qrc:/qml/main.qml:2:1: module "QtQuick.Controls" plugin "qtquickcontrols2plugin" not found
```

### 原因
`linuxdeploy-plugin-qt` 通过 `qmlimportscanner` 扫描二进制中的 QML 依赖，决定打包哪些 QML 模块。但本项目所有 QML 文件已编译进 qrc 嵌入二进制，`qmlimportscanner` 无法扫描到嵌入的 QML 导入，因此不部署任何 QML 模块。

### 修复
在 linuxdeploy 运行前，手动从 Qt 安装目录复制所需 QML 模块到 AppDir：

```bash
QML_SRC=$(qmake6 -query QT_INSTALL_QML 2>/dev/null || qmake -query QT_INSTALL_QML)
mkdir -p AppDir/usr/qml
[ -d "$QML_SRC/QtQml" ] && cp -r "$QML_SRC/QtQml" AppDir/usr/qml/
[ -d "$QML_SRC/QtQuick" ] && cp -r "$QML_SRC/QtQuick" AppDir/usr/qml/
```

---

## 3. deb 包文件冲突

### 现象
```
dpkg: 正试图覆盖 /usr/share/doc/libblkid1/copyright，它同时被包含于软件包 libblkid1
```

### 原因
deb 创建时用 `cp -r AppDir/usr deb-pkg/` 全量复制，将 linuxdeploy 部署时带入的系统文件（如 `share/doc/`、`share/man/`）也打进了 deb 包，与系统已安装的包冲突。

### 修复
改为只复制应用相关目录到 `/opt/QtSerialAssist/`：

```bash
cp -r AppDir/usr/lib "deb-pkg/${OPT_DIR}/"
cp -r AppDir/usr/plugins "deb-pkg/${OPT_DIR}/"
cp -r AppDir/usr/qml "deb-pkg/${OPT_DIR}/"
# 不复制 share/doc、share/man 等系统目录
```

同时将安装路径从 `/usr` 改为 `/opt`，进一步避免系统目录污染。

---

## 4. macOS AGL 框架缺失

### 现象
```
ld: framework 'AGL' not found
```

### 原因
macOS 15 (Sequoia) 移除了 AGL/OpenGL 框架。`aqtinstall` 的 Qt 预编译包在旧版 macOS SDK 上编译，CMake 导入目标中残留 `-framework AGL` 强引用。

### 修复
CI 在 cmake 配置前创建空的桩框架，配合 `target_link_options` 弱链接：

```bash
mkdir -p stub/AGL.framework stub/OpenGL.framework
clang -x c /dev/null -o stub/AGL.framework/AGL -dynamiclib
clang -x c /dev/null -o stub/OpenGL.framework/OpenGL -dynamiclib
```

```cmake
target_link_options(QtSerialAssist PRIVATE
    "LINKER:-F${CMAKE_SOURCE_DIR}/stub"
    "LINKER:-weak_framework,AGL"
    "LINKER:-weak_framework,OpenGL"
)
```

---

## 5. 版本号未同步

### 修复
版本号在两处定义，需保持同步：
- `CMakeLists.txt:3` — `VERSION` 字段
- `src/backend/AppCore.cpp:84` — `version()` 返回值

---

## 本地测试 AppImage

CI 迭代太慢，建议本地先跑通再推送。需要下载两个工具：

```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x *.AppImage
export QMAKE=qmake6
export LDAI_OUTPUT="QtSerialAssist-x86_64.AppImage"
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage
```

测试：直接运行生成的 `.AppImage`，确认无 `qt.qpa.plugin` 和 `QQmlApplicationEngine` 报错。
