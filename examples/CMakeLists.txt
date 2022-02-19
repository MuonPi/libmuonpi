add_subdirectory(log)
add_subdirectory(config)

if (LIBMUONPI_BUILD_DETECTOR)
add_subdirectory(i2c)
add_subdirectory(spi)
add_subdirectory(gpiod)
endif ()

if (LIBMUONPI_BUILD_HTTP)
add_subdirectory(http)
add_subdirectory(http-request)
endif ()

if (LIBMUONPI_BUILD_LINK)
add_subdirectory(mqtt)
add_subdirectory(influx)
endif ()
