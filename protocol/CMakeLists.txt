# emc::protocol

set(PROTOCOL_SDK_DIR ${EMC_SDK_DIR}/protocol)

if(SDK)
  file(MAKE_DIRECTORY ${PROTOCOL_SDK_DIR})
endif(SDK)

add_subdirectory(raw)
add_subdirectory(emc)

if(EMC_ENABLE_MQTT)
endif()

if(EMC_ENABLE_HTTP)
endif()

