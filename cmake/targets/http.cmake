option(LIBMUONPI_BUILD_HTTP "Build the HTTP service code" ON)

set(HTTP_SOURCE_FILES
    "${PROJECT_SRC_DIR}/http_server.cpp"
    "${PROJECT_SRC_DIR}/http_request.cpp"
    "${PROJECT_SRC_DIR}/http_response.cpp"
    "${PROJECT_SRC_DIR}/http_tools.cpp"
    )

set(HTTP_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/http_server.h"
    "${PROJECT_HEADER_DIR}/muonpi/http_request.h"
    "${PROJECT_HEADER_DIR}/muonpi/http_response.h"
    "${PROJECT_HEADER_DIR}/muonpi/http_tools.h"

    "${PROJECT_DETAIL_DIR}/http_session.hpp"
    )

if (NOT LIBMUONPI_FORMAT_ONLY)
if (LIBMUONPI_BUILD_HTTP) # libraries specific to the REST library
    add_library(muonpi-http SHARED ${HTTP_SOURCE_FILES} ${HTTP_HEADER_FILES})

    add_dependencies(muonpi-http muonpi-core)
    target_link_libraries(muonpi-http ${PROJECT_INCLUDE_LIBS} muonpi-core ssl crypto)

    setup_packaging(
        COMPONENT "http"
        HEADERS "${HTTP_HEADER_FILES}"
        DESCRIPTION "Libraries for MuonPi
 Link package"
        DESCRIPTIONDEV "Libraries for MuonPi
 Link dev package")
endif ()
endif ()
