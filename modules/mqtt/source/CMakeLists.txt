
set(MQTT_SOURCE_SOURCES
    mqtt-source.c
    mqtt-source.h
)

include_directories(${MQTT_DIR})

add_module(
  TARGET mqtt-source
  DEPENDS eclipse-paho-mqtt-c::paho-mqtt3cs
  SOURCES ${MQTT_SOURCE_SOURCES}
  LIBRARY_TYPE STATIC # STATIC: Built as part of a larger libmqtt module
)
