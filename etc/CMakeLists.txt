# emc::etc

set(ETC_SDK_DIR ${EMC_SDK_DIR}/etc)

set(inc
  timer.h latch.h
)

if(SDK)
  file(MAKE_DIRECTORY ${ETC_SDK_DIR})
  install(
    FILES
      ${inc}
    DESTINATION
      ${ETC_SDK_DIR}
  )
endif(SDK)

