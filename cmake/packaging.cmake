include(GNUInstallDirs)

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
    SUFFIX ".so.${CPACK_PACKAGE_VERSION}"
    INSTALL_REMOVE_ENVIRONMENT_RPATH ON
    SKIP_BUILD_RPATH ON
    )
configure_file("${PROJECT_CONFIG_DIR}/changelog-core" "${CMAKE_CURRENT_BINARY_DIR}/core/changelog")
add_custom_target(changelog-libmuonpi-core ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/core/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/core/changelog.gz")


if (LIBMUONPI_BUILD_LINK)
    set_target_properties(muonpi-link PROPERTIES
        SUFFIX ".so.${CPACK_PACKAGE_VERSION}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-link" "${CMAKE_CURRENT_BINARY_DIR}/link/changelog")
    add_custom_target(changelog-libmuonpi-link ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/link/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/link/changelog.gz")
endif ()
if (LIBMUONPI_BUILD_HTTP)
    set_target_properties(muonpi-http PROPERTIES
        SUFFIX ".so.${CPACK_PACKAGE_VERSION}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-rest" "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog")
    add_custom_target(changelog-libmuonpi-http ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/rest/changelog.gz")
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
    COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${CPACK_PACKAGE_VERSION}")
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
    ln -sf "libmuonpi-core.so.${CPACK_PACKAGE_VERSION}" "libmuonpi-core.so.${PROJECT_VERSION_MAJOR}" &&
    ln -sf "libmuonpi-core.so.${CPACK_PACKAGE_VERSION}" libmuonpi-core.so
    )
install(
    FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${CPACK_PACKAGE_VERSION}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so.${PROJECT_VERSION_MAJOR}"
    DESTINATION lib
    COMPONENT libmuonpicore
    )
install(
    FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-core.so"
    DESTINATION lib
    COMPONENT libmuonpicoredev
    )

if (LIBMUONPI_BUILD_LINK)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-link" COMPONENT libmuonpilink)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/link/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-link" COMPONENT libmuonpilink)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-link-dev" COMPONENT libmuonpilinkdev)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/link/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-link-dev" COMPONENT libmuonpilinkdev)
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET muonpi-link
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-link.so.${CPACK_PACKAGE_VERSION}")
    endif ()
    install_with_directory(
        FILES ${LINK_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpilinkdev)
    add_custom_command(
        TARGET muonpi-link
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-link.so.${CPACK_PACKAGE_VERSION}" "libmuonpi-link.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-link.so.${CPACK_PACKAGE_VERSION}" libmuonpi-link.so
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-link.so.${CPACK_PACKAGE_VERSION}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-link.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT libmuonpilink
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-link.so"
        DESTINATION lib
        COMPONENT libmuonpilinkdev
        )
endif ()

if (LIBMUONPI_BUILD_HTTP)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest" COMPONENT libmuonpihttp)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/link/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest" COMPONENT libmuonpihttp)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest-dev" COMPONENT libmuonpihttpdev)
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/link/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-rest-dev" COMPONENT libmuonpihttpdev)
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET muonpi-http
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-http.so.${CPACK_PACKAGE_VERSION}")
    endif ()
    install_with_directory(
        FILES ${HTTP_HEADER_FILES}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT libmuonpihttpdev
    )
    add_custom_command(
        TARGET muonpi-http
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-http.so.${CPACK_PACKAGE_VERSION}" "libmuonpi-http.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-http.so.${CPACK_PACKAGE_VERSION}" libmuonpi-http.so
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-http.so.${CPACK_PACKAGE_VERSION}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-http.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT libmuonpihttp
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-http.so"
        DESTINATION lib
        COMPONENT libmuonpihttpdev
        )
endif ()








set(CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION "Libraries for MuonPi
 Core package")
set(CPACK_COMPONENT_LIBMUONPICORE_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICORE_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICORE_PACKAGE_NAME "libmuonpi-core")
set(CPACK_DEBIAN_LIBMUONPICORE_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

set(CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION "Libraries for MuonPi
 Core dev package")
set(CPACK_COMPONENT_LIBMUONPICOREDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPICOREDEV_DESCRIPTION}")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_NAME "libmuonpi-core-dev")
set(CPACK_DEBIAN_LIBMUONPICOREDEV_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION}), libboost-dev (>= 1.69)")

if (LIBMUONPI_BUILD_LINK)
    set(CPACK_DEBIAN_LIBMUONPILINK_DESCRIPTION "Libraries for MuonPi
 LINK package")
    set(CPACK_COMPONENT_LIBMUONPILINK_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPILINK_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPILINK_PACKAGE_NAME "libmuonpi-link")
    set(CPACK_DEBIAN_LIBMUONPILINK_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_LIBMUONPILINK_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set(CPACK_DEBIAN_LIBMUONPILINKDEV_DESCRIPTION "Libraries for MuonPi
 LINK dev package")
    set(CPACK_COMPONENT_LIBMUONPILINKDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPILINKDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPILINKDEV_PACKAGE_NAME "libmuonpi-link-dev")
    set(CPACK_DEBIAN_LIBMUONPILINKDEV_PACKAGE_DEPENDS "libmuonpi-link (= ${CPACK_PACKAGE_VERSION}), libmuonpi-core-dev (= ${CPACK_PACKAGE_VERSION}), libmosquitto-dev")
endif ()

if (LIBMUONPI_BUILD_HTTP)
    set(CPACK_DEBIAN_LIBMUONPIHTTP_DESCRIPTION "Libraries for MuonPi
 HTTP package")
    set(CPACK_COMPONENT_LIBMUONPIHTTP_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIHTTP_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIHTTP_PACKAGE_NAME "libmuonpi-http")
    set(CPACK_DEBIAN_LIBMUONPIHTTP_PACKAGE_DEPENDS "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set(CPACK_DEBIAN_LIBMUONPIHTTP_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set(CPACK_DEBIAN_LIBMUONPIHTTPDEV_DESCRIPTION "Libraries for MuonPi
 HTTP dev package")
    set(CPACK_COMPONENT_LIBMUONPIHTTPDEV_DESCRIPTION "${CPACK_DEBIAN_LIBMUONPIHTTPDEV_DESCRIPTION}")
    set(CPACK_DEBIAN_LIBMUONPIHTTPDEV_PACKAGE_NAME "libmuonpi-http-dev")
    set(CPACK_DEBIAN_LIBMUONPIHTTPDEV_PACKAGE_DEPENDS "libmuonpi-http (= ${CPACK_PACKAGE_VERSION}), libmuonpi-core-dev (= ${CPACK_PACKAGE_VERSION})")
endif ()

set(CPACK_DEB_COMPONENT_INSTALL ON)

message(STATUS "${CPACK_PACKAGE_VERSION}")

include(CPack)

