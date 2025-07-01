# emc
# Embedded Machine Control library
project(emc)
set(NAME emc)
set(PREFIX)

add_definitions(${HOST_DEFS})

if(LINUX)
  add_definitions(
    -fPIC
    -DPIC
  )
endif()

include_directories(
  ${HOST_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(EMC_ENABLE_MQTT ON CACHE BOOL "Enable MQTT protocol stack" FORCE)
set(EMC_ENABLE_HTTP ON CACHE BOOL "Enable MQTT protocol stack" FORCE)
set(EMC_SDK_DIR ${HOST_SDK_DIR}/${NAME})

configure_file(config.in.h ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(inc
  emc.h event.h error.h stage.h reactor.h
)

set(srcs
  etc/timer.cpp
  etc/latch.cpp
  event.cpp
  stage.cpp
  transport/base16.cpp transport/base64.cpp
  reactor.cpp
)

set(libs
  host
  ${HOST_LIBS}
)

add_subdirectory(etc)
add_subdirectory(transport)
add_subdirectory(protocol)

set(srcs ${srcs} reactor.cpp)

add_library(${NAME} ${srcs})
set_target_properties(${NAME} PROPERTIES PREFIX "${PREFIX}")
target_link_libraries(${NAME} ${libs})

if(SDK)
  file(MAKE_DIRECTORY ${EMC_SDK_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${EMC_SDK_DIR}
  )
endif(SDK)
