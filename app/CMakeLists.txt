file(GLOB QML_JS_FILES *.qml *.js)

# Make the files visible on qtcreator
add_custom_target(just-say-it_QMlFiles ALL SOURCES ${QML_JS_FILES})

# Substitute variables in the desktop file
configure_file(${DESKTOP_FILE_NAME}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME}.in)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE_NAME} DESTINATION ${DESKTOP_DIR})
install(FILES ${QML_JS_FILES} DESTINATION ${JUST-SAY-IT_DIR})


