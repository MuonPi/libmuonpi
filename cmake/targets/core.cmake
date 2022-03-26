
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




if (NOT LIBMUONPI_FORMAT_ONLY)
add_library(muonpi-core SHARED ${CORE_SOURCE_FILES} ${CORE_HEADER_FILES})
target_link_libraries(muonpi-core ${PROJECT_INCLUDE_LIBS})

setup_packaging(
    COMPONENT "core"
    HEADERS "${CORE_HEADER_FILES}"
    DESCRIPTION "Libraries for MuonPi
 Core package"
    DESCRIPTIONDEV "Libraries for MuonPi
 Core dev package")
 endif ()
