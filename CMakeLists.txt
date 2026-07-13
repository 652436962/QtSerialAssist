cmake_minimum_required(VERSION 3.19)
project(QtSerialAssist
    VERSION 2.1.10
    DESCRIPTION "跨平台串口/网络调试助手"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message(STATUS "构建平台: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Network Quick QuickControls2)

qt_standard_project_setup()

qt_add_executable(QtSerialAssist
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/backend/AppCore.cpp
    src/backend/AppCore.h
    src/backend/SerialPortManager.cpp
    src/backend/SerialPortManager.h
    src/backend/NetworkManager.cpp
    src/backend/NetworkManager.h
    src/backend/ModbusEngine.cpp
    src/backend/ModbusEngine.h
    src/backend/QuickCmdManager.cpp
    src/backend/QuickCmdManager.h
    src/backend/DataProcessor.cpp
    src/backend/DataProcessor.h
    src/backend/SettingsManager.cpp
    src/backend/SettingsManager.h
    resources.qrc
)

qt_add_resources(QtSerialAssist "qml"
    PREFIX "/"
    FILES
        qml/main.qml
        qml/pages/LeftPanel.qml
        qml/pages/RightPanel.qml
        qml/pages/ModbusPanel.qml
        qml/pages/QuickCmdPanel.qml
        qml/pages/SettingsPage.qml
        qml/components/DButton.qml
        qml/components/DComboBox.qml
        qml/components/DropWindow.qml
        qml/components/DSwitch.qml
        qml/components/DTextEdit.qml
        qml/components/DTabBar.qml
        qml/components/DTabButton.qml
        qml/theme/DeepinTheme.qml
)

target_link_libraries(QtSerialAssist
    PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::Quick
        Qt::QuickControls2
)

# 串口支持（可选，Windows/Linux 需要对应开发包）
include(CheckIncludeFileCXX)
check_include_file_cxx("QSerialPort" HAS_SERIAL_PORT_HEADER)

if(TARGET Qt6::SerialPort)
    set(HAS_SERIAL_PORT ON)
    target_link_libraries(QtSerialAssist PRIVATE Qt::SerialPort)
    message(STATUS "串口支持: 已启用")
else()
    find_package(Qt6 QUIET COMPONENTS SerialPort)
    if(Qt6SerialPort_FOUND)
        set(HAS_SERIAL_PORT ON)
        target_link_libraries(QtSerialAssist PRIVATE Qt6::SerialPort)
        message(STATUS "串口支持: 已启用")
    else()
        set(HAS_SERIAL_PORT OFF)
        message(STATUS "串口支持: 未启用 (安装 qt6-serialport-dev 可启用)")
    endif()
endif()

if(HAS_SERIAL_PORT)
    target_compile_definitions(QtSerialAssist PRIVATE HAS_SERIAL_PORT)
endif()

# 翻译文件：使用 lrelease 编译 .ts → .qm
find_program(LRELEASE_EXECUTABLE lrelease-qt6 lrelease HINTS ${QT_HOST_PATH}/bin ${Qt6_DIR}/../../../bin)
if(LRELEASE_EXECUTABLE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/QtSerialAssist_zh_CN.qm
        COMMAND ${LRELEASE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/QtSerialAssist_zh_CN.ts -qm ${CMAKE_BINARY_DIR}/QtSerialAssist_zh_CN.qm
        DEPENDS ${CMAKE_SOURCE_DIR}/QtSerialAssist_zh_CN.ts
        COMMENT "生成翻译文件 QtSerialAssist_zh_CN.qm"
    )
    add_custom_target(translations ALL DEPENDS ${CMAKE_BINARY_DIR}/QtSerialAssist_zh_CN.qm)
    add_dependencies(QtSerialAssist translations)
    message(STATUS "翻译支持: lrelease 已找到 (${LRELEASE_EXECUTABLE})")
else()
    message(WARNING "翻译支持: lrelease 未找到，跳过翻译文件生成")
endif()

# post-build: 复制 commands 目录
add_custom_command(TARGET QtSerialAssist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/commands
        ${CMAKE_BINARY_DIR}/commands
    COMMENT "复制 commands 目录"
)

# 编译选项
if(MSVC)
    target_compile_options(QtSerialAssist PRIVATE /W4)
else()
    target_compile_options(QtSerialAssist PRIVATE -Wall -Wextra)
endif()

# macOS / Windows 特定设置
if(APPLE)
    set_target_properties(QtSerialAssist PROPERTIES
        MACOSX_BUNDLE TRUE
    )
    # aqtinstall 的 Qt 预编译包在旧版 macOS SDK 上构建，
    # CMake import target 中仍会注入 -framework AGL/OpenGL 强引用。
    # macOS 15+ 已移除这些框架，需要桩框架 + 弱链接配合
    if(EXISTS "${CMAKE_SOURCE_DIR}/stub/AGL.framework")
        target_link_options(QtSerialAssist PRIVATE
            "LINKER:-F${CMAKE_SOURCE_DIR}/stub"
            "LINKER:-weak_framework,AGL"
            "LINKER:-weak_framework,OpenGL"
        )
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-weak_framework,AGL -Wl,-weak_framework,OpenGL")
    endif()
endif()
