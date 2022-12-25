#phobia::core::control::support

set(SUPPORT_SDK_DIR ${EMC_SDK_DIR}/support)

set(inc
)

if(SDK)
  if(NOT "${inc}" STREQUAL "")
    file(MAKE_DIRECTORY ${SUPPORT_SDK_DIR})
    install(
      FILES
        ${inc}
      DESTINATION
        ${SUPPORT_SDK_DIR}
    )
  endif(NOT "${inc}" STREQUAL "")
endif(SDK)
