set(component_requires
    driver
    esp_timer
    nvs_flash
)

if(APP_PROTOCOL STREQUAL "matter")
    list(APPEND component_requires esp_matter)
endif()

idf_component_register(
    SRCS
        "app_main.c"
        "matter_bridge.cpp"
        "pc_control.c"
        "protocol_bridge.c"
        "uart_console.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        ${component_requires}
)

if(APP_PROTOCOL STREQUAL "matter")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE APP_PROFILE_MATTER=1)
elseif(APP_PROTOCOL STREQUAL "zigbee")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE APP_PROFILE_ZIGBEE=1)
else()
    target_compile_definitions(${COMPONENT_LIB} PRIVATE APP_PROFILE_NONE=1)
endif()

message(STATUS "esp-pc APP_PROTOCOL=${APP_PROTOCOL}")
