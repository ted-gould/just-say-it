
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Werror")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(
    JustSayItbackend_SRCS
    modules/JustSayIt/backend.cpp
    modules/JustSayIt/AudioRecorder.cpp
)

add_library(JustSayItbackend MODULE
    ${JustSayItbackend_SRCS}
)

set_target_properties(JustSayItbackend PROPERTIES
         LIBRARY_OUTPUT_DIRECTORY JustSayIt)

qt5_use_modules(JustSayItbackend Gui Qml Quick Multimedia Network)

# Copy qmldir file to build dir for running in QtCreator
add_custom_target(JustSayItbackend-qmldir ALL
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/modules/JustSayIt/qmldir ${CMAKE_CURRENT_BINARY_DIR}/JustSayIt
    DEPENDS ${QMLFILES}
)

# Install plugin file
install(TARGETS JustSayItbackend DESTINATION ${QT_IMPORTS_DIR}/JustSayIt/)
install(FILES   modules/JustSayIt/qmldir DESTINATION ${QT_IMPORTS_DIR}/JustSayIt/)

