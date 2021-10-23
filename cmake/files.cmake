set(LINK_SOURCE_FILES
    "${PROJECT_SRC_DIR}/link/mqtt.cpp"
    "${PROJECT_SRC_DIR}/link/influx.cpp"
    )
set(LINK_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/link/mqtt.h"
    "${PROJECT_HEADER_DIR}/muonpi/link/influx.h"
    )

set(HTTP_SOURCE_FILES
    "${PROJECT_SRC_DIR}/http_server.cpp"
    "${PROJECT_SRC_DIR}/http_request.cpp"
    "${PROJECT_SRC_DIR}/http_tools.cpp"

    )
set(HTTP_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/http_server.h"
    "${PROJECT_HEADER_DIR}/muonpi/http_request.h"
    "${PROJECT_HEADER_DIR}/muonpi/http_tools.h"

    "${PROJECT_DETAIL_DIR}/http_session.hpp"
    )


set(CORE_SOURCE_FILES
    "${PROJECT_SRC_DIR}/threadrunner.cpp"
    "${PROJECT_SRC_DIR}/log.cpp"
    "${PROJECT_SRC_DIR}/configuration.cpp"
    "${PROJECT_SRC_DIR}/utility.cpp"
    "${PROJECT_SRC_DIR}/scopeguard.cpp"
    "${PROJECT_SRC_DIR}/exceptions.cpp"
    "${PROJECT_SRC_DIR}/supervision/resource.cpp"
    )

set(CORE_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/sink/base.h"
    "${PROJECT_HEADER_DIR}/muonpi/source/base.h"
    "${PROJECT_HEADER_DIR}/muonpi/pipeline/base.h"
    "${PROJECT_HEADER_DIR}/muonpi/threadrunner.h"
    "${PROJECT_HEADER_DIR}/muonpi/log.h"
    "${PROJECT_HEADER_DIR}/muonpi/configuration.h"
    "${PROJECT_HEADER_DIR}/muonpi/utility.h"
    "${PROJECT_HEADER_DIR}/muonpi/base64.h"
    "${PROJECT_HEADER_DIR}/muonpi/scopeguard.h"
    "${PROJECT_HEADER_DIR}/muonpi/exceptions.h"
    "${PROJECT_HEADER_DIR}/muonpi/gnss.h"
    "${PROJECT_HEADER_DIR}/muonpi/units.h"
    "${PROJECT_HEADER_DIR}/muonpi/analysis/dataseries.h"
    "${PROJECT_HEADER_DIR}/muonpi/analysis/cachedvalue.h"
    "${PROJECT_HEADER_DIR}/muonpi/analysis/ratemeasurement.h"
    "${PROJECT_HEADER_DIR}/muonpi/analysis/histogram.h"
    "${PROJECT_HEADER_DIR}/muonpi/analysis/uppermatrix.h"
    "${PROJECT_HEADER_DIR}/muonpi/supervision/resource.h"
    "${PROJECT_HEADER_DIR}/muonpi/global.h"
    "${PROJECT_HEADER_DIR}/muonpi/types.h"
    )

set(PROJECT_SOURCE_FILES
    ${CORE_SOURCE_FILES}
    ${LINK_SOURCE_FILES}
    ${HTTP_SOURCE_FILES}
    )

set(PROJECT_HEADER_FILES
    ${CORE_HEADER_FILES}
    ${LINK_HEADER_FILES}
    ${HTTP_HEADER_FILES}
    )
