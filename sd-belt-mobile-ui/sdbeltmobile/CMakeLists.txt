cmake_minimum_required(VERSION 3.16)
project(sdbeltmobile VERSION 0.1 LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    QuickLayouts
)

qt_standard_project_setup(REQUIRES 6.8)

# Main executable first
qt_add_executable(appsdbeltmobile
    main.cpp
    apiservice.cpp
)

# QML module
qt_add_qml_module(appsdbeltmobile
    URI sdbeltmobile
    VERSION 1.0
    RESOURCES
        resources.qrc
    QML_FILES
        Main.qml
        OperationsPage.qml
        HomePage.qml
        ProductsPage.qml
        LoginPage.qml
        ProductStatsPage.qml
    # This ensures the QML dependencies are properly detected
    IMPORTS
        QtQuick
        QtQuick.Controls
        QtQuick.Layouts
        QtQuick.Controls.Material
)

set_target_properties(appsdbeltmobile PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appsdbeltmobile PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickLayouts
)

if(ANDROID)
    # Android icon configuration
    set_target_properties(appsdbeltmobile PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android
    )

    # OpenSSL setup
    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP true
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    
    if(EXISTS "${android_openssl_SOURCE_DIR}/android_openssl.cmake")
        include(${android_openssl_SOURCE_DIR}/android_openssl.cmake)
        message(STATUS "Included android_openssl.cmake from KDAB repository.")
    endif()
    
    if(COMMAND add_android_openssl_libraries)
        add_android_openssl_libraries(appsdbeltmobile)
        message(STATUS "Added Android OpenSSL libraries to target appsdbeltmobile.")
    endif()
    
    # Generate Android deployment settings
    qt_android_generate_deployment_settings(appsdbeltmobile)
    
    # Set the main QML file
    set_property(TARGET appsdbeltmobile PROPERTY QT_ANDROID_APPLICATION_ARGUMENT_QML "qrc:/Main.qml")
endif()
