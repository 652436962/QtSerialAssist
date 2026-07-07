cmake_minimum_required(VERSION 3.19)
project(QtSerialAssist
    VERSION 1.0.0
    DESCRIPTION "跨平台串口/网络调试助手"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出构建平台信息
message(STATUS "构建平台: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(QtSerialAssist
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    settingsdialog.cpp
    settingsdialog.h
    resources.qrc
)

target_link_libraries(QtSerialAssist
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

# 串口支持（可选，Windows/Linux 需要对应开发包）
find_package(Qt6 COMPONENTS SerialPort)
if(Qt6SerialPort_FOUND)
    target_link_libraries(QtSerialAssist PRIVATE Qt::SerialPort)
    target_compile_definitions(QtSerialAssist PRIVATE HAS_SERIAL_PORT)
    message(STATUS "串口支持: 已启用")
else()
    message(STATUS "串口支持: 未找到 Qt6::SerialPort，已禁用")
endif()

# 平台特定编译选项
if(MSVC)
    target_compile_options(QtSerialAssist PRIVATE /W4 /utf-8)
    target_link_options(QtSerialAssist PRIVATE /MANIFEST:NO)
else()
    target_compile_options(QtSerialAssist PRIVATE -Wall -Wextra)
endif()

# macOS 特定设置
if(APPLE)
    set_target_properties(QtSerialAssist PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

install(TARGETS QtSerialAssist
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QtSerialAssist
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# 复制快捷指令配置到构建目录
add_custom_command(TARGET QtSerialAssist POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/commands
    $<TARGET_FILE_DIR:QtSerialAssist>/commands
)

# 编译翻译文件并复制到构建目录
set(LRELEASE_EXECUTABLE "/usr/lib/qt5/bin/lrelease")
if(EXISTS ${LRELEASE_EXECUTABLE})
    add_custom_command(TARGET QtSerialAssist POST_BUILD
        COMMAND ${LRELEASE_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/QtSerialAssist_zh_CN.ts
        -qm $<TARGET_FILE_DIR:QtSerialAssist>/QtSerialAssist_zh_CN.qm
        COMMENT "生成翻译文件 QtSerialAssist_zh_CN.qm"
    )
else()
    message(WARNING "lrelease not found, skipping translation file generation")
endif()
