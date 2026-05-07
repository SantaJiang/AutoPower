cmake_minimum_required(VERSION 3.16)

project(AutoPower
    VERSION 1.2.0
    LANGUAGES CXX
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

option(AUTOPOWER_DEPLOY_QT "Deploy Qt runtime libraries during install/package when supported." ON)

set(QT_COMPONENTS Widgets LinguistTools)
if(UNIX AND NOT APPLE)
    list(APPEND QT_COMPONENTS DBus)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${QT_COMPONENTS})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    powerop.cpp
    powerop.h
    trayicon.cpp
    trayicon.h
    worker.cpp
    worker.h
    src.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AutoPower
        WIN32
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(AutoPower
        WIN32
        MACOSX_BUNDLE
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(AutoPower PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(UNIX AND NOT APPLE)
    target_link_libraries(AutoPower PRIVATE Qt${QT_VERSION_MAJOR}::DBus)
endif()

if(WIN32)
    target_link_libraries(AutoPower PRIVATE PowrProf User32 Advapi32)

    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_BINARY_DIR}/autopower.rc")
    file(WRITE "${APP_ICON_RESOURCE_WINDOWS}" "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/logo.ico\"\n")
    target_sources(AutoPower PRIVATE "${APP_ICON_RESOURCE_WINDOWS}")
endif()

set_target_properties(AutoPower PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.autopower.app
    MACOSX_BUNDLE_BUNDLE_NAME AutoPower
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
)

install(TARGETS AutoPower
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(AUTOPOWER_DEPLOY_QT)
    if(QT_VERSION_MAJOR EQUAL 6 AND COMMAND qt_generate_deploy_app_script)
        qt_generate_deploy_app_script(
            TARGET AutoPower
            OUTPUT_SCRIPT AUTOPOWER_QT_DEPLOY_SCRIPT
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
        install(SCRIPT ${AUTOPOWER_QT_DEPLOY_SCRIPT})
    elseif(WIN32)
        get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
        if(WINDEPLOYQT_EXECUTABLE)
            install(CODE "
                execute_process(
                    COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" --no-translations \"\${CMAKE_INSTALL_PREFIX}/bin/AutoPower.exe\"
                    RESULT_VARIABLE deploy_result
                )
                if(NOT deploy_result EQUAL 0)
                    message(FATAL_ERROR \"windeployqt failed with exit code \${deploy_result}\")
                endif()
            ")
        endif()
    elseif(APPLE)
        get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
        if(MACDEPLOYQT_EXECUTABLE)
            install(CODE "
                execute_process(
                    COMMAND \"${MACDEPLOYQT_EXECUTABLE}\" \"\${CMAKE_INSTALL_PREFIX}/AutoPower.app\" -always-overwrite
                    RESULT_VARIABLE deploy_result
                )
                if(NOT deploy_result EQUAL 0)
                    message(FATAL_ERROR \"macdeployqt failed with exit code \${deploy_result}\")
                endif()
            ")
        endif()
    endif()
endif()

set(CPACK_PACKAGE_NAME "AutoPower")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "AutoPower")
set(CPACK_PACKAGE_CONTACT "AutoPower Maintainers")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cross-platform scheduled power operation utility")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AutoPower")
elseif(WIN32)
    find_program(MAKENSIS_EXECUTABLE makensis)
    if(MAKENSIS_EXECUTABLE)
        set(CPACK_GENERATOR "NSIS;ZIP")
    else()
        set(CPACK_GENERATOR "ZIP")
    endif()
    set(CPACK_NSIS_DISPLAY_NAME "AutoPower")
    set(CPACK_NSIS_PACKAGE_NAME "AutoPower")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\AutoPower.exe")
else()
    find_program(DPKG_DEB_EXECUTABLE dpkg-deb)
    if(DPKG_DEB_EXECUTABLE)
        set(CPACK_GENERATOR "TGZ;DEB")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "AutoPower Maintainers")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()
include(CPack)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AutoPower)
endif()
