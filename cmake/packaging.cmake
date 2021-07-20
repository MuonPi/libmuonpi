include(GNUInstallDirs)



macro(install_with_directory)
    set(optionsArgs "")
    set(oneValueArgs DESTINATION BASEDIR COMPONENT)
    set(multiValueArgs FILES)
    cmake_parse_arguments(CAS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    string(LENGTH ${CAS_BASEDIR} LEN_BASEDIR)

    foreach(FILE ${CAS_FILES})
        get_filename_component(DIR ${FILE} DIRECTORY)
        string(SUBSTRING ${DIR} ${LEN_BASEDIR} -1 RELDIR)
        INSTALL(FILES ${FILE} DESTINATION ${CAS_DESTINATION}/${RELDIR} COMPONENT ${CAS_COMPONENT})
    endforeach()
endmacro(install_with_directory)


set(LIBMUONPI_IS_RELEASE OFF)

configure_file("${PROJECT_CONFIG_DIR}/triggers" "${CMAKE_CURRENT_BINARY_DIR}/triggers")
configure_file("${PROJECT_CONFIG_DIR}/copyright" "${CMAKE_CURRENT_BINARY_DIR}/copyright")


set_target_properties(muonpi-core PROPERTIES
    SUFFIX ".so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
    INSTALL_REMOVE_ENVIRONMENT_RPATH ON
    SKIP_BUILD_RPATH ON
    )
configure_file("${PROJECT_CONFIG_DIR}/changelog-core" "${CMAKE_CURRENT_BINARY_DIR}/core/changelog")
add_custom_target(changelog-libmuonpi-core ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/core/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz")


if (LIBMUONPI_BUILD_MQTT)
    set_target_properties(muonpi-mqtt PROPERTIES
        SUFFIX ".so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-mqtt" "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog")
    add_custom_target(changelog-libmuonpi-mqtt ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz")
endif ()
if (LIBMUONPI_BUILD_REST)
    set_target_properties(muonpi-rest PROPERTIES
        SUFFIX ".so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-rest" "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog")
    add_custom_target(changelog-libmuonpi-rest ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog.gz")
endif ()
if (LIBMUONPI_BUILD_DETECTOR)
    set_target_properties(muonpi-detector PROPERTIES
        SUFFIX ".so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-detector" "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog")
    add_custom_target(changelog-libmuonpi-detector ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog.gz")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(LIBMUONPI_IS_RELEASE ON)
endif()

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz"
  DESTINATION "${CMAKE_INSTALL_DOCDIR}-core"
  COMPONENT libmuonpicore
  )
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-core" COMPONENT libmuonpicore)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-core" COMPONENT libmuonpicore)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-core-dev" COMPONENT libmuonpicoredev)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-core-dev" COMPONENT libmuonpicoredev)
if (${LIBMUONPI_IS_RELEASE})
  add_custom_command(
    TARGET muonpi-core
    POST_BUILD
    COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
endif ()
install_with_directory(
    FILES ${CORE_HEADER_FILES}
    DESTINATION include
    BASEDIR ${PROJECT_HEADER_DIR}
    COMPONENT libmuonpicoredev)
add_custom_command(
    TARGET muonpi-core
    POST_BUILD
    COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
    ln -sf "libmuonpi-core.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "libmuonpi-core.so.${PROJECT_VERSION_MAJOR}" &&
    ln -sf "libmuonpi-core.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" libmuonpi-core.so
    )
install(
    FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${PROJECT_VERSION_MAJOR}"
    DESTINATION lib
    COMPONENT libmuonpicore
    )
install(
    FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so"
    DESTINATION lib
    COMPONENT libmuonpicoredev
    )

if (LIBMUONPI_BUILD_MQTT)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-mqtt" COMPONENT libmuonpimqtt)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-mqtt" COMPONENT libmuonpimqtt)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-mqtt-dev" COMPONENT libmuonpimqttdev)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-mqtt-dev" COMPONENT libmuonpimqttdev)
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET muonpi-mqtt
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    endif ()
    install_with_directory(
        FILES ${MQTT_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpimqttdev)
    add_custom_command(
        TARGET muonpi-mqtt
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" libmuonpi-mqtt.so
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-mqtt.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT libmuonpimqtt
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-mqtt.so"
        DESTINATION lib
        COMPONENT libmuonpimqttdev
        )
endif ()

if (LIBMUONPI_BUILD_REST)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest" COMPONENT libmuonpirest)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest" COMPONENT libmuonpirest)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest-dev" COMPONENT libmuonpirestdev)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest-dev" COMPONENT libmuonpirestdev)
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET muonpi-rest
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    endif ()
    install_with_directory(
        FILES ${REST_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpirestdev
    )
    add_custom_command(
        TARGET muonpi-rest
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" libmuonpi-rest.so
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-rest.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT libmuonpirest
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-rest.so"
        DESTINATION lib
        COMPONENT libmuonpirestdev
        )
endif ()

if (LIBMUONPI_BUILD_DETECTOR)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-detector" COMPONENT libmuonpidetector)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-detector" COMPONENT libmuonpidetector)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-detector-dev" COMPONENT libmuonpidetectordev)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-detector-dev" COMPONENT libmuonpidetectordev)
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET muonpi-detector
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    endif ()
    install_with_directory(
        FILES ${DETECTOR_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpidetectordev
    )
    add_custom_command(
        TARGET muonpi-detector
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" libmuonpi-detector.so
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-detector.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT libmuonpidetector
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-detector.so"
        DESTINATION lib
        COMPONENT libmuonpidetectordev
        )
endif ()



set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/MuonPi/libmuonpi")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MuonPi <developer@muonpi.org>")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_CONFIG_DIR}/license")
set(CPACK_PACKAGE_VENDOR "MuonPi.org")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")





