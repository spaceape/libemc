#phobia::core::control::service

set(SERVICE_SRC_DIR ${CONTROL_SRC_DIR}/service)

set(inc
)

if(SDK)
  if(NOT "${inc}" STREQUAL "")
    file(MAKE_DIRECTORY ${SERVICE_SRC_DIR})
    install(
      FILES
        ${inc}
      DESTINATION
        ${SERVICE_SRC_DIR}
    )
  endif(NOT "${inc}" STREQUAL "")
endif(SDK)
