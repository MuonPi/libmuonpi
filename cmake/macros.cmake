macro(install_with_directory)
    set(optionsArgs "")
    set(oneValueArgs DESTINATION BASEDIR COMPONENT)
    set(multiValueArgs FILES)
    cmake_parse_arguments(CBS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    string(LENGTH ${CBS_BASEDIR} LEN_BASEDIR)

    foreach(FILE ${CBS_FILES})
        get_filename_component(DIR ${FILE} DIRECTORY)
        string(SUBSTRING ${DIR} ${LEN_BASEDIR} -1 RELDIR)
        INSTALL(FILES ${FILE} DESTINATION ${CBS_DESTINATION}/${RELDIR} COMPONENT ${CBS_COMPONENT})
    endforeach()
endmacro(install_with_directory)


macro(setup_packaging)
    set(optionsArgs "")
    set(oneValueArgs COMPONENT HEADERS DESCRIPTION DESCRIPTIONDEV)
    set(multiValueArgs "")
    cmake_parse_arguments(CAS "${optionsArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set_target_properties("muonpi-${CAS_COMPONENT}" PROPERTIES
        SUFFIX ".so.${CPACK_PACKAGE_VERSION}"
        INSTALL_REMOVE_ENVIRONMENT_RPATH ON
        SKIP_BUILD_RPATH ON
        )
    configure_file("${PROJECT_CONFIG_DIR}/changelog-${CAS_COMPONENT}" "${CMAKE_CURRENT_BINARY_DIR}/${CAS_COMPONENT}/changelog")
    add_custom_target("changelog-libmuonpi-${CAS_COMPONENT}" ALL COMMAND gzip -cn9 "${CMAKE_CURRENT_BINARY_DIR}/${CAS_COMPONENT}/changelog" > "${CMAKE_CURRENT_BINARY_DIR}/${CAS_COMPONENT}/changelog.gz")


      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-${CAS_COMPONENT}" COMPONENT "libmuonpi${CAS_COMPONENT}")
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CAS_COMPONENT}/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-${CAS_COMPONENT}" COMPONENT "libmuonpi${CAS_COMPONENT}")
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/copyright" DESTINATION "${CMAKE_INSTALL_DOCDIR}-${CAS_COMPONENT}-dev" COMPONENT "libmuonpi${CAS_COMPONENT}dev")
      install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CAS_COMPONENT}/changelog.gz" DESTINATION "${CMAKE_INSTALL_DOCDIR}-${CAS_COMPONENT}-dev" COMPONENT "libmuonpi${CAS_COMPONENT}dev")
    if (${LIBMUONPI_IS_RELEASE})
      add_custom_command(
        TARGET "muonpi-${CAS_COMPONENT}"
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-${CAS_COMPONENT}.so.${CPACK_PACKAGE_VERSION}")
    endif ()
    install_with_directory(
        FILES ${CAS_HEADERS}
        DESTINATION include
        BASEDIR ${PROJECT_HEADER_DIR}
        COMPONENT "libmuonpi${CAS_COMPONENT}dev")
    add_custom_command(
        TARGET "muonpi-${CAS_COMPONENT}"
        POST_BUILD
        COMMAND cd "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/" &&
        ln -sf "libmuonpi-${CAS_COMPONENT}.so.${CPACK_PACKAGE_VERSION}" "libmuonpi-${CAS_COMPONENT}.so.${PROJECT_VERSION_MAJOR}" &&
        ln -sf "libmuonpi-${CAS_COMPONENT}.so.${CPACK_PACKAGE_VERSION}" "libmuonpi-${CAS_COMPONENT}.so"
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-${CAS_COMPONENT}.so.${CPACK_PACKAGE_VERSION}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-${CAS_COMPONENT}.so.${PROJECT_VERSION_MAJOR}"
        DESTINATION lib
        COMPONENT "libmuonpi${CAS_COMPONENT}"
        )
    install(
        FILES "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmuonpi-${CAS_COMPONENT}.so"
        DESTINATION lib
        COMPONENT "libmuonpi${CAS_COMPONENT}dev"
        )


    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}_DESCRIPTION" "${CAS_DESCRIPTION}")
    set("CPACK_COMPONENT_LIBMUONPI${CAS_COMPONENT}_DESCRIPTION" "${CAS_DESCRIPTION}")
    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}_PACKAGE_NAME" "libmuonpi-${CAS_COMPONENT}")
    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}_PACKAGE_DEPENDS" "libmuonpi-core (= ${CPACK_PACKAGE_VERSION})")
    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}_PACKAGE_CONTROL_EXTRA" "${CMAKE_CURRENT_BINARY_DIR}/triggers")

    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}DEV_DESCRIPTION" "${CAS_DESCRIPTIONDEV}")
    set("CPACK_COMPONENT_LIBMUONPI${CAS_COMPONENT}DEV_DESCRIPTION" "${CAS_DESCRIPTIONDEV}")
    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}DEV_PACKAGE_NAME" "libmuonpi-${CAS_COMPONENT}-dev")
    set("CPACK_DEBIAN_LIBMUONPI${CAS_COMPONENT}DEV_PACKAGE_DEPENDS" "libmuonpi-${CAS_COMPONENT} (= ${CPACK_PACKAGE_VERSION}), libmuonpi-core-dev (= ${CPACK_PACKAGE_VERSION}), libmosquitto-dev")

endmacro(setup_packaging)
