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

set(EMC_SDK_DIR ${HOST_SDK_DIR}/${NAME})

configure_file(config.in.h ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(inc
  emc.h protocol.h event.h error.h
  timer.h 
  pipeline.h
  gateway.h monitor.h controller.h
  reactor.h 
)

set(srcs
  timer.cpp
  pipeline.cpp
  transport/base16.cpp transport/base64.cpp
  gateway.cpp monitor.cpp controller.cpp mapper.cpp
  reactor.cpp
)

set(libs
  host
  ${HOST_LIBS}
)

add_subdirectory(transport)

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
