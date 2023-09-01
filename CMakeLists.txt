# emc
# Enhanced Machine Control library
project(emc)
set(NAME emc)
set(PREFIX)

add_definitions(
  -fPIC
  -DCPU=${CPU}
  -DDPU=${DPU} -DDPU_H=<dpu/${DPU}.h>
  -DFPU=${FPU} -DFPU_H=<fpu/${FPU}.h>
  -DGPU=${GPU} -DGPU_H=<gpu/${GPU}.h>
  -DPIC
  ${HOST_DEFS}
)

include_directories(
  ${HOST_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(EMC_SDK_DIR ${HOST_SDK_DIR}/${NAME})

set(inc
  emc.h protocol.h error.h
  timer.h 
  pipeline.h
  controller.h monitor.h service.h
  session.h 
)

set(srcs
  timer.cpp
  pipeline.cpp
  gateway.cpp monitor.cpp service.cpp
  controller.cpp
)

configure_file(config.in.h ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(libs host ${HOST_LIBS})

add_library(${NAME} STATIC ${srcs})
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
