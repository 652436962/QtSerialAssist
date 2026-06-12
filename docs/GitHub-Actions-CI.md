# GitHub Actions CI/CD 配置指南

跨平台 Qt6 CMake 项目的 CI 构建 + 自动 Release 发布模板。

## 功能概览

| 事件 | 行为 |
|------|------|
| `push` 到 `main` | 三平台编译 + 上传 Artifacts |
| `push` `v*` 标签 | 同上 + 自动创建 GitHub Release |
| `pull_request` | 三平台编译验证 |
| `release: published` | 下载产物并发布到 Release |
| `workflow_dispatch` | 手动触发，输入版本号 |

## 平台产物

| 平台 | Runner | 产物 |
|------|--------|------|
| Linux | `ubuntu-22.04` | `.AppImage` / `.deb` / `.rpm` |
| Windows | `windows-2022` | `.zip`（含 windeployqt 捆绑） |
| macOS | `macos-latest` | `.zip`（含 .app 捆绑） |

## 流程详解

### 1. Qt 安装

使用 [install-qt-action](https://github.com/jurplel/install-qt-action)，统一跨平台安装：

```yaml
- uses: jurplel/install-qt-action@v4
  with:
    version: '6.5.3'          # Qt 版本
    modules: 'qtserialport'   # 附加模块，基础模块(Core/Widgets/Network)不需要
    cache: true               # 缓存加速
```

> **常见坑**：Qt6 的 Core/Widgets/Network 是基础模块，`modules` 只填附加模块名（如 `qtserialport`），**不要填** `qtnetwork`。

### 2. 编译

多配置生成器（Visual Studio / Xcode）必须加 `--config Release`：

```yaml
- run: cmake -B build -DCMAKE_BUILD_TYPE=Release
- run: cmake --build build --config Release --parallel
```

> **单配置**（Unix Makefiles / Ninja）可省略 `--config`，但加上也不报错，建议统一写。

### 3. Linux 打包

#### AppImage

使用 [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) + [linuxdeploy-plugin-qt](https://github.com/linuxdeploy/linuxdeploy-plugin-qt)：

```bash
# 准备 AppDir
mkdir -p AppDir/usr/bin AppDir/usr/share/applications

# 创建 desktop 文件和 AppRun
cat > AppDir/usr/share/applications/app.desktop << 'EOF'
[Desktop Entry]
Type=Application
Name=MyApp
Exec=MyApp
Icon=myapp
Categories=Development;
Terminal=false
EOF

cat > AppDir/AppRun << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="$HERE/usr/bin:$PATH"
export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$HERE/usr/plugins"
exec "$HERE/usr/bin/MyApp" "$@"
EOF
chmod +x AppDir/AppRun

# 下载并运行 linuxdeploy
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage
```

#### deb

```bash
mkdir -p deb-pkg/DEBIAN
cp -r AppDir/usr deb-pkg/
cat > deb-pkg/DEBIAN/control << EOF
Package: myapp
Version: 1.0.0
Section: utils
Architecture: amd64
Maintainer: Developer
Description: My Application
EOF
dpkg-deb --build deb-pkg myapp_1.0.0_amd64.deb
```

#### rpm

通过 alien 从 deb 转换：

```bash
sudo apt-get install -y alien
alien -r --scripts myapp_1.0.0_amd64.deb
```

### 4. Windows 打包

使用 Qt 自带的 `windeployqt` 捆绑 DLL，再用 `Compress-Archive` 打包 zip：

```powershell
windeployqt --release --no-translations build/Release/MyApp.exe
Compress-Archive -Path build/Release/* -DestinationPath MyApp-Windows-x86_64.zip
```

> `commands/` 等运行时目录通过 CMake post-build 自动拷贝到 `build/Release/`，无需手动处理。

### 5. macOS 打包

```bash
cd build
zip -r ../MyApp-macOS-x86_64.zip MyApp.app
```

### 6. 自动 Release

```yaml
release:
  needs: build
  if: github.event_name == 'release' || github.event_name == 'workflow_dispatch' || startsWith(github.ref, 'refs/tags/v')
  runs-on: ubuntu-latest
  permissions:
    contents: write          # 必须，否则 403
  steps:
    - uses: actions/download-artifact@v4
      with:
        path: artifacts
    - run: |
        mkdir -p release
        find artifacts -type f \( -name '*.AppImage' -o -name '*.deb' -o -name '*.rpm' -o -name '*.zip' \) -exec cp {} release/ \;
    - uses: softprops/action-gh-release@v2
      with:
        files: release/*
        generate_release_notes: true
```

> **关键**：`permissions: contents: write` 必不可少，默认 GITHUB_TOKEN 无权创建 Release。

## 动态版本号

从标签或手动输入提取，避免硬编码：

```bash
if [ "${{ github.event_name }}" = "workflow_dispatch" ]; then
  VERSION=${{ inputs.version }}
elif [ "${GITHUB_REF#refs/tags/v}" != "$GITHUB_REF" ]; then
  VERSION=${GITHUB_REF#refs/tags/v}
else
  VERSION=1.0.0
fi
```

## 手动触发 Release

推送 `v*` 标签即可触发完整的构建 + 自动发布流程：

```bash
# 本地打标签并推送
git tag -a v1.0.1 -m "Release v1.0.1"
git push origin v1.0.1
```

推送后前往 https://github.com/652436962/QtSerialAssist/actions 查看构建进度，构建完成后 Release 自动出现在 https://github.com/652436962/QtSerialAssist/releases。

> **注意**：标签必须匹配 `v*` 模式（如 `v1.0.0`、`v2.1.3`），否则不会触发 Release。

## 复用清单

适配到新项目时，主要修改以下位置：

| 位置 | 修改内容 |
|------|----------|
| `jurplel/install-qt-action` `modules` | 项目的额外 Qt 模块 |
| 编译产物路径 | `build/MyApp` / `build/Release/MyApp.exe` |
| AppDir/desktop/control 中的应用名 | `MyApp` |
| `find` 命令的文件名匹配 | `*.AppImage` / `*.deb` / 等 |
| `dpkg-deb` / `alien` 包名 | `myapp` |
| Linux 构建依赖 | `apt-get install` 额外系统库 |
| macOS runner | 当前兼容版本 `macos-latest`（`macos-13` 已弃用） |

## 踩坑记录

1. **`qtnetwork` 不存在** — Qt6 基础模块无需在 `modules` 中列出
2. **`--config Release`** — 多配置生成器必须加，否则找不到 `build/Release/`
3. **`permissions: contents: write`** — Release 403 错误就是这个
4. **`macos-13` 已弃用** — 改用 `macos-latest`
5. **Release 产物散开** — 用 `find` 过滤只取包文件（AppImage/deb/rpm/zip），不扁平化
6. **`commands/` 路径** — CMake post-build 已拷贝到构建输出目录，不要在 CI 中重复复制
7. **Windows DLL 多** — `windeployqt` 自动捆绑所有依赖 DLL 到 zip 中，这是正常现象，用户解压即用
