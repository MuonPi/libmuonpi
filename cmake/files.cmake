set(MQTT_SOURCE_FILES
    "${PROJECT_SRC_DIR}/link/mqtt.cpp"
    )
set(MQTT_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/link/mqtt.h"
    )

set(REST_SOURCE_FILES
    "${PROJECT_SRC_DIR}/restservice.cpp"
    )
set(REST_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/restservice.h"
    )


set(DETECTOR_SOURCE_FILES
    "${PROJECT_SRC_DIR}/detector.cpp"
    )
set(DETECTOR_HEADER_FILES
    "${PROJECT_HEADER_DIR}/muonpi/detector.h"
    )

set(CORE_SOURCE_FILES
    "${PROJECT_SRC_DIR}/threadrunner.cpp"
    "${PROJECT_SRC_DIR}/log.cpp"
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
    )

set(PROJECT_SOURCE_FILES
    ${CORE_SOURCE_FILES}
    ${MQTT_SOURCE_FILES}
    ${REST_SOURCE_FILES}
    )

set(PROJECT_HEADER_FILES
    ${CORE_HEADER_FILES}
    ${MQTT_HEADER_FILES}
    ${REST_HEADER_FILES}
    )
