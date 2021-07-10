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


set(LIBMUONPI_PACKAGE_ADDITION "")
set(LIBMUONPI_IS_RELEASE OFF)

configure_file("${PROJECT_CONFIG_DIR}/changelog-core" "${CMAKE_CURRENT_BINARY_DIR}/core/changelog")
add_custom_target(changelog-libmuonpi-core ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/core/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz")

if (LIBMUONPI_BUILD_MQTT)
    configure_file("${PROJECT_CONFIG_DIR}/changelog-mqtt" "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog")
    add_custom_target(changelog-libmuonpi-mqtt ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz")
endif ()
if (LIBMUONPI_BUILD_REST)
    configure_file("${PROJECT_CONFIG_DIR}/changelog-rest" "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog")
    add_custom_target(changelog-libmuonpi-rest ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog.gz")
endif ()
if (LIBMUONPI_BUILD_DETECTOR)
    configure_file("${PROJECT_CONFIG_DIR}/changelog-detector" "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog")
    add_custom_target(changelog-libmuonpi-detector ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog.gz")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(LIBMUONPI_PACKAGE_ADDITION "-dbg")
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
    set(LIBMUONPI_IS_RELEASE ON)
endif()

install(
  TARGETS muonpi-core
  DESTINATION lib
  COMPONENT libmuonpicore
  )
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz"
  DESTINATION "${CMAKE_INSTALL_DOCDIR}"
  COMPONENT libmuonpicore
  )
if (${LIBMUONPI_IS_RELEASE})
install_with_directory(
    FILES ${CORE_HEADER_FILES}
    DESTINATION include
    BASEDIR ${PROJECT_HEADER_DIR}
    COMPONENT libmuonpicoredev)
endif ()

if (LIBMUONPI_BUILD_MQTT)
    install(
        TARGETS muonpi-mqtt
        DESTINATION lib
        COMPONENT libmuonpimqtt)
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/mqtt/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT libmuonpimqtt
        )
    if (${LIBMUONPI_IS_RELEASE})
    install_with_directory(
        FILES ${MQTT_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpimqttdev)
    endif ()
endif ()

if (LIBMUONPI_BUILD_REST)
    install(
        TARGETS muonpi-rest
        DESTINATION lib
        COMPONENT libmuonpirest
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT libmuonpirest
    )
    if (${LIBMUONPI_IS_RELEASE})
    install_with_directory(
        FILES ${REST_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpirestdev
    )
    endif ()
endif ()

if (LIBMUONPI_BUILD_DETECTOR)
    install(
        TARGETS muonpi-detector
        DESTINATION lib
        COMPONENT libmuonpidetector
    )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/detector/changelog.gz"
        DESTINATION "${CMAKE_INSTALL_DOCDIR}"
        COMPONENT libmuonpidetector
    )
    if (${LIBMUONPI_IS_RELEASE})
    install_with_directory(
        FILES ${DETECTOR_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpidetectordev
    )
    endif ()
endif ()



set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/MuonPi/libmuonpi")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MuonPi <developer@muonpi.org>")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_CONFIG_DIR}/license")
set(CPACK_PACKAGE_VENDOR "MuonPi.org")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")





set(CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION "Libraries for MuonPi")
set(CPACK_COMPONENT_LIBMUONPICORE_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICORE_PACKAGE_NAME "libmuonpi-core${LIBMUONPI_PACKAGE_ADDITION}")

if (${LIBMUONPI_IS_RELEASE})
set(CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION "Libraries for MuonPi")
set(CPACK_COMPONENT_LIBMUONPICOREDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_NAME "libmuonpi-core-dev")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_DEPENDS "libmuonpi-core libboost-dev")
endif ()

if (LIBMUONPI_BUILD_MQTT)
    set(CPACK_DEBIAN_LIBMUONPIMQTT_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIMQTT_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIMQTT_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIMQTT_PACKAGE_NAME "libmuonpi-mqtt${LIBMUONPI_PACKAGE_ADDITION}")
    set(CPACK_DEBIAN_LIBMUONPIMQTT_PACKAGE_DEPENDS "libmuonpi-core")

    if (${LIBMUONPI_IS_RELEASE})
    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIMQTTDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIMQTTDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_PACKAGE_NAME "libmuonpi-mqtt-dev")
    set(CPACK_DEBIAN_LIBMUONPIMQTTDEV_PACKAGE_DEPENDS "libmuonpi-mqtt libmosquitto-dev")
    endif ()
endif ()

if (LIBMUONPI_BUILD_REST)
    set(CPACK_DEBIAN_LIBMUONPIREST_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIREST_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIREST_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIREST_PACKAGE_NAME "libmuonpi-rest${LIBMUONPI_PACKAGE_ADDITION}")
    set(CPACK_DEBIAN_LIBMUONPIREST_PACKAGE_DEPENDS "libmuonpi-core")

    if (${LIBMUONPI_IS_RELEASE})
    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIRESTDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIRESTDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_PACKAGE_NAME "libmuonpi-rest-dev")
    set(CPACK_DEBIAN_LIBMUONPIRESTDEV_PACKAGE_DEPENDS "libmuonpi-rest libmuonpi-core-dev")
    endif ()
endif ()

if (LIBMUONPI_BUILD_DETECTOR)
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIDETECTOR_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIDETECTOR_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_PACKAGE_NAME "libmuonpi-detector${LIBMUONPI_PACKAGE_ADDITION}")
    set(CPACK_DEBIAN_LIBMUONPIDETECTOR_PACKAGE_DEPENDS "libmuonpi-core")

    if (${LIBMUONPI_IS_RELEASE})
    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_DESCRIPTION "Libraries for MuonPi")
    set(CPACK_COMPONENT_LIBMUONPIDETECTORDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIDETECTORDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_PACKAGE_NAME "libmuonpi-rest-dev")
    set(CPACK_DEBIAN_LIBMUONPIDETECTORDEV_PACKAGE_DEPENDS "libmuonpi-detector libmuonpi-core-dev")
    endif ()
endif ()

set(CPACK_DEB_COMPONENT_INSTALL ON)
include(CPack)
