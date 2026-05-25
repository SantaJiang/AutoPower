cmake_minimum_required(VERSION 3.16)

project(AutoPower
    VERSION 1.2.0
    DESCRIPTION "Cross-platform scheduled power operation utility"
    HOMEPAGE_URL "https://github.com/SantaJiang/AutoPower"
    LANGUAGES CXX
)

set(AUTOPOWER_APP_ID "com.santa.autopower" CACHE STRING "Reverse-DNS application identifier")
option(AUTOPOWER_DEPLOY_QT "Deploy Qt runtime libraries during install/package when supported." ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

include(GNUInstallDirs)

set(AUTOPOWER_QT_COMPONENTS Widgets LinguistTools)
if(UNIX AND NOT APPLE)
    list(APPEND AUTOPOWER_QT_COMPONENTS DBus)
endif()

find_package(Qt6 QUIET COMPONENTS ${AUTOPOWER_QT_COMPONENTS})
if(Qt6_FOUND)
    set(AUTOPOWER_QT_MAJOR 6)
    qt_standard_project_setup()
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS ${AUTOPOWER_QT_COMPONENTS})
    set(AUTOPOWER_QT_MAJOR 5)
endif()

set(AUTOPOWER_SOURCES
    main.cpp
    mainwindow.cpp
    powerop.cpp
    trayicon.cpp
    worker.cpp
    schedule/customdateedit.cpp
    schedule/lunarcalendarinfo.cpp
    schedule/minicalendaritem.cpp
    schedule/minicalendarwidget.cpp
)

set(AUTOPOWER_HEADERS
    mainwindow.h
    powerop.h
    trayicon.h
    worker.h
    schedule/customdateedit.h
    schedule/lunarcalendarinfo.h
    schedule/minicalendaritem.h
    schedule/minicalendarwidget.h
)

set(AUTOPOWER_FORMS
    mainwindow.ui
)

set(AUTOPOWER_RESOURCES
    src.qrc
)

if(AUTOPOWER_QT_MAJOR EQUAL 6)
    qt_add_executable(AutoPower
        WIN32
        MACOSX_BUNDLE
        MANUAL_FINALIZATION
        ${AUTOPOWER_SOURCES}
        ${AUTOPOWER_HEADERS}
        ${AUTOPOWER_FORMS}
        ${AUTOPOWER_RESOURCES}
    )
else()
    add_executable(AutoPower
        WIN32
        MACOSX_BUNDLE
        ${AUTOPOWER_SOURCES}
        ${AUTOPOWER_HEADERS}
        ${AUTOPOWER_FORMS}
        ${AUTOPOWER_RESOURCES}
    )
endif()

target_include_directories(AutoPower PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/schedule
)

target_link_libraries(AutoPower PRIVATE
    Qt${AUTOPOWER_QT_MAJOR}::Widgets
)

target_compile_definitions(AutoPower PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    AUTOPOWER_APP_ID="${AUTOPOWER_APP_ID}"
)

if(UNIX AND NOT APPLE)
    target_link_libraries(AutoPower PRIVATE Qt${AUTOPOWER_QT_MAJOR}::DBus)
endif()

if(WIN32)
    set(AUTOPOWER_WINDOWS_ICON "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/deploy/win/app.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/autopower.rc
        @ONLY
    )
    target_sources(AutoPower PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/autopower.rc)
    target_link_libraries(AutoPower PRIVATE PowrProf User32 Advapi32)
    set_target_properties(AutoPower PROPERTIES WIN32_EXECUTABLE ON)
elseif(APPLE)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/deploy/mac/Info.plist.in
        ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        @ONLY
    )
    set(AUTOPOWER_MAC_ICON "${CMAKE_CURRENT_SOURCE_DIR}/deploy/mac/autopower.icns")
    if(EXISTS "${AUTOPOWER_MAC_ICON}")
        set_source_files_properties("${AUTOPOWER_MAC_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        target_sources(AutoPower PRIVATE "${AUTOPOWER_MAC_ICON}")
        set(AUTOPOWER_MAC_ICON_FILE autopower.icns)
    endif()
    set_target_properties(AutoPower PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
        MACOSX_BUNDLE_GUI_IDENTIFIER ${AUTOPOWER_APP_ID}
        MACOSX_BUNDLE_BUNDLE_NAME AutoPower
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_ICON_FILE "${AUTOPOWER_MAC_ICON_FILE}"
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
elseif(UNIX)
    set_target_properties(AutoPower PROPERTIES
        INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    )
endif()

install(TARGETS AutoPower
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/autopower.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/autopower.appdata.xml
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
    )
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/autopower.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    )
endif()

if(AUTOPOWER_DEPLOY_QT)
    if(AUTOPOWER_QT_MAJOR EQUAL 6 AND COMMAND qt_generate_deploy_app_script)
        qt_generate_deploy_app_script(
            TARGET AutoPower
            OUTPUT_SCRIPT AUTOPOWER_QT_DEPLOY_SCRIPT
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
        install(SCRIPT ${AUTOPOWER_QT_DEPLOY_SCRIPT})
    elseif(WIN32)
        get_target_property(AUTOPOWER_QMAKE_EXECUTABLE Qt${AUTOPOWER_QT_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(AUTOPOWER_QT_BIN_DIR "${AUTOPOWER_QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${AUTOPOWER_QT_BIN_DIR}")
        if(WINDEPLOYQT_EXECUTABLE)
            install(CODE "
                execute_process(
                    COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" --no-translations \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/AutoPower.exe\"
                    RESULT_VARIABLE deploy_result
                )
                if(NOT deploy_result EQUAL 0)
                    message(FATAL_ERROR \"windeployqt failed with exit code \${deploy_result}\")
                endif()
            ")
        endif()
    elseif(APPLE)
        get_target_property(AUTOPOWER_QMAKE_EXECUTABLE Qt${AUTOPOWER_QT_MAJOR}::qmake IMPORTED_LOCATION)
        get_filename_component(AUTOPOWER_QT_BIN_DIR "${AUTOPOWER_QMAKE_EXECUTABLE}" DIRECTORY)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${AUTOPOWER_QT_BIN_DIR}")
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
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Santa")
set(CPACK_PACKAGE_CONTACT "Santa")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "AutoPower")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "AutoPower")
    set(CPACK_DMG_FORMAT "UDZO")
elseif(WIN32)
    find_program(MAKENSIS_EXECUTABLE makensis)
    if(MAKENSIS_EXECUTABLE)
        set(CPACK_GENERATOR "NSIS;ZIP")
    else()
        set(CPACK_GENERATOR "ZIP")
    endif()
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
    set(CPACK_NSIS_DISPLAY_NAME "AutoPower")
    set(CPACK_NSIS_PACKAGE_NAME "AutoPower")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_INSTALL_BINDIR}\\\\AutoPower.exe")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\AutoPower.lnk' '$INSTDIR\\\\${CMAKE_INSTALL_BINDIR}\\\\AutoPower.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\AutoPower.lnk'"
    )
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Santa")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
endif()

include(CPack)

if(AUTOPOWER_QT_MAJOR EQUAL 6)
    qt_finalize_executable(AutoPower)
endif()