set(CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION "Libraries for MuonPi
 Core package")
set(CPACK_COMPONENT_LIBMUONPICORE_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICORE_PACKAGE_NAME "libmuonpi-core")
set(CPACK_DEBIAN_LIBMUONPICORE_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

set(CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION "Libraries for MuonPi
 Core dev package")
set(CPACK_COMPONENT_LIBMUONPICOREDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_NAME "libmuonpi-core-dev")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_DEPENDS "libmuonpi-core (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}), libboost-dev (>= 1.69)")

if (LIBMUONPI_BUILD_MQTT)
    set(CPACK_DEBIAN_LIBMUONPIMQTT_DESCRIPTION "Libraries for MuonPi
 MQTT package")
    set(CPACK_COMPONENT_LIBMUONPIMQTT_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIMQTT_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIMQTT_PACKAGE_NAME "libmuonpi-mqtt")
    set(CPACK_DEBIAN_LIBMUONPIMQTT_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_LIBMUONPIMQTT_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_DESCRIPTION "Libraries for MuonPi
 MQTT dev package")
    set(CPACK_COMPONENT_LIBMUONPIMQTTDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIMQTTDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_PACKAGE_NAME "libmuonpi-mqtt-dev")
    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_PACKAGE_DEPENDS "libmuonpi-mqtt (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}), libmuonpi-core-dev (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}), libmosquitto-dev")
endif ()

if (LIBMUONPI_BUILD_REST)
    set(CPACK_DEBIAN_LIBMUONPIREST_DESCRIPTION "Libraries for MuonPi
 REST package")
    set(CPACK_COMPONENT_LIBMUONPIREST_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIREST_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIREST_PACKAGE_NAME "libmuonpi-rest")
    set(CPACK_DEBIAN_LIBMUONPIREST_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_LIBMUONPIREST_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_DESCRIPTION "Libraries for MuonPi
 REST dev package")
    set(CPACK_COMPONENT_LIBMUONPIRESTDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIRESTDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_PACKAGE_NAME "libmuonpi-rest-dev")
    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_PACKAGE_DEPENDS "libmuonpi-rest (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}), libmuonpi-core-dev (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})")
endif ()

if (LIBMUONPI_BUILD_DETECTOR)
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_DESCRIPTION "Libraries for MuonPi
 detector package")
    set(CPACK_COMPONENT_LIBMUONPIDETECTOR_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIDETECTOR_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_PACKAGE_NAME "libmuonpi-detector")
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_DESCRIPTION "Libraries for MuonPi
 detector dev package")
    set(CPACK_COMPONENT_LIBMUONPIDETECTORDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIDETECTORDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_PACKAGE_NAME "libmuonpi-detector-dev")
    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_PACKAGE_DEPENDS "libmuonpi-detector (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}), libmuonpi-core-dev (= ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})")
endif ()

set(CPACK_DEB_COMPONENT_INSTALL ON)

message(STATUS "${CPACK_PACKAGE_VERSION}")

include(CPack)

