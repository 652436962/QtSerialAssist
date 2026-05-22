cmake_minimum_required(VERSION 3.19)
project(QtSerialAssist LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(QtSerialAssist
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(QtSerialAssist
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

find_package(Qt6 COMPONENTS SerialPort)
if(Qt6SerialPort_FOUND)
    target_link_libraries(QtSerialAssist PRIVATE Qt::SerialPort)
    target_compile_definitions(QtSerialAssist PRIVATE HAS_SERIAL_PORT)
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
