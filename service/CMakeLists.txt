#phobia::core::control::service

set(SERVICE_SDK_DIR ${EMC_SDK_DIR}/service)

set(inc
)

if(SDK)
  if(NOT "${inc}" STREQUAL "")
    file(MAKE_DIRECTORY ${SERVICE_SDK_DIR})
    install(
      FILES
        ${inc}
      DESTINATION
        ${SERVICE_SDK_DIR}
    )
  endif(NOT "${inc}" STREQUAL "")
endif(SDK)
