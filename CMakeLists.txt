cmake_minimum_required(VERSION 3.24)
project(virtualhere VERSION 0.1.1 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Concurrent DBus LinguistTools)
qt_standard_project_setup()

qt_add_executable(virtualhere
    frontend/src/main.cpp
    frontend/src/client.cpp
    frontend/src/client.h
    frontend/src/window.cpp
    frontend/src/window.h)
target_compile_features(virtualhere PRIVATE cxx_std_20)
target_link_libraries(virtualhere PRIVATE Qt6::Widgets Qt6::Concurrent Qt6::DBus)

qt_add_resources(virtualhere virtualhere_icon
    PREFIX /icons
    BASE assets
    FILES assets/virtualhere.png)
qt_add_translations(virtualhere
    TS_FILES i18n/virtualhere_de.ts i18n/virtualhere_ru.ts
    RESOURCE_PREFIX /i18n)

install(TARGETS virtualhere RUNTIME DESTINATION bin)
